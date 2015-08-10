#ifndef COMM_H
#define COMM_H

#include <vector>

/*! \enum Parse_Status 
 * describes the type of error encountered during the parsing of a received message 
 */
enum Parse_Status {
	PARSE_VALID, ///< The message is properly formatted and data can be properly extracted
	PARSE_NO_VALID_MSG, ///< The Message does not contain proper starting and ending characters
	PARSE_INVALID_FORMAT, ///< The message has missing or improperly placed signifiers within it
	PARSE_INCORRECT_ID, ///< The message contains an ID that could not properly be extracted
    PARSE_INVALID_RESPONSE_VAL ///< The message contained parameter values that could not be extracted
};

std::vector<char> ConstructCartridgePowerSetCommand(int Slot, bool power);
std::vector<char> ConstructCartridgePowerSetResponse(int Slot, bool power);
std::vector<char> ConstructCartridgePowerQueryCommand(int Slot);
std::vector<char> ConstructSCMicroQueryCommand();
std::vector<char> ConstructSCMicroQueryResponse(
        std::vector<char> TxVec,
        int SCMicrosSet);
std::vector<char> ConstructLCCommand(int SCMicroID, int ModuleID);
std::vector<char> ConstructTempCommand(int SCMicroID);
std::vector<char> ConstructTempRHCommand(int SCMicroID, char command);
std::vector<char> ConstructGainCommand(
        int SCMicroID,
        int ModuleID,
        bool PSAPD,
        int Cap);
std::vector<char> ConstructGainResponse(
        int SCMicroID,
        int ModuleID,
        bool PSAPD,
        int Cap);
std::vector<char> ConstructIDSetCommand(
        const std::vector<int> & SCMicroIDs);
std::vector<char> ConstructIDSetCommand();
std::vector<char> ConstructIDSetCommand(
        int DecrementStart,
        int abbreviated = 2);
std::vector<char> ConstructVoltageCommand(
        int CartridgeSlot,
        int DAC_Chip,
        int DAC_Channel,
        int Voltage);
std::vector<char> ConstructVoltageResponse(
        int CartridgeSlot,
        int DAC_Chip,
        int DAC_Channel,
        int Voltage);

int ParseTempResponse(
        const std::vector<char> & rxv,
        int & SCMicroID,
        int & HighNumber,
        int & LowNumber);
int ParseTempRHResponse(
        const std::vector<char> & rxv,
        int & SCMicroID,
        int & value);
int ParseLCResponse(
        const std::vector<char> & rxv,
        int & SCMicroID,
        int & LocalModuleID,
        int & Current);
int ParseCartridgePowerQueryResponse(
        const std::vector<char> & rxv,
        int & CartridgeSlot,
        bool &Status);
int ParseCartridgePowerSetResponse(
        const std::vector<char> & rxv,
        int & CartridgeSlot,
        bool &Status);
int ParseIDSetResponse(
        const std::vector<char> & rxv,
        const std::vector<char> & Command,
        int & SCMicrosSet);
int ParseVoltageResponse(
        const std::vector<char> & rxv,
        int & RxCartridgeSlot,
        int & RxDACID,
        int & RxDACAddress,
        int & RxDACVoltage);

int HexChar2Int(char c);
int Char2Int(char c);
char Int2HexChar(int val, bool uppercase=true);
char Int2Char(int val);

#endif /* COMM_H */
