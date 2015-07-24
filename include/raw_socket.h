#ifndef SOCKET_H
#define SOCKET_H

#include <ethernet.h>
#include <stdint.h>
#include <poll.h>

class RawSocket : public Ethernet {
public:
    RawSocket(const std::string & _interface = "eth1",
              const std::string & recv_a = "192.168.1.1",
              const std::string & send_a = "192.168.1.2",
              int recv_p = 21844,
              int send_p = 21845,
              const std::string & local_mac = "68:05:CA:19:50:C3",
              const std::string & remote_mac = "68:05:CA:19:50:C2");
    ~RawSocket();

    int recv(std::vector<char> & data);
    int send(const std::vector<char> & data);
    int send(std::vector<char>::const_iterator start,
             std::vector<char>::const_iterator stop);
    void set_ethernet(std::string dst_mac, std::string src_mac);
    void set_ip(std::string src_ip, std::string dst_ip);
    void set_udp(int src_port, int dst_port);

    int Open(const std::string & if_name);
    int Open();
    int Close();
    bool list(std::vector<std::string> & list);
    void set_receive_timeout(int milliseconds);

private:
    std::vector<uint8_t> GeneratePacket(std::vector<uint8_t> data);

    std::string dst_mac;
    std::string src_mac;

    int fd;
    struct pollfd fds;
    int timeout_ms;
};


#endif /* SOCKET_H */
