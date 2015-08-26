#ifndef DAQ_CONTROL_H
#define DAQ_CONTROL_H

#include <vector>
#include <miil/SystemConfiguration.h>

class DaqControl {
public:

    enum direct_fpga_commands {
        ADD_TO_BUFFER = 0x00,
        EXECUTE_INSTRUCTION = 0x40,
        RESET_BUFFER = 0x81,
        START_PACKET = 0x82,
        END_PACKET = 0x83,
        RESET_TIMESTAMP = 0x88
    };

    enum fpga_instructions {
        LOAD_RENA_SETTINGS = 0x5,
        COINC_OVERRIDE = 0x6,
        FORCE_TRIGGER = 0x7,
        TRIGGERS_NOT_TIMESTAMP = 0x8,
        ENABLE_READOUT = 0x9,
        LOAD_HIT_REGISTERS = 0xA,
        SET_COINC_LOGIC = 0xD
    };

    enum hit_register_types {
        TRIGGER_SET = 0x00,
        SLOW_HIT = 0x01,
        FAST_HIT = 0x02,
        UNDEFINED_HIT = 0x03
    };

    static int createRenaSettingsPacket(
            int backend_address,
            int daq_board,
            int fpga,
            int rena,
            const RenaChannelConfig & config,
            std::vector<char> & packet);

    static int createHitRegisterPacket(
            int backend_address,
            int daq_board,
            int fpga,
            int rena,
            int module,
            int register_type,
            RenaChannelConfig const * const configs,
            std::vector<char> & packet);

    static int createCoincOverridePacket(
            int backend_address,
            int daq_board,
            int fpga,
            bool enable,
            std::vector<char> & packet);

    static int createForceTriggerPacket(
            int backend_address,
            int daq_board,
            int fpga,
            bool enable,
            std::vector<char> & packet);

    static int createTriggerNotTimestampPacket(
            int backend_address,
            int daq_board,
            int fpga,
            bool enable,
            std::vector<char> & packet);

    static int createReadoutEnablePacket(
            int backend_address,
            int daq_board,
            int fpga,
            bool enable,
            std::vector<char> & packet);

    static int createCoincWindowPacket(
            int backend_address,
            int daq_board,
            int fpga,
            const BackendBoardConfig & config,
            std::vector<char> & packet);
};

#endif // DAQ_CONTROL_H
