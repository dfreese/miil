#include <miil/process/ProcessParams.h>
#include <algorithm>
#include <iterator>
#include <miil/SystemConfiguration.h>
#include <miil/sorting.h>
#include <miil/ethernet.h>
#include <miil/processing.h>
#include <miil/util.h>
#include <miil/process/ProcessControl.h>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <atomic>
#include <condition_variable>

using namespace std;

int ProcessParams::no_instances = 0;

namespace {
atomic_bool increment_filename = ATOMIC_VAR_INIT(false);
atomic_int no_threads_waiting = ATOMIC_VAR_INIT(0);
std::mutex mtx;
condition_variable cv_thread_arrived;
}

ProcessParams::ProcessParams(
        Ethernet * const eth_ptr,
        SystemConfiguration const * const sysconfig_ptr,
        ProcessControl * const control_ptr,
        size_t buffer_receive_side_size,
        size_t buffer_transfer_size,
        size_t raw_storage_size,
        size_t decoded_storage_size,
        size_t calibrated_storage_size,
        long sorting_max_delay,
        float egate_low,
        float egate_high,
        bool split_files,
        size_t max_file_size) :
    ethernet(eth_ptr),
    system_config(sysconfig_ptr),
    control(control_ptr),
    assumed_max_delay(sorting_max_delay),
    energy_gate_low(egate_low),
    energy_gate_high(egate_high),
    buffer_transfer(buffer_transfer_size),
    split_files_flag(split_files),
    file_size_max(max_file_size),
    file_count(-1),
    write_raw_data_flag(false),
    write_decoded_events_flag(false),
    write_calibrated_events_flag(false),
    files_reset_flag(false),
    current_file_size(0),
    raw_storage(raw_storage_size),
    decoded_storage(decoded_storage_size),
    calibrated_storage(calibrated_storage_size)
{
    buffer_receive_side.reserve(buffer_receive_side_size);
    no_instances++;
}

int ProcessParams::DecodeBuffer(size_t write_to_position) {
    assert(write_to_position <= buffer_process_side.size());
    for (size_t ii = info.current_index;
         ii < write_to_position;
         ii++)
    {
        info.bytes_processed++;
        if (buffer_process_side[ii] == (char) 0x80) {
            info.start_index = ii;
            info.found_start = true;
        } else if (buffer_process_side[ii] == (char) 0x81) {
            // Check to make sure start byte has been found first.
            // The contrary condition would indicating we are dropping start
            // bytes but finding end bytes, which might be useful for debug
            // purposes.
            if (info.found_start) {
                int decode_status =
                    DecodePacketByteStream(
                            buffer_process_side.begin() +
                                info.start_index,
                            buffer_process_side.begin() +
                                ii + 1,
                            system_config,
                            decoded_data);
                if (decode_status == 0) {
                    info.accepted_decode++;
                } else if (decode_status == -1) {
                    info.dropped_empty++;
                } else if (decode_status == -2) {
                    info.dropped_start_stop++;
                } else if (decode_status == -3) {
                    info.dropped_trigger_code++;
                } else if (decode_status == -4) {
                    info.dropped_packet_size++;
                } else if (decode_status == -5) {
                    info.dropped_address_byte++;
                }
            }
            info.found_start = false;
        }
    }
    return(0);
}

int ProcessParams::ClearProcessedData() {
    // If the loop exits with a packet found, delete up to the start of
    // the packet.  If not, then delete all of the data.  Either way the next
    // loop should start reading at the point new data will be added to the
    // buffer, which is buffer.size(), or one past the current last index.
    if (info.found_start) {
        buffer_process_side.erase(
                buffer_process_side.begin(),
                buffer_process_side.begin() +
                    info.start_index);
        info.start_index = 0;
    } else {
        buffer_process_side.clear();
    }
    info.current_index =
            buffer_process_side.size();
    return(0);
}

// The functions that should be run during the loop of ProcessData as well as at
// the loop's completion.
int ProcessParams::HandleData(bool write_out_remaining_cal_data) {
    size_t size_before_pull = buffer_process_side.size();
    buffer_transfer.copy_and_clear(buffer_process_side);
    if (buffer_process_side.size() == size_before_pull) {
        buffer_transfer.wait_for_pull_all(buffer_process_side, 500);
    }
    raw_storage.try_insert(
            buffer_process_side.begin(),
            buffer_process_side.end());


    // Figure out if there's enought room in the raw file for the current buffer
    size_t bytes_to_write = std::distance(
            buffer_process_side.begin() + info.current_index,
            buffer_process_side.end());

    info.bytes_transferred += bytes_to_write;

    size_t bytes_left = file_size_max - current_file_size;
    size_t write_to_position = buffer_process_side.size();

    if (split_files_flag) {
        // If there is, write to the end of the file and continue as normal
        // if not, write out to the end of the file
        if (bytes_to_write > bytes_left) {
            // Raise a flag to tell all threads to go to the next filename.
            increment_filename = true;
            bytes_to_write = bytes_left;
            // Override this function, by forcing the program to write out the
            // rest of the calibrated data, even if it might not be sorted in
            // the full unsplit bytestream from the system, so that each raw
            // data file is matched with a corresponding decoded and calibrated
            // file.
            write_out_remaining_cal_data = true;
            // Write out to the end of the full file
            write_to_position = info.current_index + bytes_left;
        }
    }

    // Declare iterator so that it can be used for writing at the end.  This
    // will be set to write out all events, or all the events that we can assume
    // to be sorted, unless the data acqusition is ending or the file is being
    // close, in which case we write out all of them.
    //
    // Don't want to initialize this to a value until the end to avoid it
    // possibly being invalidated.
    vector<EventCal>::iterator write_out_iter;

    if (control->decode_events_flag) {
        // Make sure to only decode through the end of the written file
        DecodeBuffer(write_to_position);
        decoded_storage.try_insert(
                decoded_data.begin(),
                decoded_data.end());

        // Calibrate data
        if (control->calibrate_events_flag) {
            for (size_t ii = 0;
                 ii < decoded_data.size();
                 ii++)
            {
                EventCal event;
                int cal_status = RawEventToEventCal(
                        decoded_data[ii],
                        event,
                        system_config);
                info.decoded_events_processed++;
                if (cal_status == 0) {
                    if (control->energy_gate_calibrated_events_flag) {
                        if (InEnergyWindow(event,
                                           energy_gate_low,
                                           energy_gate_high))
                        {
                            calibrated_data.push_back(event);
                            info.accepted_calibrate++;
                        } else {
                            info.dropped_energy_gate++;
                        }
                    } else {
                        calibrated_data.push_back(event);
                        info.accepted_calibrate++;
                    }
                } else if (cal_status == -1) {
                    info.dropped_threshold++;
                } else if (cal_status == -2) {
                    info.dropped_double_trigger++;
                } else if (cal_status == -3) {
                    info.dropped_crystal_id++;
                } else if (cal_status == -4) {
                    info.dropped_crystal_invalid++;
                }
            }

            if (control->sort_calibrated_events_flag &&
                !calibrated_data.empty())
            {
                insertion_sort(
                        calibrated_data,
                        EventCalLessThan,
                        (float) system_config->uv_period_ns,
                        (float) system_config->ct_period_ns);
                // 2) for each event, calculate event time - last event time
                // 3) find lower_bound of time_diff wrt (0 - assumed max delay)
                EventCal delay_reference_event =
                        calibrated_data.back();
                delay_reference_event.ct -=
                        assumed_max_delay;
                write_out_iter = lower_bound(
                        calibrated_data.begin(),
                        calibrated_data.end(),
                        delay_reference_event,
                        EventCalLessThanOnlyCt);
            } else {
                // If we are not sorting, write out until end.
                write_out_iter = calibrated_data.end();
            }
            if (write_out_remaining_cal_data) {
                write_out_iter = calibrated_data.end();
            }
            // 4) write beginning to lower_bound

            calibrated_storage.try_insert(
                    calibrated_data.begin(),
                    write_out_iter);
        }
    }

    if (control->write_data_flag) {
        if (split_files_flag) {
            // Increment the counter anyways, since we split files based on the
            // number of received bytes only, not the size of the decoded or
            // calibrated files.
            current_file_size += bytes_to_write;
        }
        if (write_raw_data_flag) {
            std::copy(
                    buffer_process_side.begin() + info.current_index,
                    buffer_process_side.begin() + write_to_position,
                    std::ostreambuf_iterator<char>(raw_output_file));
            info.written_raw_bytes += bytes_to_write;
        }
        if (write_decoded_events_flag) {
            decoded_output_file.write(
                    (char*) decoded_data.data(),
                    sizeof(EventRaw) * decoded_data.size());
            info.written_decoded_events +=
                    decoded_data.size();
        }
        if (write_calibrated_events_flag) {
            size_t no_events = distance(
                    calibrated_data.begin(), write_out_iter);
            eventcal_output_file.write(
                    (char*) calibrated_data.data(),
                    sizeof(EventCal) * no_events);
            info.written_calibrated_events += no_events;
        }
    }


    if (increment_filename) {
        if (raw_output_file.is_open()) {
            raw_output_file.close();
        }
        if (decoded_output_file.is_open()) {
            decoded_output_file.close();
        }
        if (eventcal_output_file.is_open()) {
            eventcal_output_file.close();
        }
        current_file_size = 0;
        file_count++;
        SetupFiles();

        // Synchronize threads so that they all start on the next file
        // simultaneously.
        no_threads_waiting++;
        if (no_threads_waiting < no_instances) {
            std::unique_lock<std::mutex> lck(mtx);
            cv_thread_arrived.wait(lck);
        } else {
            std::unique_lock<std::mutex> lck(mtx);
            // All of the threads have arrived, reset and exit.
            no_threads_waiting = 0;
            increment_filename = false;
            cv_thread_arrived.notify_all();
        }
    }

    // This will clear out the process side buffer up through what we have
    // decoded.  If we are not waiting for the end of the packet
    // (info.packet_found = true), then it will clear the entire buffer.  This
    // is the default case if we are not decoding data.
    ClearProcessedData();
    // Clear out the other buffers
    decoded_data.clear();
    // Leave the part of the calibrated data that we cannot assume is sorted yet
    calibrated_data.erase(calibrated_data.begin(), write_out_iter);
    return(0);
}

int ProcessParams::ProcessData() {
    while (control->process_data_flag) {
        HandleData(false);
        updateProcessInfo();
    }
    if (control->end_of_acquisiton_flag) {
        // Clean up any data remaining in the queue.
        HandleData(true);
        // Clear out any incomplete packets left hanging in the queue.
        buffer_process_side.clear();
        updateProcessInfo();
        current_file_size = 0;
    }
    files_reset_flag = false;
    return(0);
}

int ProcessParams::ReadSockets() {
    while(control->read_sockets_flag) {
        int status = ethernet->recv(buffer_receive_side);
        if (status > 0) {
            info.recv_calls_normal++;
            info.bytes_received += status;
            buffer_transfer.try_insert(buffer_receive_side);
        } else if (status == 0) {
            info.recv_calls_zero++;
        } else {
            info.recv_calls_error++;
        }
        updateProcessInfo();
    }
    buffer_transfer.insert(buffer_receive_side);
    return(0);
}

int ProcessParams::ReadWriteSockets() {
    while(control->read_sockets_flag) {
        int status = ethernet->recv(buffer_receive_side);
        if (status > 0) {
            info.recv_calls_normal++;
            info.bytes_received += status;
        } else if (status == 0) {
            info.recv_calls_zero++;
        } else {
            info.recv_calls_error++;
        }
        size_t bytes_to_write = buffer_receive_side.size();
        size_t bytes_left = file_size_max - current_file_size;

        if (split_files_flag) {
            // If there is, write to the end of the file and continue as normal
            // if not, write out to the end of the file
            if (bytes_to_write > bytes_left) {
                // Raise a flag to tell all threads to go to the next filename.
                increment_filename = true;
                bytes_to_write = bytes_left;
            }
            // Increment the counter anyways, since we split files based on the
            // number of received bytes only, not the size of the decoded or
            // calibrated files.
            current_file_size += bytes_to_write;
        }

        raw_output_file.write((char*) buffer_receive_side.data(), bytes_to_write);
        info.written_raw_bytes += bytes_to_write;

        if (increment_filename) {
            if (write_raw_data_flag) {
                raw_output_file.close();
            }
            current_file_size = 0;
            file_count++;
            SetupFiles();
            // Synchronize threads to make sure there is no data race to the
            // increment_filename section.
            no_threads_waiting++;
            while (no_threads_waiting < no_instances) {
            }
            // All of the threads have arrived, reset and exit.
            no_threads_waiting = 0;
            increment_filename = false;
        }
        buffer_receive_side.clear();

        updateProcessInfo();
    }
    buffer_transfer.insert(buffer_receive_side);
    return(0);
}

void ProcessParams::updateProcessInfo() {
    if (lock_locked_info.try_lock()) {
        locked_info = info;
        lock_locked_info.unlock();
    }
}

ProcessInfo ProcessParams::getProcessInfo() {
    lock_locked_info.lock();
    ProcessInfo local_copy = locked_info;
    lock_locked_info.unlock();
    return(local_copy);
}

void ProcessParams::resetProcessInfo() {
    info.reset();
    locked_info = info;
}

int ProcessParams::ResetFiles() {
    if (!files_reset_flag) {
        files_reset_flag = true;
        if (raw_output_file.is_open()) {
            raw_output_file.close();
        }
        if (decoded_output_file.is_open()) {
            decoded_output_file.close();
        }
        if (eventcal_output_file.is_open()) {
            eventcal_output_file.close();
        }
        file_count = 0;
        write_raw_data_flag = false;
        write_decoded_events_flag = false;
        write_calibrated_events_flag = false;
    }
    return(0);
}


int ProcessParams::SetupFiles() {
    string local_filename_raw = filename_raw;
    string local_filename_decode = filename_decode;
    string local_filename_calibrate = filename_calibrate;

    if (split_files_flag) {
        local_filename_raw = Util::buildSplitFilename(
                filename_raw, file_count, 3);

        local_filename_decode = Util::buildSplitFilename(
                filename_decode, file_count, 3);

        local_filename_calibrate = Util::buildSplitFilename(
                filename_calibrate, file_count, 3);
    }

    if (write_raw_data_flag) {
        if (!raw_output_file.is_open()) {
            raw_output_file.open(local_filename_raw.c_str());
        }
        if (!raw_output_file.good()) {
            return(-4);
        }
    }

    if (write_decoded_events_flag) {
        if (!decoded_output_file.is_open()) {
            decoded_output_file.open(local_filename_decode.c_str());
        }
        if (!decoded_output_file.good()) {
            return(-5);
        }
    }

    if (write_calibrated_events_flag) {
        if (!eventcal_output_file.is_open()) {
            eventcal_output_file.open(local_filename_calibrate.c_str());
        }
        if (!eventcal_output_file.good()) {
            return(-6);
        }
    }
    return(0);
}

void ProcessParams::setRawFilename(const std::string & filename) {
    // Close all of the files and turn off writing to them until they have had
    // their filename set again.
    ResetFiles();
    filename_raw = filename;
    write_raw_data_flag = true;
    // Setup split filenames if need be, and then open up all of the files
    SetupFiles();
}

void ProcessParams::setDecodeFilename(const std::string & filename) {
    ResetFiles();
    filename_decode = filename;
    write_decoded_events_flag = true;
    SetupFiles();
}

void ProcessParams::setCalibratedFilename(const std::string & filename) {
    ResetFiles();
    filename_calibrate = filename;
    write_calibrated_events_flag = true;
    SetupFiles();
}
