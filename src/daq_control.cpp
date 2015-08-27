#include <miil/daq_control.h>
#include <miil/util.h>

namespace {
/*!
 * \brief Create backend address byte for specific daq board
 *
 * Takes a 5 bit backend address and 2 bit daq board id and converts it into an
 * address byte that can be interpreted by the backend boards.
 *
 * \param backend_address The 5 bit address hardwired on the board with jumpers
 * \param daq_board The 2 bit daq board id
 *
 * \return a byte that can be sent to the board after a start packet
 */
char createAddress(
        int backend_address,
        int daq_board)
{
    char address = ((0x1F & backend_address) << 2) |
                   ((0x03 & daq_board) << 0);
    return(address);
}

/*!
 * \brief Create a byte to execute an instruction on a particular FPGA
 *
 * \param instruction The specific 4 bit instruction, taken from \see
 *        fpga_instructions, to be executed on the specified fpga
 * \param fpga The 2 bit id of the FPGA on the daq board (four-up board).
 *
 * \return a byte to be sent to the board
 */
char createExecuteInstruction(
        int instruction,
        int fpga)
{
    return(DaqControl::EXECUTE_INSTRUCTION |
           ((fpga & 0x3) << 4) |
           ((instruction & 0x0F) << 0));
}

/*!
 * \brief Create a packet for an instruction needing only a bit flag
 *
 * There are several commands that require only a single bit flag to be placed
 * into the buffer of the FPGA.  This command takes care of that structure, and
 * the padding of the buffer on the FPGA with zeros to make those commands
 * function properly.  Only the first buffer slot is used, but the remaining six
 * are filled with zeros to make sure that the board functions properly.
 *
 * \param backend_address Address of the backend board to run command
 * \param daq_board 2 bit id of the daq board to run the command
 * \param fpga 2 bit id of the fpga to run the command
 * \param instruction 4 bit instruction taken from fpga_instructions
 * \param enable the enable bit to be placed in buffer(0)(0) on the fpga
 * \param packet the packet where the bytes are appended
 *
 * \return 0 if successful, less than otherwise
 */
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

/*!
 * \brief Creates a bit stream with rena, channel address, and settings
 *
 * Creates the 42 bit bitsream needed by \see createFullChannelSettingsBuffer to
 * program the rena channel.  The bitstream in order is the rena flag (1 bit),
 * the channel number (6 bits), and the channel settings (35 bits).
 *
 * \param rena The rena to be programmed by the fpga
 * \param channel The channel on the rena the settings are for
 * \param settings The settings for the rena channel to be programmed
 * \param bitstream boolean vector bitstream where bits are appended
 *
 * \return 0 if successful, less than otherwise
 */
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

/*!
 * \brief Creates a bytestream with rena, channel address, and settings
 *
 * The LOAD_RENA_SETTINGS instruction requires all 42 bits of the buffer on the
 * FPGA to be filled with the rena in buffer(6)(5), the channel in buffer(6)(4)
 * to buffer(5)(5), and the 35 bit rena settings bitstream in buffer(5)(4) to
 * buffer(0)(0).  Generate an appropriate bitstream with \see
 * createFullChannelSettingsBitstream, then configure that into ADD_TO_BUFFER
 * commands to the FPGA.
 *
 * \param rena The rena to be programmed by the fpga
 * \param channel The channel on the rena the settings are for
 * \param settings The settings for the rena channel to be programmed
 * \param bitstream boolean vector bitstream where bits are appended
 *
 * \return 0 if successful, less than otherwise
 */
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

/*!
 * \brief Append buffer add commands to a packet for a hit register instruction
 *
 * Generates the bytes to be put into the buffer for a LOAD_HIT_REGISTERS
 * instruction.  A bitstream is generated from the configs of channels 0 to 35.
 * A list of ones, the slow readout enable, or fast hit enable flags for the
 * TRIGGER_SET, SLOW_HIT, or FAST_HIT instructions respectively, are ANDed with
 * whether or not that channel is associated with the specified module.  The
 * bitstream is then placed into bytes.  This generates a buffer with channel
 * channel 0 at buffer(5)(5) down to channel 35 at buffer(0)(0).  The 1 bit
 * rena number is placed at buffer(6)(5).  The 2 bit instruction is placed at
 * buffer(6)(4) to buffer(6)(3).  The 2 bit module number is placed at
 * buffer(6)(1) to buffer(6)(0).  The bytes are then appended to the packet.
 *
 * \param rena Specify the rena the FPGA is dealing with
 * \param module Module on the rena the FPGA is dealing with
 * \param register_type A hit register type from \see hit_register_types
 * \param configs A vector of pointers to RenaChannelConfig representing the
 *        channels of the rena in order of their channel number
 * \param packet The vector of bytes where the buffer bytes should be appended
 *
 * \return 0 if successful, less than otherwise
 *         -1 if UNDEFINED_HIT is used, as it is not implemented
 *         -2 if an invalid register type is given
 */
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

/*!
 * \brief Generate a reset timestamp command
 *
 * Appends a RESET_TIMESTAMP command to the packet.
 *
 * \param packet vector where command is appended
 *
 * \return 0 if successful, less than otherwise
 */
int DaqControl::createResetTimestampPacket(std::vector<char> & packet) {
    packet.push_back(RESET_TIMESTAMP);
    return(0);
}

/*!
 * \brief Append a command to program a rena channel's settings
 *
 * Appends a packet to the given vector to program a particular channel on the
 * a rena with the given settings.  Uses \see createFullChannelSettingsBuffer to
 * generate all of the bytes to put information into the FPGA buffers.
 *
 * \param backend_address The address of the backend board to be addressed
 * \param daq_board The daq board on the backend board to be addressed
 * \param fpga The frontend fpga to be addressed
 * \param rena The rena number on the front end fpga to be programmed
 * \param channel The channel number on the rena to be programmed
 * \param config The settings with which to program the channel
 * \param packet vector where the bytes are appended
 *
 * \return 0 on success, less than otherwise
 *         -1 if createFullChannelSettingsBuffer fails
 */
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
    if (createFullChannelSettingsBuffer(rena, channel, config, packet) < 0) {
        return(-1);
    }
    packet.push_back(createExecuteInstruction(
            DaqControl::LOAD_RENA_SETTINGS, fpga));
    packet.push_back(DaqControl::END_PACKET);
    return(0);
}

/*!
 * \brief Append a command to program a rena channel's settings
 *
 * Appends a packet to the given vector to program the hit registers of the
 * FPGA.  This tells the FPGA which channels are associated with each module
 * (TRIGGER_SET) and which channels should be readout if it triggers (SLOW_HIT,
 * FAST_HIT).  It uses \see createHitRegisterBuffer to generate all of the bytes
 * to put information into the FPGA buffers.
 *
 * \param backend_address The address of the backend board to be addressed
 * \param daq_board The daq board on the backend board to be addressed
 * \param fpga The frontend fpga to be programmed
 * \param rena The rena number the fpga is being programmed for
 * \param module The module for which the rena is being programmed
 * \param register_type A hit register type from \see hit_register_types
 * \param configs Vector of channel settings used to program the fpga
 * \param packet vector where the bytes are appended
 *
 * \return 0 on success, less than otherwise
 *         -1 if createHitRegisterBuffer fails
 */
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
    if (createHitRegisterBuffer(
                rena, module, register_type, configs, packet) < 0) {
        return(-1);
    }
    packet.push_back(createExecuteInstruction(
            DaqControl::LOAD_HIT_REGISTERS, fpga));
    packet.push_back(DaqControl::END_PACKET);
    return(0);
}

/*!
 * \brief Append a command to program FPGA coincidence override flag
 *
 * Appends a packet to the given vector to set the coincidence override flag of
 * the FPGA.  True for enable disables the coincidence logic, putting the device
 * in singles mode.  False enables the coincidence logic programmed using \see
 * createCoincWindowPacket.
 *
 * \param backend_address The address of the backend board to be addressed
 * \param daq_board The daq board on the backend board to be addressed
 * \param fpga The frontend fpga to be programmed
 * \param packet vector where the bytes are appended
 *
 * \return the result createBoolEnablePacket
 */
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

/*!
 * \brief Append a command to program FPGA force trigger flag
 *
 * Appends a packet to the given vector to set the force trigger flag of the
 * FPGA.  When enabled, the FPGA triggers the rena at regular intervals to read
 * out the channel values when there is no signal (i.e. noise).
 *
 * \param backend_address The address of the backend board to be addressed
 * \param daq_board The daq board on the backend board to be addressed
 * \param fpga The frontend fpga to be programmed
 * \param packet vector where the bytes are appended
 *
 * \return the result createBoolEnablePacket
 */
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

/*!
 * \brief Append a command to program FPGA triggers not timestamp flag
 *
 * Appends a packet to the given vector to set the triggers not timestamp flag
 * of the FPGA.  When enabled, the FPGA replaces the timestamp with flags for
 * each of the channels on the rena with 1 indicating the channel triggered.
 * This is primarily a debugging tool.
 *
 * \param backend_address The address of the backend board to be addressed
 * \param daq_board The daq board on the backend board to be addressed
 * \param fpga The frontend fpga to be programmed
 * \param packet vector where the bytes are appended
 *
 * \return the result createBoolEnablePacket
 */
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

/*!
 * \brief Append a command to program the FPGA readout enable flag
 *
 * Appends a packet to the given vector to set the readout enable flag of the
 * FPGA.  When enabled, the FPGA will readout and send data in response to
 * trigger events on the rena.  False turns off readout in the system.
 *
 * \param backend_address The address of the backend board to be addressed
 * \param daq_board The daq board on the backend board to be addressed
 * \param fpga The frontend fpga to be programmed
 * \param packet vector where the bytes are appended
 *
 * \return the result createBoolEnablePacket
 */
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

/*!
 * \brief Append a command to program the FPGA coincidence logic
 *
 * Appends a packet to the given vector to set the values of the FPGA's
 * coincidence logic.  The values that are programmed are the width of the
 * coincidence window, the delay that the FPGA puts on its coincidence signal
 * output (output_delay), and the how long it should hold onto events before
 * discarding them as a single (input_delay).  Each value is 6 bits.  The
 * coincidence window is placed in buffer(0), output delay in buffer(1), and the
 * input delay in buffer(2).
 *
 * \param backend_address The address of the backend board to be addressed
 * \param daq_board The daq board on the backend board to be addressed
 * \param fpga The frontend fpga to be programmed
 * \param config The backend board configuration with the coinc window params
 * \param packet vector where the bytes are appended
 *
 * \return the result createBoolEnablePacket
 */
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
