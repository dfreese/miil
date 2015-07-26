#ifndef USBPORT_H
#define USBPORT_H

#include <string>
#include <fstream>
#include <vector>

class USBPort
{
public:
    USBPort();
    ~USBPort();
    virtual bool openPort(const int &portNumber = 0) = 0;
    virtual bool openPort(const std::string &portName, bool block = false, int timeout100MS = -1) = 0;
    virtual void closePort() = 0;
    virtual bool getDeviceList(std::vector<std::string> &list) = 0;
    virtual int getQueSize() = 0;
    virtual int send(const std::vector<char> &sendBuff) = 0;
    virtual int send(const char &c) = 0;
    virtual int recv(std::vector<char> &recvBuff) = 0;
    virtual int recv(char &c) = 0;
    virtual int recv(char *buffer, int numBytes) = 0;
    virtual bool purge(bool rxBuff = true, bool txBuff = true) = 0;
    bool setRecvLog(const std::string &recvLogFilename, bool hexMode = true);
    bool setTranLog(const std::string &tranLogFilename, bool hexMode = true);
    bool isOpen() { return portState; }
    void setSimulation(bool value = true) { simulation = value; }
    long getTotalReceived() { return totalReceived; }
    long getTotalSent() { return totalSent; }
    long getTotalRetansmit() {return totalRetransmit; }

protected:
	std::ofstream recvLog;
	std::ofstream tranLog;
    long totalSent;
    long totalReceived;
    long totalRetransmit;
    bool logReceived;
    bool logTransmitted;
    bool portState;
    bool simulation;
    bool recvLogHexMode;
    bool tranLogHexMode;


};

#endif // USBPORT_H
