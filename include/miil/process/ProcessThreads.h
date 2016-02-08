#ifndef PROCESS_THREADS_H
#define PROCESS_THREADS_H

#include <thread>
#include <string>
#include <vector>

class ProcessParams;
class ProcessControl;

class ProcessThreads {
    std::vector<ProcessParams *> process_params_vec;
    ProcessControl * const control;
    std::vector<std::thread> read_sockets_threads;
    std::vector<std::thread> process_data_threads;
    bool is_running;
    void stopProcessing(bool end_acquisition);
    void startProcessing();
    void stopReceiving();
    void startReceiving();

public:
    ProcessThreads(ProcessControl * const control_ptr);
    void addParams(ProcessParams * const process_params_ptr);
    void start();
    void stop(bool end_acquisition);
    void setRawFilename(const std::string & filename, int index);
    void setDecodeFilename(const std::string & filename, int index);
    void setCalibratedFilename(const std::string & filename, int index);
    bool isRunning() const;
};

#endif  // PROCESS_THREADS_H
