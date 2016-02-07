#ifndef PROCESS_CONTROL_H
#define PROCESS_CONTROL_H

#include <atomic>

class ProcessControl {
    std::atomic_bool read_sockets_flag;
    std::atomic_bool process_data_flag;
    std::atomic_bool end_of_acquisiton_flag;
    friend class ProcessThreads;
    friend class ProcessParams;
public:
    std::atomic_bool write_data_flag;
    std::atomic_bool decode_events_flag;
    std::atomic_bool calibrate_events_flag;
    std::atomic_bool energy_gate_calibrated_events_flag;
    std::atomic_bool sort_calibrated_events_flag;
    ProcessControl();
};

#endif // PROCESS_CONTROL_H
