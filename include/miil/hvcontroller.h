#ifndef HVCONTROLLER_H
#define HVCONTROLLER_H

#include <miil/usbport1.h>
#include <string>
#include <sstream>


class HVController
{
public:
    HVController();
    bool openPort(const std::string &portname);
    bool isOpen();
    void closePort();
    int readVoltage1() { return readVoltage(true); }
    int readVoltage2() { return readVoltage(false); }
    int readRampSpeed1() { return readRampSpeed(true); }
    int readRampSpeed2() { return readRampSpeed(false); }
    bool setVoltage1(int u1) { return setVoltage(true, u1); }
    bool setVoltage2(int u2) { return setVoltage(false, u2); }
    bool setRampSpeed1(int v1) { return setRampSpeed(true, v1); }
    bool setRampSpeed2(int v2) { return setRampSpeed(false, v2); }
    std::string readStatus1() { return readStatus(true); }
    std::string readStatus2() { return readStatus(false); }
    double readCurrent1() { return readCurrent(true); }
    double readCurrent2() { return readCurrent(false); }
    int getVMax() { return vMax; }

    bool readModuleIdentifier(std::string &unitNumber,
                              std::string &softwareRel,
                              std::string &vMaxStr,
                              std::string &iMaxStr);

private:
    USBPort1 usbPort;
    int vMax;
    int iMax;
    int resendCnt;
    int retryCnt;
    std::string portName;


    bool send(const std::string &command);
    bool recv(std::string &response);

    int readVoltage(bool ch1);
    int readRampSpeed(bool ch1);
    bool setVoltage(bool ch1, int u);
    bool setRampSpeed(bool ch1, int v);
    std::string readStatus(bool ch1);
    double readCurrent(bool ch1);

};

#endif // HVCONTROLLER_H
