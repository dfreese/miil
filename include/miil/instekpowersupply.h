#ifndef POWERSUPPLY_H
#define POWERSUPPLY_H

#include <string>
#include <termios.h>

class USBPort1;

// Class to control the GW Instek 3202 Power Supply

class InstekPowerSupply {

public:
    InstekPowerSupply(const std::string & portName,
                int baud = B9600,
                int noChannels = 3);
    std::string getName();
    int Open(const std::string & portName, int baud = B9600);
    int Open();
    void Close();
    int getOutput(bool & state);
    int setOutput(bool state);
    int getVoltage(int channel, float & voltage);
    int setVoltage(int channel, float voltage);
    int getCurrent(int channel, float & current);
    int setCurrent(int channel,float current);
    int getMeasuredCurrent(int channel, float & measured_current);
    int getMeasuredVoltage(int channel, float & measured_voltage);
    int getOverVoltage(int channel, float & over_voltage);
    int setOverVoltage(int channel,float voltage);
    int getOverCurrent(int channel, bool & over_current_protect);
    int setOverCurrent(int channel, bool current);
    bool isOpen();

private:
    bool channelGood(int channel);

    USBPort1 * port;
    std::string port_name;
    int baud_rate;
    std::string port_base;
    int no_channels;

};

#endif /* POWERSUPPLY_H */
