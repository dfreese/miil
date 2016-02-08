#include <miil/process/ProcessThreads.h>
#include <miil/process/ProcessParams.h>
#include <miil/process/ProcessControl.h>

using namespace std;

// TODO: Make ProcessThreads handle multiple ProcessParams

ProcessThreads::ProcessThreads(ProcessControl * const control_ptr) :
    control(control_ptr),
    is_running(false)
{
}

void ProcessThreads::addParams(ProcessParams * const process_params_ptr) {
    process_params_vec.push_back(process_params_ptr);
    read_sockets_threads.emplace_back();
    process_data_threads.emplace_back();
}

void ProcessThreads::stopProcessing(bool end_acquisition) {
    control->end_of_acquisiton_flag = end_acquisition;
    control->process_data_flag = false;
    for (size_t ii = 0; ii < process_params_vec.size(); ii++) {
        if (process_data_threads[ii].joinable()) {
            process_data_threads[ii].join();
        }
    }
}

void ProcessThreads::startProcessing() {
    control->process_data_flag = true;
    control->end_of_acquisiton_flag = false;
    for (size_t ii = 0; ii < process_params_vec.size(); ii++) {
        ProcessParams * process_params = process_params_vec[ii];
        thread swap_thread(&ProcessParams::ProcessData, process_params);
        process_data_threads[ii].swap(swap_thread);
        if (swap_thread.joinable()) {
            swap_thread.join();
        }
    }
}

void ProcessThreads::stopReceiving() {
    control->read_sockets_flag = false;
    for (size_t ii = 0; ii < process_params_vec.size(); ii++) {
        if (read_sockets_threads[ii].joinable()) {
            read_sockets_threads[ii].join();
        }
    }
}

void ProcessThreads::startReceiving() {
    control->read_sockets_flag = true;
    for (size_t ii = 0; ii < process_params_vec.size(); ii++) {
        ProcessParams * process_params = process_params_vec[ii];
        thread swap_thread(&ProcessParams::ReadSockets, process_params);
        read_sockets_threads[ii].swap(swap_thread);
        if (swap_thread.joinable()) {
            swap_thread.join();
        }
    }
}

void ProcessThreads::start() {
    startProcessing();
    startReceiving();
    is_running = true;
}

void ProcessThreads::stop(bool end_acquisition) {
    stopReceiving();
    stopProcessing(end_acquisition);
    is_running = false;
}

void ProcessThreads::setRawFilename(
        const std::string & filename,
        int index)
{
    if (index < 0 || index >= process_params_vec.size()) {
        return;
    }
    bool was_running = isRunning();
    if (was_running) {
        stopProcessing(false);
    }
    process_params_vec[index]->setRawFilename(filename);
    if (was_running) {
        startProcessing();
    }
}

void ProcessThreads::setDecodeFilename(
        const std::string & filename,
        int index)
{
    if (index < 0 || index >= process_params_vec.size()) {
        return;
    }
    bool was_running = isRunning();
    if (was_running) {
        stopProcessing(false);
    }
    process_params_vec[index]->setDecodeFilename(filename);
    if (was_running) {
        startProcessing();
    }
}

void ProcessThreads::setCalibratedFilename(
        const std::string & filename,
        int index)
{
    if (index < 0 || index >= process_params_vec.size()) {
        return;
    }
    bool was_running = isRunning();
    if (was_running) {
        stopProcessing(false);
    }
    process_params_vec[index]->setCalibratedFilename(filename);
    if (was_running) {
        startProcessing();
    }
}

bool ProcessThreads::isRunning() const {
    return(is_running);
}
