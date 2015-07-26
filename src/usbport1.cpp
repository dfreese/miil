/*! \class     USBPort1
 *  \brief     Set up USB1 communication
 *  \details   
               
	
	
 *  \author    Umit Yoruk
 *  \version   1.0
 *  \date      2012
 *  \bug       N/A
 *  \warning   
 */
#include <usbport1.h>

using namespace std;

USBPort1::USBPort1()
{
    tty_fd = -1;
}

USBPort1::USBPort1(const string &portName) {
    tty_fd = -1;
    openPort(portName);
}

bool USBPort1::openPort(const int &portNumber)
{
    portState = false;
    return false; // This is not implemented in USB1 (see USB2)
}


bool USBPort1::openPort(const string &portName, bool block, int timeout100MS)
{
    int vMin = 0;   // If timeout100MS is given waits for 0.1*timeout100MS seconds after the last transmission for timeout.
    int vTime = timeout100MS;

    if (timeout100MS == -1) {  // Pure blocking read() - Does not return until at least 1 byte is received.
        vTime = 5;
        vMin = 1;
    }

    if (portName.empty()) {
        portState = false;
        return false;
    }

    this->portName = portName;
    if (block) { // By default this is set to false
        // Needed for High Voltage Controls
        tty_fd = open(portName.c_str(), O_RDWR | O_NOCTTY);
    }
    else {
        tty_fd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    }
    if (tty_fd ==-1){
        cout << "Failed to open port \"" << portName << "\"" << endl;
        portState = false;
        return false;
    }

    totalSent = 0;
    totalReceived = 0;
    totalRetransmit = 0;

    memset(&tio,0,sizeof(tio));
    tio.c_iflag=0;
    tio.c_oflag=0;
    tio.c_cflag=CS8|CREAD|CLOCAL;       // 8n1, see termios.h for more information
    tio.c_lflag=0;
    tio.c_cc[VMIN]=vMin;
    tio.c_cc[VTIME]=vTime;
    cfsetospeed(&tio,B921600);          // 921600 baud
    cfsetispeed(&tio,B921600);          // 921600 baud
    tcsetattr(tty_fd,TCSANOW,&tio);
    tio.c_cflag &= ~PARENB;
    tio.c_cflag &= ~CSTOPB;
    tio.c_cflag &= ~CSIZE;              // Mask the character size bits
    tio.c_cflag |= CS8;                 // Select 8 data bits

    portState = true;
    return true;
}

void USBPort1::closePort()
{
    close(tty_fd);
    portState = false;
}


bool USBPort1::getDeviceList(vector<string> &list)
{
    return false; // This is not implemented in USB1 (see USB2)
}

int USBPort1::getQueSize()
{
    return -1; // This is not implemented in USB1 (see USB2)
}

int USBPort1::send(const vector<char> &sendBuff)
{
    char c;
    for(unsigned int ii=0; ii<sendBuff.size(); ii++){
        c=sendBuff[ii];
        send(c);
    }
    return sendBuff.size();
}


int USBPort1::send(const char &c)
{
    int cntE = 0;
    if (!simulation) {
        while (write(tty_fd, &c, 1)==-1){
            cntE++;
            if (cntE > 10000) {
                cout << "Transmission Failed!" << endl;
                throw(-1);
            }
        }
    }
    if (logTransmitted) {
        if (recvLogHexMode) {
            tranLog << uppercase << hex << (0x000000FF&int(c)) << dec << endl;
        }
        else {
            tranLog << c; // Log as binary
        }
    }
    totalSent++;
    totalRetransmit += cntE;
    return 1;
}

int USBPort1::recv(vector<char> &recvBuff)
{
    char tempBuffer[100];
    int numBytes = 0;
    if (!simulation) {
        numBytes = read(tty_fd, tempBuffer, 100);
        for (int ii=0; ii<numBytes; ii++) {
            recvBuff.push_back(tempBuffer[ii]);
        }
    }
    totalReceived += numBytes;
    return numBytes;
}

int USBPort1::recv(char &c)
{
    if (simulation) {
        return 0;
    }
    int numBytes = read(tty_fd, &c, 1);
    totalReceived += numBytes;
    return numBytes;
}


void USBPort1::setBaudRate(speed_t baud)
{
    cfsetospeed(&tio,baud);
    cfsetispeed(&tio,baud);
    tcsetattr(tty_fd,TCSANOW,&tio);
}

int USBPort1::recv(char *buffer, int numBytes) {
	cout << "Reading of buffer: " << buffer << " with numBytes: " << numBytes << "\n";
}
