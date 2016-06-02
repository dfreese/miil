#ifndef DAQ_CONTROL_H
#define DAQ_CONTROL_H

#include <vector>
#include <miil/SystemConfiguration.h>

class DaqControl {
public:
    /*!
     * Commands that are given to the FPGAs either full bytes, or ORed with
     * other information.
     */
    enum direct_fpga_commands {
        /*!
         * ORed with 6 bits (0b00xx xxxx) causing the 6 bits to be added to the
         * FPGA buffer and the buffer index incremented
         */
        ADD_TO_BUFFER = 0x00,
        /*!
         * ORed with 6 bits (0b00yy zzzz) where yy is a 2 bit FPGA id and zzzz
         * is a 4 bit instruction from fpga_instructions that causes the
         * instruction to be executed on the FPGA.
         */
        EXECUTE_INSTRUCTION = 0x40,
        /*!
         * Resets the buffer counter on the FPGAs.  Is sent by itself as a byte
         */
        RESET_BUFFER = 0x81,
        /*!
         * Represents the start of a packet to the backend FPGAs.  Is sent by
         * itself as a byte.  A backend board address is expected to follow.
         */
        START_PACKET = 0x82,
        /*!
         * Represents the end of a packet to the backend FPGAs.  Is sent by
         * itself as a byte.
         */
        END_PACKET = 0x83,
        /*!
         * Sent by itself as a byte to tell the system to reset the coarse
         * timestamp on every FPGA.  This should only be sent once to any
         * ethernet, as the reset logic is global.
         */
        RESET_TIMESTAMP = 0x88
    };

    enum fpga_instructions {
        /// Used by createRenaSettingsPacket to program a rena
        LOAD_RENA_SETTINGS = 0x5,
        /// Used by createCoincOverridePacket to program a FPGA
        COINC_OVERRIDE = 0x6,
        /// Used by createForceTriggerPacket to program a FPGA
        FORCE_TRIGGER = 0x7,
        /// Used by createTriggerNotTimestampPacket to program a FPGA
        TRIGGERS_NOT_TIMESTAMP = 0x8,
        /// Used by createReadoutEnablePacket to program a FPGA
        ENABLE_READOUT = 0x9,
        /// Used by createHitRegisterPacket to program a FPGA
        LOAD_HIT_REGISTERS = 0xA,
        /// Used by createCoincWindowPacket to program a FPGA
        SET_COINC_LOGIC = 0xD
    };

    /// The different register types accepted by createHitRegisterPacket
    enum hit_register_types {
        /// Ties rena channels to a module number for the FPGA
        TRIGGER_SET = 0x00,
        /// Tells the rena which energy channels to readout for a module trig
        SLOW_HIT = 0x01,
        /// Tells the rena which timing channels to readout for a module trig
        FAST_HIT = 0x02,
        /// A bit combination that is not programmed on the FPGA
        UNDEFINED_HIT = 0x03
    };

    static int createResetTimestampPacket(std::vector<char> & packet);

    static int createRenaSettingsPacket(
            int backend_address,
            int daq_board,
            int fpga,
            int rena,
            int channel,
            const RenaChannelConfig & config,
            std::vector<char> & packet);

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
            const std::vector<RenaChannelConfig *> & configs,
            std::vector<char> & packet);

    static int createHitRegisterPacket(
            int backend_address,
            int daq_board,
            int fpga,
            int rena,
            int module,
            int register_type,
            const ModuleChannelConfig & config,
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

    static int createCoincWindowPacket(
            int backend_address,
            int daq_board,
            int fpga,
            const FrontendFpgaConfig &config,
            std::vector<char> &packet);
};

#endif // DAQ_CONTROL_H
