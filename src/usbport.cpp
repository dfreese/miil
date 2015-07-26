#include "usbport.h"
#include <iostream>

USBPort::USBPort() :
    totalSent(0),
    totalReceived(0),
    logReceived(false),
    logTransmitted(false),
    portState(false),
    simulation(false),
    recvLogHexMode(false),
    tranLogHexMode(false)
{
}

USBPort::~USBPort()
{
    if(recvLog.is_open()) {
        recvLog.close();
    }
    if(tranLog.is_open()) {
        tranLog.close();
    }
}

bool USBPort::setRecvLog(const std::string &recvLogFilename, bool hexMode)
{
    if (recvLog.is_open()) {
        recvLog.close();
    }

    if (recvLogFilename.empty()) {
        logReceived = false; // usbPort->setRecvLog("") stops logging.
        return false;
    }

    recvLog.open(recvLogFilename.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

    if (!recvLog.good()) {
		std::cout << "Cannot open file \"" << recvLogFilename << "\" for write operation." << std::endl;
        logReceived = false;
        return false;
    }
    logReceived = true;
    recvLogHexMode = hexMode;
    return true;
}

bool USBPort::setTranLog(const std::string &tranLogFilename, bool hexMode)
{
    if (tranLog.is_open()) {
        tranLog.close();
    }

    if (tranLogFilename.empty()) {
        logTransmitted = false; // usbPort->setRecvLog("") stops logging.
        return false;
    }

    tranLog.open(tranLogFilename.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

    if (!tranLog.good()) {
		std::cout << "Cannot open file \"" << tranLogFilename << "\" for write operation." << std::endl;
        logTransmitted = false;
        return false;
    }
    logTransmitted = true;
    tranLogHexMode = hexMode;
    return true;
}

