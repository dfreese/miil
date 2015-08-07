#include <miil/temprhmonitor.h>
#include <algorithm>
#include <sstream>
#include <miil/usbport1.h>
#include <stdio.h>

using namespace std;

TempRhMonitor::TempRhMonitor(int portid) :
    port(new USBPort1),
    portnumber(portid)
{
}

bool TempRhMonitor::Open(int portid) {
    portnumber = portid;
    stringstream portin;
    portin << "/dev/ttyUSB" << portid;
    portname = portin.str();
    bool stat = port->openPort(portname, false, 5);
    port->setBaudRate(B9600);
    return(stat);
}

bool TempRhMonitor::Open() {
    return Open(portnumber);
}

void TempRhMonitor::Close() {
    port->closePort();
}

int TempRhMonitor::getTempAndRH(float & temp, float & rh) {
    if (port->send("PA\r\n\0") != 4) {
        return(-1);
    }

    sleep(1);

    vector<char> buffer;
    if (port->recv(buffer) < 0) {
        return(-2);
    }

    char end_msg_char = '>';
    vector<char>::iterator end_msg_itr =
            find(buffer.begin(), buffer.end(), end_msg_char);

    if (end_msg_itr == buffer.end()) {
        return(-3);
    }

    *(end_msg_itr - 2) = 0;

    int scanstat = sscanf(&buffer[0],"%f,%f",&rh,&temp);
    if (scanstat != 2) {
        return(-4);
    }

    temp -= 32;
    temp *= (5.0/9.0);
    temperature = temp;
    relhumidity = rh;
    return(0);
}
