#include <miil/usbport2.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <miil/util.h>
#include <unistd.h>
#include <sys/time.h>

USBPort2::USBPort2() {
}

USBPort2::USBPort2(const int &portNumber) {
    openPort(portNumber);
}

USBPort2::~USBPort2() {
}

bool USBPort2::getDeviceList(std::vector<std::string> &list) {
    DWORD numDevs(0);
    FT_STATUS ftStatus;
    ftStatus = FT_CreateDeviceInfoList(&numDevs);
    if (ftStatus != FT_OK) {
        std::cerr << "FT_CreateDeviceInfoList failed!" << std::endl;
        return(false);
    }
    FT_DEVICE_LIST_INFO_NODE *devInfo = new FT_DEVICE_LIST_INFO_NODE[numDevs];
    ftStatus = FT_GetDeviceInfoList(devInfo,&numDevs);
    if (ftStatus != FT_OK) {
        std::cerr << "FT_GetDeviceInfoList failed!" << std::endl;
        delete[] devInfo;
        return(false);
    }
    list.clear();
    for (int ii = 0; ii < (int)numDevs; ii++) {
        std::string description(devInfo[ii].Description);
        list.push_back(description);
    }
    delete[] devInfo;
    return(true);
}

int USBPort2::getQueSize() {
    DWORD bytesAvailable(0);
    if (FT_GetQueueStatus(ftHandle, &bytesAvailable) != FT_OK) {
        return(-1);
    }
    return(bytesAvailable);
}

bool USBPort2::openPort(const int &portNumber) {
    return(openPort(portNumber, FT_BAUD_9600));
}

bool USBPort2::openPort(const int &portNumber, const int baud) {
    FT_STATUS ftStatus(FT_Open(portNumber, &ftHandle));
    if (ftStatus != FT_OK) {
        std::cerr << "FT_Open failed." << std::endl;
        portState = false;
        return(false);
    }

    ftStatus = FT_OK;
    ftStatus += FT_SetBaudRate(ftHandle, baud);
    ftStatus += FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0x00, 0x00);
    ftStatus += FT_SetDataCharacteristics(ftHandle, FT_BITS_8,
                                          FT_STOP_BITS_1, FT_PARITY_NONE);
    ftStatus += FT_Purge(ftHandle, FT_PURGE_TX | FT_PURGE_RX);
    ftStatus += FT_SetTimeouts(ftHandle, 50, 50);
    if (ftStatus != FT_OK) {
        std::cerr << "Port settings failed." << std::endl;
        portState = false;
        return(false);
    }

    portState = true;
    return(true);
}

bool USBPort2::Open() {
    return(Open(interfacename, baudrate));
}

void USBPort2::closePort() {
    if (FT_Close(ftHandle) == FT_OK) {
        portState = false;
    }
}

bool USBPort2::Open(const std::string & name, int baud) {
    interfacename = name;
    baudrate = baud;

    if (isOpen()) {
        return(true);
    }

    std::vector<std::string> list;
    getDeviceList(list);
    int usbDev(-1);
    for (size_t ii = 0; ii < list.size(); ii++) {
        if(strcmp(list[ii].c_str(), interfacename.c_str()) == 0) {
            usbDev = ii;
        }
    }

    if (usbDev < 0) {
        // Handle Device Not Found
        std::cerr << "Error: Device not found: " << interfacename << std::endl;
        return(false);
    }

    if (!openPort(usbDev, baud)) {
        std::cerr << "Error: Cannot open port: " << interfacename << std::endl;
        return(false);
    }
    purge(true, true);
    return(true);
}

int USBPort2::send(const std::vector<char> &sendBuff)
{
    int status = 0;
    for (unsigned int ii=0; ii<sendBuff.size(); ii++) {
        status = send(sendBuff[ii]);
        if (status != FT_OK) {
            return status;
        }
    }
    return FT_OK;

    // The code below should be functionally equivalent to the above, however
    // the communication with the microcontrollers tends to error out with it
    // so that will be something that needs to be investigated.  Problably a
    // timing issue.
//    std::vector<char> writeable(sendBuff);
//    DWORD bytesWritten;
//    if (FT_Write(ftHandle, &writeable[0], writeable.size(), &bytesWritten) != FT_OK) {
//        return(-2);
//    } else if (bytesWritten != writeable.size()) {
//        return(-1);
//    } else {
//        totalSent++;
//        return(FT_OK);
//    }
}

int USBPort2::send(const char &c) {
    char byte = c;
    DWORD bytesWritten;
    if (FT_Write(ftHandle, &byte, 1, &bytesWritten) != FT_OK) {
        return(-2);
    }
    if (bytesWritten != 1) {
        return(-1);
    }
    totalSent++;
    return(FT_OK);
}

int USBPort2::send(const std::string & str) {
    std::vector<char> buffer;
    for (size_t ii = 0; ii < str.size(); ii++) {
        buffer.push_back(str[ii]);
    }
    return(send(buffer));
}

int USBPort2::recv(char &c) {
    DWORD bytesAvailable(0);
    int queue_status = FT_GetQueueStatus(ftHandle, &bytesAvailable);
    if (queue_status != FT_OK) {
        return(-1);
    }
    if (bytesAvailable <= 0) {
        return(0);
    }
    DWORD bytesReceived(0);
    int read_status(FT_Read(ftHandle, &c, 1, &bytesReceived));
    if (read_status != FT_OK) {
        std::cerr << "ERROR: FT_IO_ERROR in USBPort2::recv(char&)"
                  << std::endl;
        return(-2);
    }
    totalReceived += bytesReceived;
    return(bytesReceived);
}

int USBPort2::recv(std::vector<char> &rxv, char endchar, float timeout_s) {
    // Structure to hold the current time within the function
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    // timeend is the structure that holds the time that the function should
    // timeout at.  Find this by creating the structure with the timeout as
    // an offset, and then add the current time to it.
    int sec = (int) timeout_s;
    int usec = (int)(1e6 * (timeout_s - sec));
    struct timeval time_end = {sec, usec};
    // Setup a time in the future to stop collecting characters
    timeradd(&current_time, &time_end, &time_end);

    bool end_char_found(false);
    int totalrecv(0);
    while ((timercmp(&current_time, &time_end, <=)) && (!end_char_found)) {
        char c(0);
        int numrecv(recv(c));
        // Check for an Error in receive
        if (numrecv < 0) {
            return(-1);
        }
        totalrecv += numrecv;
        if (numrecv > 0){
            // Add character to vector
            rxv.push_back(c);
            if (c == endchar) {
                end_char_found = true;
            }
        } else {
            // Wait microsecond to not loop constantly
            usleep(1);
        }
        gettimeofday(&current_time, NULL);
    }
    return(totalrecv);
}

int USBPort2::recv(std::vector<char> & response, long waitns) {
    Util::sleep_ns(waitns);
    return recv(response);
}

int USBPort2::recv(std::vector<char> &recvBuff) {
    DWORD bytesAvailable(0);
    DWORD bytesReceived(0);
    if (FT_GetQueueStatus(ftHandle, &bytesAvailable) != FT_OK) {
        return(-1);
    }
    if (bytesAvailable <= 0) {
        return(0);
    }
    recvBuff.resize(bytesAvailable);

    if (FT_Read(ftHandle, &recvBuff[0], recvBuff.size(), &bytesReceived)
            != FT_OK)
    {
        return(-1);
    }
    totalReceived += bytesReceived;
    return(bytesReceived);
}

int USBPort2::recv(char *buffer, int numBytes) {
    DWORD bytesReceived(0);
    DWORD bytesAvailable(0);
    if (FT_GetQueueStatus(ftHandle, &bytesAvailable) != FT_OK) {
        return(-1);
    }
    if ((DWORD)numBytes > bytesAvailable) {
        numBytes = bytesAvailable;
    }
    if (FT_Read(ftHandle, buffer, numBytes, &bytesReceived) != FT_OK) {
        return(-1);
    }
    totalReceived += bytesReceived;
    return(bytesReceived);
}

bool USBPort2::purge(bool rxBuff, bool txBuff) {
    FT_STATUS ftStatus = FT_OK;
    if (rxBuff) {
        ftStatus += FT_Purge(ftHandle, FT_PURGE_RX);
    }
    if (txBuff) {
        ftStatus += FT_Purge(ftHandle, FT_PURGE_TX);
    }
    if (ftStatus != FT_OK) {
        std::cout << "Warning! FT_Purge is unsuccessful." << std::endl;
        return(false);
    }
    return(true);
}

void USBPort2::set(const std::string & _interfacename, const int _baudrate) {
    interfacename = _interfacename;
    baudrate = _baudrate;
}
