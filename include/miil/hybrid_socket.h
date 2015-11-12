#ifndef HYBRID_SOCKET_H
#define HYBRID_SOCKET_H

#include <vector>
#include <string.h>
#include <miil/ethernet.h>

class HybridSocket : public Ethernet {
public:
    HybridSocket(
            const std::string & if_name = "eth1",
            const std::string & recv_a = "192.168.1.1",
            const std::string & send_a = "192.168.1.2",
            int recv_p = 21844,
            int send_p = 21845);
    HybridSocket(
            const std::string & send_a,
            int recv_p = 21844);
    ~HybridSocket();
    int send(const std::string & send_address,
             int port,
             const std::vector<char> & data);
    int send(const std::vector<char> & data);
    int recv(std::vector<char> & data);
    int Open(const std::string & if_name);
    int Open();
    int Close();
private:
    /// use void to not expose the pcap_t type outside of the source file
    void * handle;
};

#endif // HYBRID_SOCKET_H
