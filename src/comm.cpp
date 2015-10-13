/*! \file      comm.cpp Functions for communicating with the Slow Control System
 *  \details   This file contains all of the necessary functions to communicate
 *             with the different Slow Control systems.
 *  \author    David Freese
 */

#include <miil/comm.h>
#include <stdio.h>
#include <string>
#include <miil/util.h>

/*! \brief Make a command to turn on a Cartridge Slot on the HV Floating Board
 *
 * This constructs a vector of characters that can be sent over a usb interface
 * to the HV Floating Board to specify that a slot should be turned on or off.
 *
 * \param Slot The slot on the HV Floating Board that the power command is
 *                specified for, not the Cartridge Number
 * \param power Specifies whether the set command will set the power on or off.
 *                (false=off) (true=off)
 *
 * \return Vector of charcters to be sent over usb interface
 */
std::vector<char> ConstructCartridgePowerSetCommand(int Slot, bool power) {
    // HV Floating Supply Board has 3 Slots
    // Check if Slot is within range, return empty vector otherwise
    if ((Slot < 0) || (Slot > 2)) {
        return (std::vector<char>());
    }

    std::vector<char> ret;
    ret.reserve(6);

    ret.push_back('<');
    ret.push_back('P');

    ret.push_back('0'+Slot);

    if (power) ret.push_back('1');
    else ret.push_back('0');

    ret.push_back('>');
    ret.push_back(' ');

    return(ret);
}

/*! \brief Constructs the expected response to a sent CartridgePowerSet Command
 *
 * This constructs a vector of characters that represents the expected response
 * of the HV Floating Board from a successful power set.  This can be used to
 * verify the actual response received matches this so that the program can
 * validate that the slot was actually set
 *
 * \param Slot The slot on the HV Floating Board that the power command is
 *             specified for, not the Cartridge Number
 * \param power Specifies whether the set command will set the power on or off.
 *              (false=off) (true=off)
 *
 * \return Vector of charcters that should be received after sending set command
 */
std::vector<char> ConstructCartridgePowerSetResponse(int Slot, bool power) {
    // HV Floating Supply Board has 3 Slots
    // Check if Slot is within range, return empty vector otherwise
    if ((Slot<0) || (Slot>2)) {
        return (std::vector<char>());
    }

    std::vector<char> ret;
    ret.reserve(6);

    ret.push_back('<');

    if (power) ret.push_back('1');
    else ret.push_back('0');

    ret.push_back('p');


    ret.push_back('0'+Slot);

    ret.push_back('>');
    ret.push_back(' ');

    return(ret);
}

/*! \brief Constructs command to check if a Cartridge Slot is on
 *
 * This constructs a vector of characters that can be sent over a usb interface
 * to the HV Floating Board to query the current state of that slot's power.
 *
 * \param Slot The slot on the HV Floating Board that will be queried
 *
 * \return Vector of charcters to be sent over usb interface
 */
std::vector<char> ConstructCartridgePowerQueryCommand(int Slot) {
    // HV Floating Supply Board has 3 Slots
    // Check if Slot is within range, return empty vector otherwise
    if ((Slot < 0) || (Slot > 2) ) {
        return (std::vector<char>());
    }

    std::vector<char> ret;
    ret.reserve(6);

    ret.push_back('<');
    ret.push_back('?');

    ret.push_back('0'+Slot);

    ret.push_back('>');
    ret.push_back(' ');

    return(ret);
}

/*! \brief Constructs command to query the IDs of SCMicros within the daisychain
 *
 * This constructs a vector of characters that can be sent over a usb interface
 * to the SCMicros the order in which the IDs have been set.
 *
 * \return Vector of charcters to be sent over usb interface
 */
std::vector<char> ConstructSCMicroQueryCommand() {
    std::vector<char> ret(6);
    sprintf(&(ret[0]),"<UQ> ");
    ret.pop_back();
    return(ret);
}

/*! \brief Constructs command to check module leakage current
 *
 * This constructs a vector of characters that can be sent over a usb interface
 * to a discrete board, or SCMicro, to query the leakage current of a specific
 * module that is connected to the SCMicro.
 *
 * \param SCMicroID The ID of the SCMicro that should be queried
 * \param ModuleID The ID of the module local to the SCMicro that should be
 *                 measured.
 *
 * \return Vector of charcters to be sent over usb interface
 */
std::vector<char> ConstructLCCommand(int SCMicroID, int ModuleID) {
    if ((SCMicroID < 0) || (ModuleID < 0) || (ModuleID > 7)) {
        return (std::vector<char>());
    }
    std::vector<char> ret(9);
    // X is replaced later by scmicro specific function
    sprintf(&(ret[0]), "<UXIM%1d> ", ModuleID);

    char id(Int2Char(SCMicroID));

    if (id < 0) {
        return (std::vector<char>());
    } else {
        ret[2] = id;
    }

    ret.pop_back();
    return(ret);
}

/*! \brief Constructs command to request temperature of the connected thermistor
 *
 * This constructs a vector of characters that can be sent over a usb interface
 * to a Discrete Board microcontroller, known as an SCMicro, to query the
 * temperature of the thermistor that is connected.
 *
 * \warning Hard Coded SCMicro ID Range
 * \warning Only half of the SCMicros have a thermistor connected and will
 *          return a valid output
 *
 * \param SCMicroID the ID number of the SCMicro that is being addressed
 *
 * \return Vector of charcters to be sent over usb interface
 */
std::vector<char> ConstructTempCommand(int SCMicroID) {
    if (SCMicroID < 0) {
        return (std::vector<char>());
    }

    char id(Int2Char(SCMicroID));

    if (id < 0) {
        return (std::vector<char>());
    } else {
        std::vector<char> ret(6);
        ret[0] = '<';
        ret[1] = 'U';
        ret[2] = id;
        ret[3] = 'T';
        ret[4] = '>';
        ret[5] = ' ';
        return(ret);
    }
}

/*! \brief Constructs command to request temperature or humidity from chip
 *
 * This constructs a vector of characters that can be sent over a usb interface
 * to a Discrete Board microcontroller, known as an SCMicro, to query the
 * temperature or humidity of a connected sensor chip.
 *
 * \warning Hard Coded SCMicro ID Range
 * \warning Only some of the SCMicros will have a connected chip
 *
 * \param SCMicroID the ID number of the SCMicro that is being addressed
 * \param command The type of command to send to the micro. 'T' or 'H' are valid
 *                and request temperature and humidity respectively.
 *
 * \return Vector of charcters to be sent over usb interface
 */
std::vector<char> ConstructTempRHCommand(int SCMicroID, char command) {
    if (SCMicroID < 0) {
        return (std::vector<char>());
    }
    if ((command != 'T') && (command != 'H')) {
        return (std::vector<char>());
    }

    char id(Int2Char(SCMicroID));

    if (id < 0) {
        return (std::vector<char>());
    } else {
        std::vector<char> ret(7);
        ret[0] = '<';
        ret[1] = 'U';
        ret[2] = id;
        ret[3] = 'H';
        ret[4] = command;
        ret[5] = '>';
        ret[6] = ' ';
        return(ret);
    }
}

/*! \brief Constructs command to set the gain on the common of the PSAPD
 *
 * This constructs a vector of characters that can be sent over a usb interface
 * to a Discrete Board microcontroller to set the programmable capacitor to a
 * specified value so that the gain to common channel can be adjusted.
 *
 * \warning Hard Coded SCMicro ID Range
 * \warning Though responing correctly Discrete Boards may not be setting values
 *          of the capacitors correctly.
 * \warning New boards will probably not have programmable capacitors.
 *
 * \param SCMicroID the ID number of the SCMicro that is being addressed
 * \param ModuleID the local ID of the Module that is being addressed
 * \param PSAPD which PSAPD on the module should be addressed
 * \param Cap the value at which the capacitor should be programmed.
 *            0 through 31 is accepted
 *
 * \return Vector of charcters to be sent over usb interface.  An empty vector
 *         for invalid values.
 */
std::vector<char> ConstructGainCommand(
        int SCMicroID,
        int ModuleID,
        bool PSAPD,
        int Cap)
{
    if ((SCMicroID < 0) ||
        (ModuleID < 0) || (ModuleID > 7) ||
        (Cap < 0) || (Cap > 31))
    {
        return (std::vector<char>());
    }
    std::vector<char> ret(12);
    sprintf(&(ret[0]),"<U%1XG%02dM%1d%1d> ",SCMicroID,Cap,ModuleID,(int) PSAPD);
    ret.pop_back();
    return(ret);
}

/*! \brief Constructs the expected response to the gain set command
 *
 * This constructs a vector of characters that can compared to the response that
 * is returned from the Discrete board microcontroller, known as an SCMicro,
 * to set the programmable capacitor to a specified value so that the gain to
 * common channel can be adjusted
 *
 * \warning Hard Coded Module ID Range
 * \warning The send command requires a zero padded gain number, but the return
 *          may not be
 *
 * \param SCMicroID the ID number of the SCMicro that is being addressed
 * \param ModuleID the local ID of the Module that is being addressed
 * \param PSAPD which PSAPD on the module should be addressed
 * \param Cap the value at which the capacitor should be programmed.  0 through
 *            31 is accepted
 *
 * \return Vector of charcters for comparison.  An empty vector for invalid
 *         values.
 */
std::vector<char> ConstructGainResponse(
        int SCMicroID,
        int ModuleID,
        bool PSAPD,
        int Cap)
{
    if ( (SCMicroID < 0) ||
         (ModuleID <0) || (ModuleID >7) ||
         (Cap <0) || (Cap >31))
    {
        return (std::vector<char>());
    }
    std::vector<char> ret(12);
    sprintf(&(ret[0]),"<g%du%1Xm%1d%1d> ",Cap,SCMicroID,ModuleID,(int) PSAPD);
    ret.pop_back();
    return(ret);
}

/*! \brief Make chain of commands to set micro ids in specific order
 *
 * The discrete boards are programmed with <US#######> command.  The
 * microcontroller takes the first value off of the chain as its ID and send the
 * command, sans that ID to the next microcontroller. If there is only one
 * number left, then this number is decremented in in ASCII form and then the
 * command is sent to the next microcontroller in the chain.  If there are no
 * numbers supplied, then the ID is assumed to be 'F' by the first
 * microcontroller in the chain, and it then provides the appropritely
 * decremented command of '\<USE\> ' to the next micro.
 *
 * \param SCMicroIDs a vector of IDs in which the chain of micros should be set.
 *
 * \return Vector of charcters to send over the usb interface. An empty vector
 *         for invalid values.
 */
std::vector<char> ConstructIDSetCommand(const std::vector<int> & SCMicroIDs) {
    std::vector<char> ret(5+SCMicroIDs.size());
    ret[0] = '<';
    ret[1] = 'U';
    ret[2] = 'S';
    for (size_t i=0; i<SCMicroIDs.size(); i++) {
        char id(Int2Char(SCMicroIDs[i]));
        if (id < 0) {
            return(std::vector<char>());
        } else {
            ret[i+3] = id;
        }
    }
    *(ret.end()-2) = '>';
    *(ret.end()-1) = ' ';

    return(ret);
}

/*! \brief Constructs command to begin set ID autodecrement starting with 'F'
 *
 * Overloaded function that uses ConstructIDSetCommand(std::vector<int>) to
 * construct '\<US\> ' which begins settings the microcontroller IDs at 'F' and
 * then begins to autodecrement from there
 *
 * \return Vector of charcters to send over the usb interface.  An empty vector
 *         for invalid values.
 */
std::vector<char> ConstructIDSetCommand(){
    return(ConstructIDSetCommand(std::vector<int>()));
}

/*! \brief Create ID autodecrement command starting with a specified number
 *
 * Overloaded function that uses ConstructIDSetCommand(std::vector<int>) to
 * construct '<US#> ' which begins settings the microcontroller IDs at a
 * specified number and then begins to autodecrement from there.
 *
 * \param DecrementStart an ID at which the decrement chain will begin.
 * \param abbreviated if 2 (default) only put the first ID and let the daisy
 *        chain do the decrementing, if 0 list out all the IDs.  1 lists out all
 *        of the ids down to 'P' to avoid the conflicts with 'Q' and 'S' and
 *        then let's the decrement chain on the micros take over.
 *
 * \return Vector of charcters to send over the usb interface.  An empty vector
 *         for invalid values.
 */
std::vector<char> ConstructIDSetCommand(int DecrementStart, int abbreviated) {
    if (abbreviated == 2) {
        return(ConstructIDSetCommand(std::vector<int>(1, DecrementStart)));
    } else if (abbreviated == 1) {
        // If the decrement start id is above Q, then specify down to just
        // below the kludge to avoid Q and S, so that the daisy chain takes
        // over after that.
        int normal_op_threshold(Char2Int('P'));
        int difference(DecrementStart - normal_op_threshold);
        if (difference > 0) {
            return(ConstructIDSetCommand(
                    Util::BuildOrderedVector(difference+1,
                                             false,
                                             DecrementStart)));
        } else {
            // Operate like normal abbreviate == 2 mode
            return(ConstructIDSetCommand(std::vector<int>(1, DecrementStart)));
        }
    } else if (abbreviated == 0) {
        return(ConstructIDSetCommand(
                Util::BuildOrderedVector(DecrementStart+1, false)));
    } else {
        return(std::vector<char>());
    }
}

/*! \brief Constructs command to set the DAC voltage on the HVBias board
 *
 * This constructs a command which is used to set a the voltage that is supplied
 * by the DAC chips on the HVBiasBoard to the specified Module.  This is a
 * positive offset from the negative floating ground that is supplied by the
 * Crate through the HVFloatingBoard.
 *
 * \param CartridgeSlot the slot number on the HVFloatingBoard to which the
 *                      Cartridge is connected. Values of slot numbers are 0, 1,
 *                      and 2.
 * \param DAC_Chip the number of the DAC chip that contains the Module whos
 *                 voltage is to be programmed. Each DAC Chip can program up to
 *                 32 modules, so each DAC chip controls 2 Fins. Chip numbers
 *                 between 0 and 3 are accepted.
 * \param DAC_Channel The channel on which the Module to be programmed is
 *                    connected. There are 32 channels per DAC Chip so 0 through
 *                    31 are acceptable numbers.
 * \param Voltage The voltage that is to be added to the negative floating
 *                ground for the Module. Voltages betwen 0 and 99 are accepted.
 *
 * \return Vector of charcters to send over the usb interface.  An empty vector
 *         for invalid values.
 */
std::vector<char> ConstructVoltageCommand(
        int CartridgeSlot,
        int DAC_Chip,
        int DAC_Channel,
        int Voltage)
{
    if ((CartridgeSlot < 0) || (CartridgeSlot > 2)) {
        return(std::vector<char>());
    }
    if ((DAC_Chip < 0) || (DAC_Chip > 3)) {
        return(std::vector<char>());
    }
    if ((DAC_Channel < 0) || (DAC_Channel > 31)) {
        return(std::vector<char>());
    }
    if ((Voltage < 0) || (Voltage > 99)) {
        return(std::vector<char>());
    }

    std::vector<char> ret(13);
    sprintf(&(ret[0]),
            "<C%1dU%1dV%02d%02d> ",
            CartridgeSlot,
            DAC_Chip,
            DAC_Channel,
            Voltage);
    ret.pop_back();
    return(ret);
}

/*! \brief Constructs expected response to command to set the DAC voltage
 *
 * This constructs an expected response to a successful command to set the
 * voltage that is supplied by the DAC chips on the HVBiasBoard to the specified
 * Module.
 *
 * \param CartridgeSlot the slot number on the HVFloatingBoard to which the
 *                      Cartridge is connected. Accepted values of slot numbers
 *                      are 0, 1, and 2.
 * \param DAC_Chip the number of the DAC chip that contains the Module whos
 *                 voltage is to be programmed. Each DAC Chip can program up to
 *                 32 modules, so each DAC chip controls 2 Fins. Chip numbers
 *                 between 0 and 3 are accepted.
 * \param DAC_Channel The channel on which the Module to be programmed is
 *                    connected. There are 32 channels per DAC Chip so 0 through
 *                    31 are acceptable numbers.
 * \param Voltage The voltage that is to be added to the negative floating
 *                ground for the Module. Voltages betwen 0 and 99 are accepted.
 *
 * \return Vector of charcters to for comparison to response.  An empty vector
 *         for invalid values.
 */
std::vector<char> ConstructVoltageResponse(
        int CartridgeSlot,
        int DAC_Chip,
        int DAC_Channel,
        int Voltage)
{
    if ((CartridgeSlot < 0) || (CartridgeSlot > 2)) {
        return (std::vector<char>());
    }
    if ((DAC_Chip < 0) || (DAC_Chip > 3)) {
        return (std::vector<char>());
    }
    if ((DAC_Channel < 0) || (DAC_Channel > 31)) {
        return (std::vector<char>());
    }
    if ((Voltage < 0) || (Voltage > 99)) {
        return (std::vector<char>());
    }

    std::vector<char> ret(13);
    sprintf(&(ret[0]),"<v%02d%02du%1dc%1d> ",
            DAC_Channel, Voltage, DAC_Chip, CartridgeSlot);
    ret.pop_back();
    return(ret);
}

/*! \brief Parses a received response to a temperature query command
 *
 * This parses the response that is received in response to a request for the
 * temperature on a thermistor connected to a Discrete Board, or SCMicro.  This
 * is done by verifying the message is in the proper format, with opening and
 * closing brackets, proper placement of the parameters within the message,
 * including 'u' for the microcontroller ID, and 't' for the start of the
 * temperature. If the message is verified to be in the proper format, then the
 * SCMicroID, HighNumber, and LowNumber are modified with the values that the
 * message contained.  These can then be parsed into physical numbers using the
 * calibration file.
 *
 * \warning If the given vector of characters contains more than one message,
 *          an error will most likely be produced rather than decoding any
 *          message.
 *
 * \param rxv The received vector of characters to be parsed
 * \param SCMicroID Where the ID of the sendign SCMicro will be placed
 * \param HighNumber Where the high number from the thermistor chip is returned
 * \param LowNumber Where the low number from the thermistor chip is returned
 *
 * \return An integer value from Parse_Status indicating a successful parse or
 *         the type of error
 */
int ParseTempResponse(
        const std::vector<char> & rxv,
        int & SCMicroID,
        int & HighNumber,
        int & LowNumber)
{
    if (rxv.empty()) {
        return(PARSE_NO_VALID_MSG);
    }
    std::vector<char>::const_iterator openmsg = rxv.end();
    std::vector<char>::const_iterator closemsg = rxv.end();
    std::vector<char>::const_iterator tempsym = rxv.end();
    std::vector<char>::const_iterator microsym = rxv.end();

    for (std::vector<char>::const_iterator it=rxv.begin();
         it!=rxv.end();
         ++it)
    {
        switch(*it){
            case '<':
                openmsg = it;
                break;
            case '>':
                closemsg = it;
                break;
            case 't':
                tempsym = it;
                break;
            case 'u':
                microsym = it;
                break;
            default:
                break;
        }
    }

    if ((openmsg == rxv.end()) ||
            (closemsg == rxv.end()) ||
            (tempsym == rxv.end()) ||
            (microsym == rxv.end()))
    {
        return(PARSE_NO_VALID_MSG);
    }

    if ((tempsym != (openmsg + 1)) ||
            (microsym != (tempsym + 7)) ||
            (closemsg != (microsym + 2)) )
    {
        return(PARSE_INVALID_FORMAT);
    }

    int id_value(Char2Int((*(microsym+1))));
    if (id_value < 0) {
        return(PARSE_INCORRECT_ID);
    }

    std::string tempstring((tempsym+1), microsym);
    unsigned int temp_high_num;
    unsigned int temp_low_num;
    if (sscanf(tempstring.c_str(),
               "%3x%3x",
               &temp_high_num,
               &temp_low_num) != 2)
    {
        return PARSE_INVALID_RESPONSE_VAL;
    }

    HighNumber = temp_high_num;
    LowNumber = temp_low_num;
    SCMicroID = id_value;
    return PARSE_VALID;
}

/*! \brief Parses a received response to a TempRH query command
 *
 * This parses the response that is received from a request for the temperature
 * or relative humidity measured by the sensor chip connected to a SCMicro.  The
 * message format is verified (i.e. opening and closing brackets, proper
 * placement of the parameters within the message, including 'u' for the
 * microcontroller ID, and 't' or 'h' for the start of the temperature or rh. If
 * the message is verified to be in the proper format, then the SCMicroID and
 * value are modified with the values that the message contained.  value can
 * then be parsed into physical numbers using the calibration file.
 *
 * \warning If the given vector of characters contains more than one message,
 *          an error will most likely be produced rather than decoding any
 *          message.
 *
 * \param rxv The received vector of characters to be parsed
 * \param SCMicroID Where the ID of the sending SCMicro will be placed
 * \param value The sensor hex value taken from the message
 *
 * \return An integer value from Parse_Status
 */
int ParseTempRHResponse(
        const std::vector<char> & rxv,
        int & SCMicroID,
        int & value)
{
    // Bail on an empty vector
    if (rxv.empty()) {
        return(PARSE_NO_VALID_MSG);
    }

    // Attempt to find all of the identifying characters in the message
    std::vector<char>::const_iterator openmsg = rxv.end();
    std::vector<char>::const_iterator closemsg = rxv.end();
    std::vector<char>::const_iterator tempsym = rxv.end();
    std::vector<char>::const_iterator microsym = rxv.end();

    for (std::vector<char>::const_iterator it=rxv.begin();
         it!=rxv.end();
         ++it)
    {
        switch(*it){
            case '<':
                openmsg = it;
                break;
            case '>':
                closemsg = it;
                break;
            case 't':
                tempsym = it;
                break;
            case 'h':
                tempsym = it;
                break;
            case 'u':
                microsym = it;
                break;
            default:
                break;
        }
    }

    // Check that all of the requisite characters were found
    if ((openmsg == rxv.end()) ||
            (closemsg == rxv.end()) ||
            (tempsym == rxv.end()) ||
            (microsym == rxv.end()))
    {
        return(PARSE_NO_VALID_MSG);
    }

    // Check that the characters are in the right spots
    if ((tempsym != (openmsg+1)) ||
            (microsym != (tempsym + 5)) ||
            (closemsg != (microsym + 2)))
    {
        return(PARSE_INVALID_FORMAT);
    }

    // Check that the ID within the message is valid
    int id_value(Char2Int((*(microsym+1))));
    if (id_value < 0) {
        return(PARSE_INCORRECT_ID);
    }

    // Check that we can correctly decode the hex value from the stream
    std::string tempstring((tempsym+1),microsym);
    unsigned int temp_value;
    if (sscanf(tempstring.c_str(), "%4x", &temp_value) != 1) {
        return(PARSE_INVALID_RESPONSE_VAL);
    }

    SCMicroID = id_value;
    value = temp_value;
    return(PARSE_VALID);
}

/*! \brief Parses a received response to a leakage current query command
 *
 * This parses the response that is received in response to a request for the
 * leakage current on a module connected to a Discrete Board, or SCMicro.  This
 * is done by verifying the message is in the proper format, with opening and
 * closing brackets, proper placement of the parameters within the message,
 * including 'u' for the microcontroller ID, 'm' for the module number and 'i'
 * for the start of the leakage current. If the message is verified to be in the
 * proper format, then the SCMicroID, LocalModuleID, and Current variables
 * supplied to the function are modified with the values that the message
 * contained.  These can then be converted into physical numbers using the
 * calibration file.
 *
 * \warning If rxv contains more than one message, the end message will likely
 *          be parsed, or errors will be produced.
 *
 * \param rxv The received vector of characters to be parsed
 * \param SCMicroID Where the ID of the sending SCMicro will be placed
 * \param LocalModuleID Where the ID of the module addressed will be placed
 * \param Current Where the ADC value of the current will be placed
 *
 * \return An integer value from Parse_Status indicating a successful parse or
 *         the type of error
 */
int ParseLCResponse(
        const std::vector<char> & rxv,
        int & SCMicroID,
        int & LocalModuleID,
        int & Current)
{
    if (rxv.empty()) {
        return(PARSE_NO_VALID_MSG);
    }
    std::vector<char>::const_iterator openmsg = rxv.end();
    std::vector<char>::const_iterator closemsg = rxv.end();
    std::vector<char>::const_iterator currentsym = rxv.end();
    std::vector<char>::const_iterator microsym = rxv.end();
    std::vector<char>::const_iterator modulesym = rxv.end();

    for (std::vector<char>::const_iterator it=rxv.begin();
         it!=rxv.end();
         ++it)
    {
        switch(*it){
            case '<':
                openmsg = it;
                break;
            case '>':
                closemsg = it;
                break;
            case 'i':
                currentsym = it;
                break;
            case 'u':
                microsym = it;
                break;
            case 'm':
                modulesym = it;
                break;
            default:
                break;
        }
    }

    if ((openmsg == rxv.end()) ||
            (closemsg==rxv.end()) ||
            (currentsym==rxv.end()) ||
            (microsym==rxv.end()) ||
            (modulesym==rxv.end()))
    {
        return(PARSE_NO_VALID_MSG);
    }

    if ((currentsym != (openmsg + 1)) ||
         ((microsym != (currentsym + 2)) &&
          (microsym != (currentsym + 3)) &&
          (microsym != (currentsym + 4))) ||
         (modulesym != (microsym + 2)) ||
         (closemsg != (modulesym + 2)))
    {
        return(PARSE_INVALID_FORMAT);
    }

    int id_value(Char2Int((*(microsym + 1))));
    if (id_value < 0) {
        return(PARSE_INCORRECT_ID);
    }

    int local_mod_id_value(HexChar2Int(*(modulesym + 1)));
    if (local_mod_id_value < 0) {
        return(PARSE_INCORRECT_ID);
    }


    std::string currentstring((currentsym+1),microsym);
    if (sscanf(currentstring.c_str(), "%x", &Current) !=1 ) {
        return(PARSE_INVALID_RESPONSE_VAL);
    }

    SCMicroID = id_value;
    LocalModuleID = local_mod_id_value;
    return(PARSE_VALID);
}

/*! \brief Parses a received response to a cartridge power query command
 *
 * This parses the response that is received in response to a request for the
 * status of power to a cartridge slot that is on an HVFloatingBoard.  This is
 * done by verifying the message is in the proper format, with opening and
 * closing brackets, proper placement of the parameters within the message,
 * including '?' for the query command, which is followed by the cartridge slot
 * number. If the message is verified to be in the proper format, then the
 * CartridgeSlot and Status variables supplied to the function are modified with
 * the values that the message contained.
 *
 * \warning If rxv contains more than one message, the end message will likely
 *          be parsed, or errors will be produced.
 *
 * \param rxv The received vector of characters to be parsed
 * \param CartridgeSlot Where the Slot Number of a Valid message is returned
 * \param Status Where the status of a valid message will be returned
 *
 * \return An integer value from Parse_Status indicating a successful parse or
 *         the type of error
 */
int ParseCartridgePowerQueryResponse(
        const std::vector<char> & rxv,
        int & CartridgeSlot,
        bool &Status)
{
    if (rxv.empty()) {
        return(PARSE_NO_VALID_MSG);
    }
    std::vector<char>::const_iterator openmsg=rxv.end();
    std::vector<char>::const_iterator closemsg=rxv.end();
    std::vector<char>::const_iterator slotsym=rxv.end();

    for (std::vector<char>::const_iterator it = rxv.begin();
         it != rxv.end();
         ++it)
    {
        switch(*it){
            case '<':
                openmsg = it;
                break;
            case '>':
                closemsg = it;
                break;
            case '?':
                slotsym = it;
                break;
            default:
                break;
        }
    }

    if ((openmsg == rxv.end()) ||
            (closemsg ==rxv.end()) ||
            (slotsym ==rxv.end()))
    {
        return(PARSE_NO_VALID_MSG);
    }

    if ((slotsym != (openmsg + 2)) || (closemsg != (slotsym + 2))) {
        return(PARSE_INVALID_FORMAT);
    }

    int powerval = (*(openmsg+1)-'0');
    if ((powerval==0) || (powerval==1)) {
        Status = (bool) powerval;
        CartridgeSlot = (*(slotsym+1)-'0');
        return(PARSE_VALID);
    } else {
        return(PARSE_INVALID_RESPONSE_VAL);
    }
}

/*! \brief Parses a received response to a cartridge power set command
 *
 * This parses the response that is received in response to a command to set the
 * power to a cartridge slot that is on an HVFloatingBoard.  This is done by
 * verifying the message is in the proper format, with opening and closing
 * brackets, proper placement of the parameters within the message, including
 * 'p' for the power set command, which is followed by the cartridge slot
 * number. If the message is verified to be in the proper format, then the
 * CartridgeSlot and Status variables supplied to the function are modified with
 * the values that the message contained.
 *
 * \warning If the given vector of characters contains two messages or more than
 *          just a single message, the key characters from the end of the vector
 *          will be selected and an error will most likely be produced.
 *
 * \param rxv The received vector of characters to be parsed
 * \param CartridgeSlot The integer where the Slot Number of a Valid message is
 *                      placed.
 * \param Status The bool in which the status of a valid message will be
 *               returned.
 *
 * \return An integer value from Parse_Status indicating a successful parse or
 *         the type of error
 */
int ParseCartridgePowerSetResponse(
        const std::vector<char> & rxv,
        int & CartridgeSlot,
        bool &Status)
{
    if (rxv.empty()) {
        return(PARSE_NO_VALID_MSG);
    }
    std::vector<char>::const_iterator openmsg = rxv.end();
    std::vector<char>::const_iterator closemsg = rxv.end();
    std::vector<char>::const_iterator slotsym = rxv.end();

    for (std::vector<char>::const_iterator it = rxv.begin();
         it!=rxv.end();
         ++it)
    {
        switch(*it){
            case '<':
                openmsg = it;
                break;
            case '>':
                closemsg = it;
                break;
            case 'p':
                slotsym = it;
                break;
            default:
                break;
        }
    }

    if ((openmsg == rxv.end()) ||
            (closemsg ==rxv.end()) ||
            (slotsym ==rxv.end()))
    {
        return(PARSE_NO_VALID_MSG);
    }

    if ((slotsym != (openmsg + 2)) || (closemsg != (slotsym + 2))) {
        return(PARSE_INVALID_FORMAT);
    }

    int powerval = (*(openmsg+1)-'0');
    if ((powerval==0) || (powerval==1)) {
        Status = (bool) powerval;
        CartridgeSlot = (*(slotsym+1)-'0');
        return(PARSE_VALID);
    } else {
        return(PARSE_INVALID_RESPONSE_VAL);
    }
}

/*! \brief Parses the response to a set ID command based on the command sent
 *
 * This parses the response to the set ID command that should be in the form of
 * '<US#> ' where # represents the ASCII decremented version of the last ID set.
 * '#' could also be a series of IDs (or just a single ID) remaining from a
 * command explicitly setting SCMicro IDs within the chain.  This takes those
 * variables into account and after verifying the formatting of the message
 * (proper placement of '<', 'U', 'S', '>', and ' ') then it extracts the number
 * of SCMicros that had their ID set by seeing how many IDs in an explicit
 * change were set first, and then identifying how many IDs were set through the
 * auto-decrement on the microcontroller
 *
 * \warning This command expects the response to be the only thing contained
 *          within rxv with nothing before or after it.
 *
 * \param rxv The received vector of characters to be parsed
 * \param Command The command that was sent to the SCMicro chain to set the ids
 * \param SCMicrosSet The integer that is set with the number of
 *        microcontrollers that were set in response to the command given a
 *        valid command
 *
 * \return An integer value from Parse_Status indicating a successful parse or
 *         the type of error
 */
int ParseIDSetResponse(
        const std::vector<char> & rxv,
        const std::vector<char> & Command,
        int & SCMicrosSet)
{
    std::vector<char> response = rxv;
    if (!response.empty()) {
        if (response.back() == '>') {
            response.push_back(' ');
        }
    }
    //Verify Response is valid
    if (response.size() < 6) {
        // Not valid response length
        return(PARSE_NO_VALID_MSG);
    }
    // Look at parameters from the beginning of the msg
    std::vector<char>::const_iterator it = response.begin();
    if ( ((*it) != '<') || (*(it+1) != 'U') || (*(it+2) != 'S') ) {
        return(PARSE_INVALID_FORMAT);
    }
    // Go to the end of the message and verify characters from there
    it = response.end()-1;
    if ( ((*it) != ' ') || (*(it-1) != '>') ) return(PARSE_INVALID_FORMAT);

    std::vector<char> EquivCommand = Command;
    // A command without an ID specified is interpreted by the micro to be
    // equivalent to an 'F' for an ID.
    if (Command.size() == 5) {
        EquivCommand.insert(EquivCommand.begin()+3,'F');
    }
    if (response.size() == 6) {
        // Single ID number contained in message.  Represents the last id
        // set minus 1. Check to see if any order was specified and if so,
        // how many of those IDs were set
        SCMicrosSet = (int) (EquivCommand.size()-response.size());
        // Check to see how many times the last id given by set command was
        // auto-decremented
        char equiv_last_id = *(EquivCommand.end()-3);
        char recv_last_id = *(response.end()-3);

        int send_id_val = Char2Int(equiv_last_id);
        int recv_id_val = Char2Int(recv_last_id);
        if (send_id_val < 0) {
            return(PARSE_INCORRECT_ID);
        }
        if (recv_id_val < -1) {
            return(PARSE_INCORRECT_ID);
        }
        SCMicrosSet += send_id_val - recv_id_val;
    } else {
        // Not every ID specified was set. The number of remaining
        // characters represents the number of IDs not set.
        SCMicrosSet = (int) (EquivCommand.size()-response.size());
    }
    return (PARSE_VALID);
}


/*! \brief Constructs the expected response to the SCMicro ID query command
 *
 * Using the supplied set command vector and the response with the number of IDs
 * that were set during the processing of that command, this constructs an
 * expected response from the chain of SCMicro microcontrollers.  This chain is
 * based on explictly set IDs being set first, and then any remaining IDs being
 * set using the auto-decrement function on the microcontrollers.
 *
 * \param TxVec The vector containing the set command that was transmitted to
 *              the micro chain
 * \param SCMicrosSet The number of SCMicros IDs that were set, as determined by
 *                    ParseIDSetResponse(const std::vector<char>,
 *                                       const std::vector<char>&, int&)
 *
 * \return A vector of char that constitutes the expected response from the
 *         micro chain
 */
std::vector<char> ConstructSCMicroQueryResponse(
        std::vector<char> TxVec,
        int SCMicrosSet)
{
    if (TxVec.size() <5) {
        return std::vector<char>();
    }
    // "<US> " is equivalent to "<USF> "
    if (TxVec.size() == 5) {
        TxVec.insert(TxVec.begin()+3,'F');
    }

    std::vector<char> ret;
    ret.reserve(6*SCMicrosSet+5);
    // Construct the responses based on the explicitly set IDs first
    for (int i=0; (i<((int)TxVec.size()-5)) && (i<SCMicrosSet); i++) {
        ret.push_back('<');
        ret.push_back('q');
        ret.push_back('u');
        ret.push_back(TxVec[3+i]);
        ret.push_back('>');
        ret.push_back(' ');
    }

    // In case decrementing was used to set microcontroller IDs after last
    // specified one
    // First Decremented ID to be used

    int LastID(Char2Int(*(TxVec.end()-3)));
    if (LastID < 0) {
        return std::vector<char>();
    }
    int FirstDecID(LastID - 1);

    for (int i=0; i<(SCMicrosSet-(TxVec.size()-5)); i++) {
        ret.push_back('<');
        ret.push_back('q');
        ret.push_back('u');
        ret.push_back(Int2Char(FirstDecID-i));
        ret.push_back('>');
        ret.push_back(' ');
    }
    // Add the Query Command that is passed through each of the microcontollers
    // and then back to the USB port
    ret.push_back('<');
    ret.push_back('U');
    ret.push_back('Q');
    ret.push_back('>');
    ret.push_back(' ');
    return(ret);
}

/*! \brief Converts single hex character to an integer
 *
 * Accepts an ASCII character from '0' to '9', 'a' to 'f', or 'A' to 'F'
 * and converts it into an integer.  Anything outside this range is regected
 *
 * \param c The ASCII character to be converted
 *
 * \return An Integer value that is represented by the ASCII character (0 to 15)
 *         -1 to indicate an invalid value
 */
int HexChar2Int(char c) {
    if ( (c >= '0') && (c <= '9') ) return((int)(c-'0'));
    else if ( (c >= 'A') && (c <= 'F') ) return((int)(c-'A')+10);
    else if ( (c >= 'a') && (c <= 'f') ) return((int)(c-'a')+10);
    else return(-1);
}

/*! \brief Converts single character to an integer for Discrete Board IDs
 *
 * Accepts an ASCII character from '0' to '9' and above 'A' and convets it to an
 * integer.  Anything char outside these ranges is regected.  This is mainly for
 * the slow control microcontroller's processing of characters.
 *
 * \param c The ASCII character to be converted
 *
 * \return An Integer value that is represented by the ASCII character within
 *         the slow control microor -1 to
           indicate an invalid value
 */
int Char2Int(char c) {
    if (c == '!') {
        // This can be considered a legitimate id, as it is what is passed after
        // the last id is set.
        return(-1);
    } else if (c < '\"') {
        // The microcontroller code can go below '0', and we assume that
        // '\"' is used as zero
        return(-2);
    } else if (c <= '9') {
        // Numbers convert assuming '\"' is 0
        return((int)(c - '\"'));
    } else if (c < 'A') {
        // The symbols between '9' and 'A' are to be considered a dead zone, as
        // the micro code was setup to treat the characters as a hex character
        // and would skip past these ascii values
        return(-3);
    } else if (c < 'Q') {
        // Return the value of the char assuming '\"' is zero
        // and accomodating dead zone betwen '9' and 'A'.
        return((int)(c - '\"' - 7));
    } else if (c == 'Q') {
        // Q is an invalid id because it causes command conflicts
        return(-4);
    } else if (c < 'S') {
        // Return the value of the char assuming '\"' is zero,
        // accomodating the dead zone betwen '9' and 'A', and 'Q' being invalid.
        return((int)(c - '\"' - 8));
    } else if (c == 'S') {
        // S is an invalid id because it causes command conflicts
        return(-5);
    } else {
        // Return the value of the char assuming '\"' is zero,
        // accomodating the dead zone betwen '9' and 'A', and 'Q' and 'S' being
        // invalid.
        return((int)(c - '\"' - 9));
    }
}

/*! \brief Converts an integer into an ASCII Character
 *
 * Accepts an integer from 0 to 15 and converts to an ASCII character.
 * 0 to 9 is converted to '0' to '9', and 10 to 15 is converted to either
 * 'a' to 'f' or 'A' to 'F' depending if uppercase is false or true respectively
 *
 * \param val The integer value to be converted.  0 to 15 is accepted
 * \param uppercase An optional flag to tell the function to return uppercase
 *                  or lowercase hex values.  The default value is true, or
 *                  uppercase.
 *
 * \return An ASCII character representing the hex of the integer. -1 is
 *         returned for integers out of range.
 */
char Int2HexChar(int val, bool uppercase) {
    if ( (val<0) || (val>15) ) return(-1);
    if (val < 10) return('0'+val);
    else if (uppercase) return('A'+val-10);
    else return('a'+val-10);
}


/*! \brief Converts an integer into an ASCII Character for Discrete Board IDs
 *
 * Accepts an integer above 0 and converts to an ASCII character within the
 * range that is usable by the slow control microcontrollers. For this range:
 * 0 to 9 is converted to '0' to '9', and above 10 is converted to 'A'-10+val.
 *
 * \param val The integer value to be converted.  0 to 15 is accepted
 *
 * \return An ASCII character representing the integer. -1 is returned for
           integers out of range.
 */
char Int2Char(int val) {
    // Assume now that the quotation character " (0x22) is the 0 value
    if (val < 0) {
        return(-1);
    } else {
        char id = '\"' + val;
        // The microcontrollers skip from 'A' down to '9', so include this dead
        // zone.
        if (id > '9') {
            // 7 == 'A' - ':'
            id += 7;
        }
        // Skip Q to avoid conflicts with commands like <UQ> when making a
        // command like "<UQT> " which wouldn't read temperature, but start a
        // query chain
        if (id >= 'Q') {
            id += 1;
        }
        // Skip S to avoid conflicts with commands like "<UST> " which wouldn't
        // read temperature, but start mess up all of the ids instead
        // query chain
        if (id >= 'S') {
            id += 1;
        }
        return(id);
    }
}

/*! \brief Parses a received response to a set DAC voltage command
 *
 * This parses the response that is received in response to a command to set the
 * DAC voltage on a module connected to a specific DAC chip on an HV Bias Board.
 * This is done by verifying the message is in the proper format, with opening
 * and closing brackets, proper placement of the parameters within the message,
 * including 'c' for the cartridge slot, 'u' for the DAC Chip ID, and 'v' for
 * the voltage. If the message is verified to be in the proper format, then the
 * RxCartridgeSlot, RxDACID, RxDACAddress, and RxDACVoltage, variables supplied\
 * to the function are modified with the values that the message contained.
 *
 * \warning If the given vector of characters contains two messages or more than
 *          just a single message, the key characters from the end of the vector
 *          will be selected and an error will most likely be produced.
 *
 * \param rxv The received vector of characters to be parsed
 * \param RxCartridgeSlot Where the ID of the cartridge slot will be placed
 * \param RxDACID Where the ID of the DAC Chip addressed will be placed
 * \param RxDACAddress Where the Address of the DAC chip set will be placed
 * \param RxDACVoltage Where the voltage set by the DAC will be placed
 *
 * \return An integer value from Parse_Status indicating a successful parse or
 *         the type of error
 */
int ParseVoltageResponse(
        const std::vector<char> & rxv,
        int & RxCartridgeSlot,
        int & RxDACID,
        int & RxDACAddress,
        int & RxDACVoltage)
{
    if (rxv.empty()) {
        return(PARSE_NO_VALID_MSG);
    }
    std::vector<char>::const_iterator openmsg = rxv.end();
    std::vector<char>::const_iterator closemsg = rxv.end();
    std::vector<char>::const_iterator voltagesym = rxv.end();
    std::vector<char>::const_iterator microsym = rxv.end();
    std::vector<char>::const_iterator cartridgesym = rxv.end();

    for (std::vector<char>::const_iterator it=rxv.begin(); it!=rxv.end(); ++it)
    {
        switch(*it){
            case '<':
                openmsg = it;
                break;
            case '>':
                closemsg = it;
                break;
            case 'v':
                voltagesym = it;
                break;
            case 'u':
                microsym = it;
                break;
            case 'c':
                cartridgesym = it;
                break;
            default:
                break;
        }
    }

    if ((openmsg == rxv.end()) ||
            (closemsg == rxv.end()) ||
            (voltagesym == rxv.end()) ||
            (microsym == rxv.end()) ||
            (cartridgesym == rxv.end()))
    {
        return(PARSE_NO_VALID_MSG);
    }

    if ((voltagesym != (openmsg + 1)) ||
            (microsym != (voltagesym + 5)) ||
            (cartridgesym != (microsym + 2)) ||
            (closemsg != (cartridgesym +2)))
    {
        return(PARSE_INVALID_FORMAT);
    }

    int TempVoltage;
    std::string voltagestring((voltagesym+3),microsym);
    if (sscanf(voltagestring.c_str(), "%d", &TempVoltage) != 1) {
        return(PARSE_INVALID_RESPONSE_VAL);
    }

    int TempDACAddress;
    std::string AddressString((voltagesym+1),(voltagesym+3));
    if (sscanf(AddressString.c_str(), "%d", &TempDACAddress) != 1) {
        return(PARSE_INVALID_RESPONSE_VAL);
    }

    RxCartridgeSlot = HexChar2Int(*(cartridgesym + 1));
    RxDACID = HexChar2Int(*(microsym + 1));
    RxDACAddress = TempDACAddress;
    RxDACVoltage = TempVoltage;

    return(PARSE_VALID);
}
