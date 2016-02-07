#ifndef PROCESS_THREADS_H
#define PROCESS_THREADS_H

#include <thread>
#include <string>

class ProcessParams;
class ProcessControl;

class ProcessThreads {
    ProcessParams * const process_params;
    ProcessControl * const control;
    std::thread read_sockets_thread;
    std::thread process_data_thread;
    bool is_running;
    void stopProcessing(bool end_acquisition);
    void startProcessing();
    void stopReceiving();
    void startReceiving();

public:
    ProcessThreads(
            ProcessParams * const process_params_ptr,
            ProcessControl * const control_ptr);
    void start();
    void stop(bool end_acquisition);
    void setRawFilename(const std::string & filename);
    void setDecodeFilename(const std::string & filename);
    void setCalibratedFilename(const std::string & filename);
    bool isRunning() const;
};

#endif  // PROCESS_THREADS_H
