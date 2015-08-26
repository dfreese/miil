#include <miil/daq_control.h>

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
    packet.push_back(START_PACKET);
    packet.push_back(createAddress(backend_address, daq_board));
    packet.push_back(RESET_BUFFER);
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
    packet.push_back(END_PACKET);
    return(0);
}
}

int DaqControl::createCoincOverridePacket(
        int backend_address,
        int daq_board,
        int fpga,
        bool enable,
        std::vector<char> & packet)
{

}

int DaqControl::createForceTriggerPacket(
        int backend_address,
        int daq_board,
        int fpga,
        bool enable,
        std::vector<char> & packet)
{

}

int DaqControl::createTriggerNotTimestampPacket(
        int backend_address,
        int daq_board,
        int fpga,
        bool enable,
        std::vector<char> & packet)
{

}

int DaqControl::createReadoutEnablePacket(
        int backend_address,
        int daq_board,
        int fpga,
        bool enable,
        std::vector<char> & packet)
{

}
