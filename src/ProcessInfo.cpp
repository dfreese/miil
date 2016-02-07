#include <miil/process/ProcessInfo.h>

ProcessInfo::ProcessInfo() :
    bytes_received(0),
    bytes_processed(0),
    decoded_events_processed(0),
    accepted_decode(0),
    accepted_calibrate(0),
    dropped_empty(0),
    dropped_start_stop(0),
    dropped_trigger_code(0),
    dropped_packet_size(0),
    dropped_address_byte(0),
    dropped_threshold(0),
    dropped_double_trigger(0),
    dropped_crystal_id(0),
    dropped_crystal_invalid(0),
    dropped_energy_gate(0),
    written_raw_bytes(0),
    written_decoded_events(0),
    written_calibrated_events(0),
    recv_calls_normal(0),
    recv_calls_zero(0),
    recv_calls_error(0),
    current_index(0),
    start_index(0),
    found_start(false)
{
}

std::ostream& operator<<(std::ostream& os, const ProcessInfo& info) {
    os << "bytes received: " << info.bytes_received << "\n"
       << "bytes processed: " << info.bytes_processed << "\n"
       << "Accepted Packets: " << info.accepted_decode << "\n"
       << "Dropped (Empty) : " << info.dropped_empty << "\n"
       << "Dropped (Start) : " << info.dropped_start_stop << "\n"
       << "Dropped (Trigg) : " << info.dropped_trigger_code << "\n"
       << "Dropped (Size)  : " << info.dropped_packet_size << "\n"
       << "Dropped (Addr)  : " << info.dropped_address_byte << "\n"
       << "\n"
       << "Events Processed: " << info.decoded_events_processed << "\n"
       << "Accepted Events        : " << info.accepted_calibrate << "\n"
       << "Dropped (Threshold)    : " << info.dropped_threshold << "\n"
       << "Dropped (Dbl Trigger)  : " << info.dropped_double_trigger << "\n"
       << "Dropped (Crystal Ident): " << info.dropped_crystal_id << "\n"
       << "Dropped (Crystal Valid): " << info.dropped_crystal_invalid << "\n"
       << "Dropped (Energy Gate)  : " << info.dropped_energy_gate << "\n"
       << "\n"
       << "Wrote (raw bytes)        : " << info.written_raw_bytes << "\n"
       << "Wrote (decoded events)   : " << info.written_decoded_events << "\n"
       << "Wrote (calibrated events): " << info.written_calibrated_events << "\n"
       << "\n"
       << "Receive Calls (Data)   : " << info.recv_calls_normal << "\n"
       << "Receive Calls (Zero)   : " << info.recv_calls_zero << "\n"
       << "Receieve Calls (Error) : " << info.recv_calls_error << "\n";
    return(os);
}
