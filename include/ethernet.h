#ifndef ETHERNET_H
#define ETHERNET_H

#include <iostream>
#include <vector>
#include <deque>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string>
#include <map>
#include <mutex>

#define DATALENGTH 1024

enum eth_error {
    ETH_NO_ERR = 0,
    ETH_ERR_SOCK = -1,
    ETH_ERR_BIND = -2,
    ETH_ERR_BLOCK = -3,
    ETH_ERR_RX = -4,
    ETH_ERR_CLOSE = -5,
    ETH_ERR_INTERFACE = -6
};

class Ethernet
{
public:
    Ethernet(const std::string & _interface = "eth1",
             const std::string & recv_a = "192.168.1.1",
             const std::string & send_a = "192.168.1.2",
             int recv_p = 21844,
             int send_p = 21845);
    virtual ~Ethernet() {}
    virtual int send(const std::vector<char> & data) = 0;
    virtual int recv(std::vector<char> & data) = 0;
    virtual int Open(const std::string & if_name) = 0;
    virtual int Open() = 0;
    virtual int Close() = 0;
    bool isOpen(){return(is_open);}
    virtual bool list(std::vector<std::string> & list) = 0;
    void setRecvAddress(const std::string & address) {recv_address = address;}
    void setSendAddress(const std::string & address) {send_address = address;}
    void setRecvPort(int port) {recv_port = port;}
    void setSendPort(int port) {send_port = port;}

protected:
    std::string interface;
    std::string recv_address;
    std::string send_address;
    int recv_port;
    int send_port;

    bool is_open;
};

#endif // ETHERNET_H
