/*! \class     HVController
 *  \brief     Interfaces with the ISEG HV module over RS232
 *  \details   Class uses USBPort for opening the interface (USB1)\n
 *             Reads currents and voltages
 */
#include <miil/hvcontroller.h>
#include <math.h>

#define RESEND_MAX 3
#define RETRY_MAX 3
#define INVALID -123456789

using namespace std;

HVController::HVController()
{
    resendCnt = 0;
    retryCnt = 0;

    vMax = 0;
    iMax = 0;
}


bool HVController::openPort(const string &portname)
{
    this->portName = portname;
    // blocking mode = true, timeout = 500 msec
    if (!usbPort.openPort(portname, true, 5)) {
        cout << "HVController cannot open port \"" << portname << "\"" << endl;
        return false;
    }
    usbPort.setBaudRate(B9600);

    string unitNumber;
    string softwareRel;
    string vMaxStr;
    string iMaxStr;

    if (!readModuleIdentifier(unitNumber, softwareRel, vMaxStr, iMaxStr)) {
        return false;
    }

    cout << "Status1 = " << readStatus1() << endl;
    cout << "Status2 = " << readStatus2() << endl;

    // Try to get Vmax and Imax to verify
    // If this fails return false!
    return true;
}

bool HVController::isOpen()
{
    return usbPort.isOpen();
}

void HVController::closePort()
{
    usbPort.closePort();
}

bool HVController::send(const string &command)
{
    resendCnt = 0;

    // For each character in the command, send it, and check for and echo back
    if (!usbPort.isOpen()) {
        return(false);
    }
    bool sent_successfully(false);
    while ((resendCnt < RESEND_MAX) && !sent_successfully) {
        resendCnt++;
        bool status(true); // Assume success until failure condition
        for (size_t ii=0; ii<command.size(); ii++) {
            char s = command[ii];
            char r;
            int send_status(usbPort.send(s));
            if (send_status != 1) {
                status = false;
                break; // From for loop
            } else {
                int recv_status(usbPort.recv(r));
                if (recv_status <= 0) {
                    status = false;
                    break; // From for loop
                } else {
                    if (s != r) {
                        // Echo from hv supply a mismatch from what was sent
                        status = false;
                        break; // From for loop
                    }
                }
            }
        }
        if (status) {
            // If the full command is sent without error, finish off
            // message and exit loop
            usbPort.send('\r');
            usbPort.send('\n');
            sent_successfully = true;
        }
    }
    if (sent_successfully) {
        return(true);
    } else {
        return(false);
    }
}


bool HVController::recv(string &response)
{
    char c;
    int lfCnt = 0;

    response.clear();
    //cout << ">>";
    while (true) {
        if (usbPort.recv(c) <= 0) {
            cout << "ERROR! HVController response timeout." << endl;
            return false;
        }
        if (c == '\n' || c == '\r') {
            if ( c == '\n') {
                lfCnt++;
            }
        } else {
            response += c;
            //cout << c;
        }
        if (lfCnt > 1) {
            break;
        }
    }
    return true;
}


bool HVController::readModuleIdentifier(
        string &unitNumber,
        string &softwareRel,
        string &vMaxStr,
        string &iMaxStr)
{
    if (!send("#")) {
        return false;
    }
    string response;
    if (!recv(response)) {
        return false;
    }

    // Parse response here!
    stringstream ss(response);
    vector<string> field;
    while (ss) {
        string s;
        if (!getline(ss, s, ';')) {
            break;
        }
        field.push_back(s);
    }

    if (field.size() != 4) {
        cout << "HVController::readModuleIdentifier()"
             << " received invalid response!"
             << endl;
        return false;
    }
    unitNumber = field[0];
    softwareRel = field[1];
    vMaxStr = field[2];
    iMaxStr = field[3];

    ss.str(vMaxStr);
    ss.clear();
    ss >> vMax;
    ss.str(iMaxStr);
    ss.clear();
    ss >> iMax;

    return true;
}

string HVController::readStatus(bool ch1) {
    string command;
    if (ch1) {
        command = "S1";
    }
    else {
        command = "S2";
    }

    string response;
    if (!send(command)) {
        return "";
    }
    recv(response);
    stringstream ss(response);
    vector<string> field;
    while (ss) {
        string s;
        if (!getline(ss, s, '=')) {
            break;
        }
        field.push_back(s);
    }

    // Check to see if the response matches the command
    // If not, then try again.
    if (field.size() != 2 || command.compare(field[0]) != 0) {
        retryCnt++;
        cout << "Trying to reconnect!" << endl;
        openPort(portName);
        if (retryCnt == RETRY_MAX) {
            retryCnt = 0;
            cout << "Command \"" << command << "\" failed!" << endl;
            return "";
        }
        return readStatus(ch1); // try again
    }

    // Command passed the check
    retryCnt = 0;
    return field[1];
}


int HVController::readVoltage(bool ch1) {
    string command;
    if (ch1) {
        command = "U1";
    }
    else {
        command = "U2";
    }


    string response;
    if (!send(command)) {
        return INVALID;
    }

    recv(response);
    stringstream ss(response);
    int u;
    if (!(ss >> u)) {
        retryCnt++;
        cout << "Trying to reconnect!" << endl;
        openPort(portName);
        if (retryCnt == RETRY_MAX) {
            retryCnt = 0;
            cout << "Command \"" << command << "\" failed!" << endl;
            return INVALID;
        }
        return readVoltage(ch1); // try again
    }

    retryCnt = 0;
    return u;
}

double HVController::readCurrent(bool ch1) {
    string command;
    if (ch1) {
        command = "I1";
    }
    else {
        command = "I2";
    }

    if (!send(command)) {
        return INVALID;
    }

    string response;
    recv(response);

    stringstream ss(response);
    int numResp;
    int power;
    char minusSign;

    if (!(ss >> numResp) || !(ss >> minusSign) || !(ss >> power)) {
        retryCnt++;
        cout << "Trying to reconnect!" << endl;
        openPort(portName);
        if (retryCnt == RETRY_MAX) {
            retryCnt = 0;
            cout << "Command \"" << command << "\" failed!" << endl;
            return INVALID;
        }
        return readCurrent(ch1); // try again
    }

    retryCnt = 0;

    return (double)numResp*pow(10,-power+6); // in uA
}


bool HVController::setVoltage(bool ch1, int u) {
    stringstream ss;
    if (ch1) {
        ss << "D1=" << u;
    }
    else {
        ss << "D2=" << u;
    }
    string command = ss.str();

    if (!send(command)) {
        return false;
    }

    string response;
    recv(response); // Should be in this format ""

    // Now start ramping up
    if (ch1) {
        command = "G1";
    }
    else {
        command = "G2";
    }

    if (!send(command)) {
        return false;
    }

    recv(response); // Should be in this format "S1=L2H"

    ss.str(response);
    ss.clear();

    vector<string> field;
    while (ss) {
        string s;
        if (!getline(ss, s, '=')) {
            break;
        }
        field.push_back(s);
    }

    // Check to see if the status is "L2H", "H2L" or "ON "
    if (field.size() != 2) {
        retryCnt++;
        if (retryCnt >= RETRY_MAX) {
            retryCnt = 0;
            cout << "Command \"" << command << "\" failed!" << endl;
            return false;
        } else {
            return setVoltage(ch1, u); // try again
        }
    } else {
        if (field[1].compare("OFF") == 0) {
            retryCnt = 0;
            return(false);
        } else {
            if (!((field[1].compare("L2H") == 0) ||
                  (field[1].compare("H2L") == 0) ||
                  (field[1].compare("ON ") == 0) ))
            {
                cout << "field.size() = " << field.size() << endl;
                cout << "status = " << field[1] << endl;
                retryCnt++;
                cout << "Trying to reconnect!" << endl;
                //sleep(3);
                //openPort(portName);
                if (retryCnt >= RETRY_MAX) {
                    retryCnt = 0;
                    cout << "Command \"" << command << "\" failed!" << endl;
                    return false;
                } else {
                    return setVoltage(ch1, u); // try again
                }
            }
        }
    }

    retryCnt = 0;
    return true;
}


bool HVController::setRampSpeed(bool ch1, int v) {
    stringstream ss;
    if (ch1) {
        ss << "V1=" << v;
    }
    else {
        ss << "V2=" << v;
    }
    string command = ss.str();

    if (!send(command)) {
        return false;
    }

    string response;
    recv(response); // Should be in this format ""

    // Verify your setting
    if (ch1) {
        command = "V1";
    }
    else {
        command = "V2";
    }

    if (!send(command)) {
        return false;
    }

    recv(response);
    ss.str(response);
    ss.clear();

    int numResp;
    if (!(ss >> numResp) || numResp != v) {
        retryCnt++;
        cout << "Trying to reconnect!" << endl;
        openPort(portName);
        if (retryCnt == RETRY_MAX) {
            retryCnt = 0;
            cout << "Command \"" << command << "\" failed!" << endl;
            return false;
        }
        return setRampSpeed(ch1, v); // try again
    }

    // Ramp speed set up successfully
    retryCnt = 0;
    return true;
}


int HVController::readRampSpeed(bool ch1) {
    string command;
    if (ch1) {
        command = "V1";
    }
    else {
        command = "V2";
    }

    if (!send(command)) {
        return INVALID;
    }

    string response;
    recv(response);
    stringstream ss(response);
    int numResp;
    if (!(ss >> numResp)) {
        retryCnt++;
        cout << "Trying to reconnect!" << endl;
        openPort(portName);
        if (retryCnt == RETRY_MAX) {
            retryCnt = 0;
            cout << "Command \"" << command << "\" failed!" << endl;
            return INVALID;
        }
        return readRampSpeed(ch1); // try again
    }

    retryCnt = 0;
    return numResp;
}
