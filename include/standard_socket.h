#ifndef STANDARD_SOCKET_H
#define STANDARD_SOCKET_H

#include <vector>
#include <string.h>
#include <ethernet.h>

class StandardSocket : public Ethernet {
public:
    StandardSocket(
            const std::string & _interface = "eth1",
            const std::string & recv_a = "192.168.1.1",
            const std::string & send_a = "192.168.1.2",
            int recv_p = 21844,
            int send_p = 21845);
    StandardSocket(
            const std::string & send_a,
            int recv_p = 21844);
    ~StandardSocket();
    int send(const std::string & send_address,
             int port,
             const std::vector<char> & data);
    int send(const std::vector<char> & data);
    int recv(std::vector<char> & data);
    int Open(const std::string & if_name);
    int Open();
    int Close();

private:
    socklen_t socklen;
    struct sockaddr_in serv_addr;
    in_addr_t receive_address_compare;
};

#endif // STANDARD_SOCKET_H
