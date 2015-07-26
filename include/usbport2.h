#ifndef USBPORT2_H
#define USBPORT2_H

#include <string>
#include <vector>
#include "usbport.h"
#include "ftd2xx.h"

class USBPort2 : public USBPort {
public:
    USBPort2();
    USBPort2(const int &portNumber);
    ~USBPort2();
    bool openPort(const int &portNumber);
    bool openPort(const int &portNumber, const int baud);
    bool Open();
    bool Open(const std::string &name, int baud = FT_BAUD_9600);
    void Close(){closePort();}
    void closePort();
    bool getDeviceList(std::vector<std::string> &list);
    int getQueSize();
    int send(std::string str);
    int send(const std::vector<char> &sendBuff);
    int send(const char &c);
    int recv(std::vector<char> & response, long waitns);
    int recv(std::vector<char> &recvBuff);
    int recv(char &c);
    int recv(char *buffer, int numBytes);
    int recv(std::vector<char> &rxv,char endchar,float timeout_s);
    bool purge(bool rxBuff, bool txBuff);
    std::string getInterfaceName(){return interfacename;}
    int getBaudRate(){return baudrate;}
    void set(const std::string & _interfacename, const int _baudrate);

private:
    FT_HANDLE ftHandle;
    std::string interfacename;
    int baudrate;
};

#endif // USBPORT2_H
