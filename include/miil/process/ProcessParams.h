#ifndef PROCESS_PARAMS_H
#define PROCESS_PARAMS_H

#include <fstream>
#include <deque>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <miil/BoundedBuffer.h>
#include <miil/EventRaw.h>
#include <miil/EventCal.h>
#include <miil/process/ProcessInfo.h>

class ProcessControl;
class Ethernet;
class SystemConfiguration;

class ProcessParams {
    Ethernet * ethernet;
    SystemConfiguration const * const system_config;
    ProcessControl * const control;
    ProcessInfo info;
    //! A mutex locked copy that is updated outside of the main thread loops
    ProcessInfo locked_info;
    std::mutex lock_locked_info;
    long assumed_max_delay;
    float energy_gate_low;
    float energy_gate_high;
    std::ofstream raw_output_file;
    std::ofstream decoded_output_file;
    std::ofstream eventcal_output_file;
    std::vector<char> buffer_receive_side;
    BoundedBuffer<char> buffer_transfer;
    std::deque<char> buffer_process_side;
    std::vector<EventRaw> decoded_data;
    std::vector<EventCal> calibrated_data;
    std::string filename_raw;
    std::string filename_decode;
    std::string filename_calibrate;
    bool split_files_flag;
    size_t file_size_max;
    int file_count;
    bool write_raw_data_flag;
    bool write_decoded_events_flag;
    bool write_calibrated_events_flag;
    bool files_reset_flag;
    size_t current_file_size;

    void updateProcessInfo();
    int DecodeBuffer(size_t write_to_position);
    int ClearProcessedData();
    int HandleData(bool write_out_remaining_cal_data);
    int ResetFiles();
    int SetupFiles();
    static int no_instances;

public:
    ProcessParams(
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
            size_t max_file_size = 524288000);
    int ProcessData();
    int ReadSockets();
    int ReadWriteSockets();
    void setRawFilename(const std::string & filename);
    void setDecodeFilename(const std::string & filename);
    void setCalibratedFilename(const std::string & filename);
    ProcessInfo getProcessInfo();
    void resetProcessInfo();
    BoundedBuffer<char> raw_storage;
    BoundedBuffer<EventRaw> decoded_storage;
    BoundedBuffer<EventCal> calibrated_storage;

    static int DecodeBuffer(
            size_t write_to_position,
            std::deque<char> & buffer_process_side,
            std::vector<EventRaw> & decoded_data,
            ProcessInfo & info,
            SystemConfiguration const * const system_config);
    static int DecodeBuffer(
            std::deque<char> & buffer_process_side,
            std::vector<EventRaw> & decoded_data,
            ProcessInfo & info,
            SystemConfiguration const * const system_config);
    static int ClearProcessedData(
            std::deque<char> & buffer_process_side,
            ProcessInfo & info);
    static int CalibrateBuffer(
            const std::vector<EventRaw> & decoded_data,
            std::vector<EventCal> & calibrated_data,
            ProcessInfo & info,
            SystemConfiguration const * const config);
    static int IDBuffer(
            const std::vector<EventRaw> & decoded_data,
            std::vector<EventCal> & calibrated_data,
            ProcessInfo & info,
            SystemConfiguration const * const config);
};

#endif // PROCESS_PARAMS_H
