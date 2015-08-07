#ifndef USBPORT_H
#define USBPORT_H

#include <string>
#include <vector>

class USBPort {
public:
    USBPort();
    virtual ~USBPort() {}
    virtual bool openPort(const int &portNumber = 0) {
        return(false);
    }
    virtual bool openPort(
            const std::string &portName,
            bool block = false,
            int timeout100MS = -1)
    {
        return(false);
    }

    virtual void closePort() = 0;

    virtual int send(const std::vector<char> &sendBuff) = 0;
    virtual int send(const char &c) = 0;
    virtual int send(const std::string & str) = 0;
    
    virtual int recv(std::vector<char> &recvBuff) = 0;
    virtual int recv(char &c) = 0;
    virtual int recv(char *buffer, int numBytes) = 0;

    bool isOpen() {
        return portState;
    }
    long getTotalReceived() {
        return totalReceived;
    }
    long getTotalSent() {
        return totalSent;
    }
    long getTotalRetansmit() {
        return totalRetransmit;
    }
    virtual bool getDeviceList(std::vector<std::string> &list) {
        return(false);
    }
    virtual int getQueSize() {
        return(-1);
    }
    virtual bool purge(bool rxBuff = true, bool txBuff = true) {
        return (false);
    }

protected:
    long totalSent;
    long totalReceived;
    long totalRetransmit;
    bool portState;
};

#endif // USBPORT_H
