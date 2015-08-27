#include <miil/daq_control.h>
#include <miil/util.h>

namespace {
char createAddress(
        int backend_address,
        int daq_board)
{
    char address = ((0x1F & backend_address) << 2) |
                   ((0x03 & daq_board) << 0);
    return(address);
}

char createExecuteInstruction(
        int instruction,
        int fpga)
{
    return(DaqControl::EXECUTE_INSTRUCTION | ((fpga & 0x3) << 4) | instruction);
}

int createBoolEnablePacket(
        int backend_address,
        int daq_board,
        int fpga,
        int instruction,
        bool enable,
        std::vector<char> & packet)
{
    packet.push_back(DaqControl::START_PACKET);
    packet.push_back(createAddress(backend_address, daq_board));
    packet.push_back(DaqControl::RESET_BUFFER);
    if (enable) {
        packet.push_back(DaqControl::ADD_TO_BUFFER | 0x01);
    } else {
        packet.push_back(DaqControl::ADD_TO_BUFFER | 0x00);
    }
    // Pad packet with zeros so that pauls board interprets it correctly
    for ( int i=0; i<6; i++) {
        packet.push_back(DaqControl::ADD_TO_BUFFER | 0x00);
    }
    packet.push_back(createExecuteInstruction(instruction, fpga));
    packet.push_back(DaqControl::END_PACKET);
    return(0);
}

/*! \brief Transforms RenaChannelConfig struct to a boolean vector
 *
 * This takes the structure of the RenaChannelConfig and transforms it into a
 * boolean vector of bits that can be sent to the rena after the channel address
 * is appended.  The order of the bits is placed into the vector is from MSB
 * to LSB which is the order expected by the Rena.
 *
 * \param settings settings for a channel on the rena
 * \param bitstream return of 35 bit vector for the rena channel settings
 *
 * \return 0 on success, less than otherwise.
 */
int createSettingsBitstream(
        const RenaChannelConfig & settings,
        std::vector<bool> & bitstream)
{
    bitstream.push_back(settings.feedback_resistor);
    bitstream.push_back(settings.test_enable);
    bitstream.push_back(settings.fast_powerdown);
    bitstream.push_back(settings.feedback_type);
    // Add two least significant bits
    std::vector<bool> add = Util::int2BoolVec(settings.gain, 2);
    bitstream.insert(bitstream.end(), add.begin(), add.end());
    bitstream.push_back(settings.powerdown);
    bitstream.push_back(settings.pole_zero_enable);
    bitstream.push_back(settings.feedback_cap);
    bitstream.push_back(settings.vref);
    // Add four least significant bits
    add = Util::int2BoolVec(settings.shaping_time, 4);
    bitstream.insert(bitstream.end(), add.begin(), add.end());
    bitstream.push_back(settings.fet_size);
    add = Util::int2BoolVec(settings.fast_daq_threshold, 8);
    bitstream.insert(bitstream.end(), add.begin(), add.end());
    bitstream.push_back(settings.polarity);
    add = Util::int2BoolVec(settings.slow_daq_threshold, 8);
    bitstream.insert(bitstream.end(), add.begin(), add.end());
    bitstream.push_back(settings.fast_trig_enable);
    bitstream.push_back(settings.slow_trig_enable);
    bitstream.push_back(settings.follower);
    return(0);
}

int createFullChannelSettingsBitstream(
    int rena,
    int channel,
    const RenaChannelConfig & settings,
    std::vector<bool> & bitstream)
{
    // Add Rena Select - 1 bit (1 bits total)
    bitstream.push_back(rena & 0x01);
    // Add Channel Address - 6 bits (7 bits total)
    std::vector<bool> channel_bool = Util::int2BoolVec(channel, 6);
    bitstream.insert(bitstream.end(), channel_bool.begin(), channel_bool.end());
    // Add Channel Settings - 35 bits (42 bits total)
    createSettingsBitstream(settings, bitstream);
    return(0);
}

int createFullChannelSettingsBuffer(
    int rena,
    int channel,
    const RenaChannelConfig & settings,
    std::vector<char> & bytestream)
{
    std::vector<bool> bitstream;
    createFullChannelSettingsBitstream(rena, channel, settings, bitstream);
    std::vector<uint8_t> buffer_vals =
            Util::BoolVec2ByteVec(bitstream, 6, true);
    for (size_t ii = 0; ii < buffer_vals.size(); ii++) {
        bytestream.push_back(DaqControl::ADD_TO_BUFFER | buffer_vals[ii]);
    }
    return(0);
}

int createHitRegisterBuffer(
        int rena,
        int module,
        int register_type,
        const std::vector<RenaChannelConfig *> & configs,
        std::vector<char> & packet)
{
    std::vector<bool> bitstream;
    for (size_t ii = 0; ii < configs.size(); ii++) {
        if (configs[ii]->module == module) {
            if (register_type == DaqControl::TRIGGER_SET) {
                bitstream.push_back(true);
            } else if (register_type == DaqControl::SLOW_HIT) {
                if (configs[ii]->slow_hit_readout) {
                    bitstream.push_back(true);
                } else {
                    bitstream.push_back(false);
                }
            } else if (register_type == DaqControl::FAST_HIT) {
                if (configs[ii]->fast_hit_readout) {
                    bitstream.push_back(true);
                } else {
                    bitstream.push_back(false);
                }
            } else if (register_type == DaqControl::UNDEFINED_HIT) {
                return(-1);
            } else {
                return(-2);
            }
        } else {
            bitstream.push_back(false);
        }
    }
    std::vector<uint8_t> buffer_vals =
            Util::BoolVec2ByteVec(bitstream, 6, true);
    for (size_t ii = 0; ii < buffer_vals.size(); ii++) {
        packet.push_back(DaqControl::ADD_TO_BUFFER | buffer_vals[ii]);
    }
    packet.push_back(DaqControl::ADD_TO_BUFFER |
                     ((rena & 0x01) << 5) |
                     ((register_type & 0x03) << 3) |
                     ((module & 0x03) << 0));
    return(0);
}
}

int DaqControl::createResetTimestampPacket(std::vector<char> & packet) {
    packet.push_back(RESET_TIMESTAMP);
    return(0);
}

int DaqControl::createRenaSettingsPacket(
        int backend_address,
        int daq_board,
        int fpga,
        int rena,
        int channel,
        const RenaChannelConfig & config,
        std::vector<char> & packet)
{
    packet.push_back(DaqControl::START_PACKET);
    packet.push_back(createAddress(backend_address, daq_board));
    packet.push_back(DaqControl::RESET_BUFFER);
    createFullChannelSettingsBuffer(rena, channel, config, packet);
    packet.push_back(createExecuteInstruction(
            DaqControl::LOAD_RENA_SETTINGS, fpga));
    packet.push_back(DaqControl::END_PACKET);
    return(0);
}

int DaqControl::createHitRegisterPacket(
        int backend_address,
        int daq_board,
        int fpga,
        int rena,
        int module,
        int register_type,
        const std::vector<RenaChannelConfig *> & configs,
        std::vector<char> & packet)
{
    packet.push_back(DaqControl::START_PACKET);
    packet.push_back(createAddress(backend_address, daq_board));
    packet.push_back(DaqControl::RESET_BUFFER);
    createHitRegisterBuffer(rena, module, register_type, configs, packet);
    packet.push_back(createExecuteInstruction(
            DaqControl::LOAD_HIT_REGISTERS, fpga));
    packet.push_back(DaqControl::END_PACKET);
    return(0);
}

int DaqControl::createCoincOverridePacket(
        int backend_address,
        int daq_board,
        int fpga,
        bool enable,
        std::vector<char> & packet)
{
    return(createBoolEnablePacket(
            backend_address,
            daq_board,
            fpga,
            COINC_OVERRIDE,
            enable,
            packet));
}

int DaqControl::createForceTriggerPacket(
        int backend_address,
        int daq_board,
        int fpga,
        bool enable,
        std::vector<char> & packet)
{
    return(createBoolEnablePacket(
            backend_address,
            daq_board,
            fpga,
            FORCE_TRIGGER,
            enable,
            packet));
}

int DaqControl::createTriggerNotTimestampPacket(
        int backend_address,
        int daq_board,
        int fpga,
        bool enable,
        std::vector<char> & packet)
{
    return(createBoolEnablePacket(
            backend_address,
            daq_board,
            fpga,
            TRIGGERS_NOT_TIMESTAMP,
            enable,
            packet));
}

int DaqControl::createReadoutEnablePacket(
        int backend_address,
        int daq_board,
        int fpga,
        bool enable,
        std::vector<char> & packet)
{
    return(createBoolEnablePacket(
            backend_address,
            daq_board,
            fpga,
            ENABLE_READOUT,
            enable,
            packet));
}

int DaqControl::createCoincWindowPacket(
        int backend_address,
        int daq_board,
        int fpga,
        const BackendBoardConfig &config,
        std::vector<char> &packet)
{
    packet.push_back(DaqControl::START_PACKET);
    packet.push_back(createAddress(backend_address, daq_board));
    packet.push_back(DaqControl::RESET_BUFFER);
    packet.push_back(DaqControl::ADD_TO_BUFFER | (config.coinc_window & 0x3f));
    packet.push_back(DaqControl::ADD_TO_BUFFER | (config.output_delay & 0x3f));
    packet.push_back(DaqControl::ADD_TO_BUFFER | (config.input_delay & 0x3f));
    // Pad packet with zeros so that pauls board interprets it correctly
    for (int i = 0; i < 4; i++) {
        packet.push_back(DaqControl::ADD_TO_BUFFER | 0x00);
    }
    packet.push_back(createExecuteInstruction(
            DaqControl::SET_COINC_LOGIC, fpga));
    packet.push_back(DaqControl::END_PACKET);
    return(0);
}
