#include <miil/hybrid_socket.h>
#include <errno.h>
#include <sstream>
#include <pcap.h>

#define UDP_HEADER_LENGTH 42

HybridSocket::HybridSocket(
        const std::string & if_name,
        const std::string & recv_a,
        const std::string & send_a,
        int recv_p,
        int send_p) :
    Ethernet(if_name, recv_a, send_a, recv_p, send_p)
{
}

HybridSocket::HybridSocket(
    const std::string & send_a,
        int recv_p) :
    Ethernet("", "", send_a, recv_p, 21845)
{
}

HybridSocket::~HybridSocket()
{
    int status = Close();
    if (status < 0) {
        throw(status);
    }
}

int HybridSocket::Open(const std::string & if_name) {
    if (is_open) {
        int status = Close();
        if (status != ETH_NO_ERR) {
            return(status);
        }
    }
    interface = if_name;
    char errbuf[PCAP_ERRBUF_SIZE];    /* Error string */
    struct bpf_program fp;        /* The compiled filter */
    bpf_u_int32 mask;        /* Our netmask */
    bpf_u_int32 net;        /* Our IP */

    std::stringstream filter_stream;
    filter_stream << "port " << recv_port;

    /* Define the device */
    if (pcap_lookupnet(if_name.c_str(), &net, &mask, errbuf) == -1) {
        net = 0;
        mask = 0;
    }

    handle = pcap_open_live(if_name.c_str(), BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        return(-1);
    }
    /* Compile and apply the filter */
    if (pcap_compile((pcap_t*) handle,
                     &fp,
                     filter_stream.str().c_str(),
                     0, net) == -1)
    {
        return(2);
    }
    if (pcap_setfilter((pcap_t*) handle, &fp) == -1) {
        return(-3);
    }
    return(ETH_NO_ERR);
}

int HybridSocket::Open() {
    return(Open(interface));
}

int HybridSocket::Close() {
    if (is_open) {
        ssize_t close_rc;
        close_rc = close(fd);
        if(close_rc == -1) {
            return(ETH_ERR_CLOSE);
        }
        is_open = false;
    }
    return(ETH_NO_ERR);
}

int HybridSocket::send(
        const std::string & send_address,
        int port,
        const std::vector<char> & data)
{
    int send_fd;
    struct sockaddr_in address;

    memset(&address,0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(send_address.c_str());
    address.sin_port = htons(port);
    send_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int val(1);
    setsockopt(send_fd, SOL_SOCKET, SO_BROADCAST, &val, sizeof(val));

    int err_or_size = sendto(send_fd,
                             (char *)(&data[0]),
                             data.size(),
                             0,
                             (struct sockaddr*)&address, sizeof(address));
    close(send_fd);
    return(err_or_size);
}

int HybridSocket::send(const std::vector<char> & data) {
    return(send(send_address, send_port, data));
}

int HybridSocket::recv(std::vector<char> & data) {
    struct pcap_pkthdr header;    /* The header that pcap gives us */
    const u_char *packet;        /* The actual packet */
    packet = pcap_next((pcap_t*) handle, &header);
    if (packet != NULL) {
        int bytes = 0;
        for (size_t ii = UDP_HEADER_LENGTH; ii < header.caplen; ii++) {
            data.push_back(*(packet + ii));
            bytes++;
        }
        return(bytes);
    } else {
        return(-1);
    }
}
