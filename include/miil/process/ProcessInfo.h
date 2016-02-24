#ifndef PROCESSINFO_H
#define PROCESSINFO_H

#include <iostream>

class ProcessInfo {
    size_t current_index;
    size_t start_index;
    bool found_start;
    friend class ProcessParams;
public:
    ProcessInfo();
    void reset();
    std::string getDecodeInfo();

    long bytes_received;
    long bytes_transferred;
    long bytes_processed;
    long decoded_events_processed;
    long accepted_decode;
    long accepted_calibrate;
    long dropped_empty;
    long dropped_start_stop;
    long dropped_trigger_code;
    long dropped_packet_size;
    long dropped_address_byte;
    long dropped_threshold;
    long dropped_double_trigger;
    long dropped_crystal_id;
    long dropped_crystal_invalid;
    long dropped_energy_gate;

    long written_raw_bytes;
    long written_decoded_events;
    long written_calibrated_events;

    long recv_calls_normal;
    long recv_calls_zero;
    long recv_calls_error;
};

std::ostream& operator<<(std::ostream& os, const ProcessInfo& info);

#endif // PROCESSINFO_H
