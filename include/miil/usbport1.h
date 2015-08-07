#ifndef USBPORT1_H
#define USBPORT1_H

#include <miil/usbport.h>
#include <fcntl.h>
#include <termios.h>
#include <iostream>
#include <string>
#include <vector>
#include <string.h>

class USBPort1 : public USBPort
{
public:
    USBPort1();
    USBPort1(const std::string &portName);
    bool openPort(
            const std::string &portName,
            bool block = false,
            int timeout100MS = -1);
    void closePort();
    int send(const std::vector<char> &sendBuff);
    int send(const char &c);
    int send(const std::string & str);
    int recv(std::vector<char> &recvBuff);
    int recv(char &c);
    int recv(char *buffer, int numBytes);
    void setBaudRate(speed_t baud = B9600);

private:
    std::string portName;
    int tty_fd;
    struct termios tio;

};

#endif // USBPORT1_H
