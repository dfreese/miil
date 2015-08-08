#include <miil/instekpowersupply.h>
#include <stdlib.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <miil/usbport1.h>
#include <miil/util.h>

using namespace std;

namespace {
template<typename T>
int readValue(USBPort1 * port, T & value, char end_char = '\n') {
    vector<char> buffer;
    if (port->recv(buffer) < 0) {
        return(-1);
    }

    vector<char>::iterator end_pos = find(buffer.begin(), buffer.end(), end_char);

    if (end_pos == buffer.end()) {
        return(-2);
    }
    T potential;
    if (Util::StringToNumber(string(buffer.begin(), end_pos), potential) < 0) {
        return(-3);
    }
    value = potential;
    return(0);
}

}

InstekPowerSupply::InstekPowerSupply(const string & portName, int baud, int noChannels) :
    port(new USBPort1),
    port_name(portName),
    baud_rate(baud),
    no_channels(noChannels)
{
}

int InstekPowerSupply::Open(const string & portName, int baud) {
    port_name = portName;
    baud_rate = baud;

    if (port->openPort(port_name, true, 5) < 0) {
        return(-1);
    }

    if (port->send("AT\n") != 3) {
        port->closePort();
        return(-2);
    }

    if (port->send("*cls\n") != 5) {
        port->closePort();
        return(-3);
    }

    vector<char> dummy;
    port->recv(dummy);

    return(0);
}

int InstekPowerSupply::Open() {
    return Open(port_name, baud_rate);
}

void InstekPowerSupply::Close() {
    port->closePort();
}

std::string InstekPowerSupply::getName() {
    string name_query = "*idn ?\n";
    if (port->send(name_query) < name_query.size()) {
        return("");
    }

    vector<char> buffer;
    if (port->recv(buffer) < 0) {
        return("");
    }

    string name;
    if (readValue(port, name) < 0) {
        return("");
    }
    return(name);
}

int InstekPowerSupply::getVoltage(int channel, float & voltage) {
    if (!channelGood(channel)) {
        return(-1);
    }

    std::stringstream ss;
    ss << ":chan" << channel << ":volt ?\n";

    if (port->send(ss.str()) < ss.str().size()) {
        return(-2);
    }

    if (readValue(port, voltage) < 0) {
        return(-3);
    }
    return(0);
}

int InstekPowerSupply::setVoltage(int channel, float voltage) {
    if (!channelGood(channel)) {
        return(-1);
    }

    std::stringstream ss;
    ss << ":chan" << channel << ":volt " << voltage << "\n";

    if (port->send(ss.str()) < ss.str().size()) {
        return(-2);
    }
    return(0);
}

int InstekPowerSupply::getCurrent(int channel, float & current) {
    if (!channelGood(channel)) {
        return(-1);
    }

    std::stringstream ss;
    ss << ":chan" << channel << ":curr ?\n";

    if (port->send(ss.str()) < ss.str().size()) {
        return(-2);
    }

    if (readValue(port, current) < 0) {
        return(-3);
    }
    return(0);
}

int InstekPowerSupply::setCurrent(int channel, float current) {
    if (!channelGood(channel)) {
        return(-1);
    }

    std::stringstream ss;
    ss << ":chan" << channel << ":curr " << current << "\n";

    if (port->send(ss.str()) < ss.str().size()) {
        return(-2);
    }
    return(0);
}

int InstekPowerSupply::getMeasuredCurrent(int channel, float & measured_current) {
    if (!channelGood(channel)) {
        return(-1);
    }

    std::stringstream ss;
    ss << ":chan" << channel << ":meas:curr ?\n";

    if (port->send(ss.str()) < ss.str().size()) {
        return(-2);
    }
    if (readValue(port, measured_current) < 0) {
        return(-3);
    }
    return(0);

}

int InstekPowerSupply::getMeasuredVoltage(int channel, float & measured_voltage) {
    if (!channelGood(channel)) {
        return(-1);
    }

    std::stringstream ss;
    ss << ":chan" << channel << ":meas:volt ?\n";

    if (port->send(ss.str()) < ss.str().size()) {
        return(-2);
    }
    if (readValue(port, measured_voltage) < 0) {
        return(-3);
    }
    return(0);

}

int InstekPowerSupply::getOverVoltage(int channel, float & over_voltage) {
    if (!channelGood(channel)) {
        return(-1);
    }

    std::stringstream ss;
    ss << ":chan" << channel << ":prot:volt ?\n";

    if (port->send(ss.str()) < ss.str().size()) {
        return(-2);
    }
    if (readValue(port, over_voltage) < 0) {
        return(-3);
    }
    return(0);
}

int InstekPowerSupply::setOverVoltage(int channel, float voltage) {
    if (!channelGood(channel)) {
        return(-1);
    }

    std::stringstream ss;
    ss << ":chan" << channel << ":prot:volt " << voltage << "\n";

    if (port->send(ss.str()) < ss.str().size()) {
        return(-2);
    }
    return(0);
}

int InstekPowerSupply::getOverCurrent(int channel, bool &over_current_protect) {
    if (!channelGood(channel)) {
        return(-1);
    }

    std::stringstream ss;
    ss << ":chan" << channel << ":prot:curr ?\n";

    if (port->send(ss.str()) < ss.str().size()) {
        return(-2);
    }
    if (readValue(port, over_current_protect) < 0) {
        return(-3);
    }
    return(0);
}

int InstekPowerSupply::setOverCurrent(int channel, bool current) {
    if (!channelGood(channel)) {
        return(-1);
    }

    std::stringstream ss;
    ss << ":chan" << channel << ":prot:curr " << current << "\n";

    if (port->send(ss.str()) < ss.str().size()) {
        return(-2);
    }
    return(0);
}

bool InstekPowerSupply::isOpen()
{
    return(port->isOpen());
}

bool InstekPowerSupply::channelGood(int channel)
{
    if (channel < 1 || channel > 3) {
        return(false);
    }
    return(true);
}

int InstekPowerSupply::getOutput(bool & state) {
    std::stringstream ss;
    ss << ":outp:stat ?\n";

    if (port->send(ss.str()) < ss.str().size()) {
        return(-1);
    }

    if (readValue(port, state) < 0) {
        return(-2);
    }
    return(0);
}

int InstekPowerSupply::setOutput(bool state) {
    std::stringstream ss;
    ss << ":outp:stat " << state << "\n";

    if (port->send(ss.str()) < ss.str().size()) {
        return(-1);
    }
    return(0);
}
