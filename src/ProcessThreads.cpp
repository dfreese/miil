#include <miil/process/ProcessThreads.h>
#include <miil/process/ProcessParams.h>
#include <miil/process/ProcessControl.h>

using namespace std;

// TODO: Make ProcessThreads handle multiple ProcessParams

ProcessThreads::ProcessThreads(
        ProcessParams * const process_params_ptr,
        ProcessControl * const control_ptr) :
    process_params(process_params_ptr),
    control(control_ptr),
    is_running(false)
{
}

void ProcessThreads::stopProcessing(bool end_acquisition) {
    control->end_of_acquisiton_flag = end_acquisition;
    control->process_data_flag = false;
    if (process_data_thread.joinable()) {
        process_data_thread.join();
    }
}

void ProcessThreads::startProcessing() {
    control->process_data_flag = true;
    control->end_of_acquisiton_flag = false;
    thread swap_thread(&ProcessParams::ProcessData, process_params);
    process_data_thread.swap(swap_thread);
    if (swap_thread.joinable()) {
        swap_thread.join();
    }
}

void ProcessThreads::stopReceiving() {
    control->read_sockets_flag = false;
    if (read_sockets_thread.joinable()) {
        read_sockets_thread.join();
    }
}

void ProcessThreads::startReceiving() {
    control->read_sockets_flag = true;
    thread swap_thread(&ProcessParams::ReadSockets, process_params);
    read_sockets_thread.swap(swap_thread);
    if (swap_thread.joinable()) {
        swap_thread.join();
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

void ProcessThreads::setRawFilename(const std::string & filename) {
    bool was_running = isRunning();
    if (was_running) {
        stopProcessing(false);
    }
    process_params->setRawFilename(filename);
    if (was_running) {
        startProcessing();
    }
}

void ProcessThreads::setDecodeFilename(const std::string & filename) {
    bool was_running = isRunning();
    if (was_running) {
        stopProcessing(false);
    }
    process_params->setDecodeFilename(filename);
    if (was_running) {
        startProcessing();
    }
}

void ProcessThreads::setCalibratedFilename(const std::string & filename) {
    bool was_running = isRunning();
    if (was_running) {
        stopProcessing(false);
    }
    process_params->setCalibratedFilename(filename);
    if (was_running) {
        startProcessing();
    }
}

bool ProcessThreads::isRunning() const {
    return(is_running);
}
