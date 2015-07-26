#ifndef USBPORT1_H
#define USBPORT1_H

#include <usbport.h>
#include <fcntl.h>
#include <termios.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <string.h>

class USBPort1 : public USBPort
{
public:
    USBPort1();
    USBPort1(const std::string &portName);
    bool openPort(const int &portNumber);
    bool openPort(const std::string &portName, bool block = false, int timeout100MS = -1);
    bool getDeviceList(std::vector<std::string> &list);
    int getQueSize();
    void closePort();
    int send(const std::vector<char> &sendBuff);
    int send(const char &c);
    int recv(std::vector<char> &recvBuff);
    int recv(char &c);
    int recv(char *buffer, int numBytes);
    bool purge(bool, bool) {return false;} // Not implemented in USB1 (see USB2)
    void setBaudRate(speed_t baud = B9600);
private:
    std::string portName;
    int tty_fd;
    struct termios tio;

};

#endif // USBPORT1_H
