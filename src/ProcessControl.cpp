#include <miil/process/ProcessControl.h>

ProcessControl::ProcessControl() {
    read_sockets_flag = true;
    process_data_flag = true;
    end_of_acquisiton_flag = false;
    write_data_flag = false;
    decode_events_flag = false;
    calibrate_events_flag = false;
    energy_gate_calibrated_events_flag = false;
    sort_calibrated_events_flag = false;
}
