
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iterator>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <math.h>
#include <cstdio>
#include <miil/raw_socket.h>

#define UDP_HEADER_LENGTH 42
#define MAX_UDP_PAYLOAD 1024

RawSocket::RawSocket(
        const std::string & _interface,
        const std::string & recv_a,
        const std::string & send_a,
        int recv_p,
        int send_p,
        const std::string & local_mac,
        const std::string & remote_mac) :
    Ethernet(_interface, recv_a, send_a, recv_p, send_p),
    dst_mac(remote_mac),
    src_mac(local_mac)
{
}

RawSocket::~RawSocket() {
    Close();
}

namespace {
uint16_t protocol(0x0800);
}

int RawSocket::Open(const std::string & if_name) {
    interface = std::string(if_name);

    // Open raw packet socket
    fd = socket(AF_PACKET, SOCK_RAW, htons(protocol));
    //if (fd < 0) throw std::string("Could not open socket");
    if (fd<0) std::cerr << "Could not open socket" << std::endl;
    if (fd < 0) return fd;

    // Get interface index from name
    if (if_name.size() >= IFNAMSIZ) {
        std::cerr << "Interface name too long" << std::endl;
        return (-1);
    }
    ifreq ifr;
    memcpy(ifr.ifr_name,if_name.c_str(),if_name.size()+1);
    int ioctl_status = ioctl(fd, SIOCGIFINDEX, &ifr);
    if (ioctl_status) {
        std::cerr << "Could not get interface index" << std::endl;
        return(ioctl_status);
    }

    // Bind socket to requested interface
    struct sockaddr_ll addr;
    memset(&addr, 0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(protocol);
    addr.sll_ifindex = ifr.ifr_ifindex;
    int bind_status = bind(fd, (struct sockaddr *)(&addr), sizeof(addr));
    if (bind_status) {
        std::cerr << "Could not bind to socket" << std::endl;
        return(bind_status);
    }

    is_open = true;

    memset(&fds, 0, sizeof(fds));
    fds.fd = fd;
    fds.events = POLLIN;

    return fd;
}

int RawSocket::Open() {
    return Open(interface);
}

int RawSocket::Close() {
    is_open = false;
    return close(fd);
}

char buf[ETH_FRAME_LEN];

int RawSocket::recv(std::vector<char> & data) {
    struct sockaddr_in remote_addr;
    socklen_t address_len = sizeof(remote_addr);

    ssize_t err_or_size(0);

    if (poll(&fds, 1, timeout_ms) > 0) {
        if (fds.revents & (POLLIN)) {
            err_or_size = ::recvfrom(fd, buf, sizeof(buf), MSG_TRUNC,
                                     (struct sockaddr*)&remote_addr,
                                     &address_len);
        }
    } else {
        return(ETH_NO_ERR);
    }

    if ((err_or_size < 0) && (errno != EAGAIN)) {
        errno = 0;   // clear the error
        return(ETH_ERR_RX);
    } else if((err_or_size == 0) || (errno == EAGAIN)) {  // no data
        errno = 0;   // clear the error
        return(ETH_NO_ERR);
    } else if (err_or_size < UDP_HEADER_LENGTH) {
        return(ETH_NO_ERR);
    } else if (buf[23] != (char) 0x11) {
        // Protocol was not UDP
        return(ETH_NO_ERR);
    } else {
        int port = (int) ((unsigned char) buf[UDP_HEADER_LENGTH - 6] << 8) |
            (int) (unsigned char) buf[UDP_HEADER_LENGTH - 5];
        if (port == recv_port) {
            data.insert(
                    data.end(),
                    buf + UDP_HEADER_LENGTH,
                    buf + err_or_size);
            return(err_or_size - UDP_HEADER_LENGTH);
        } else {
            return(ETH_NO_ERR);
        }
    }
}

// Start of anonymous namespace to limit functions to local file
namespace {
std::vector<uint8_t> GenerateUDPHeader(
        std::vector<uint8_t> src_port,
        std::vector<uint8_t> dst_port)
{
    std::vector<uint8_t> hdr;
    hdr.reserve(8);

    for (std::vector<uint8_t>::iterator it=src_port.begin(); it!=src_port.end(); it++) hdr.push_back(*it);
    for (std::vector<uint8_t>::iterator it=dst_port.begin(); it!=dst_port.end(); it++) hdr.push_back(*it);
    hdr.push_back(0x00); // Length to be modified later
    hdr.push_back(0x08); // ^ (8 is min length)
    hdr.push_back(0x00); // Checksum (optional)
    hdr.push_back(0x00); // ^

    return hdr;
}

void CalculateIPHeaderChecksum(std::vector<uint8_t> & hdr) {
    int16_t checksum = 0;
    for ( int i=0; i<10; i++ ) { // Assuming 20 byte header
        int16_t word = ((uint16_t) hdr[i*2+0] << 8) | ((uint16_t) hdr[i*2+1] );
        int16_t overflow = ((checksum >> 15) & 1) & ( (word >> 15) & 1);
        checksum = checksum + word + overflow;
    }

    checksum = ~checksum;
    //std::cout << "Checksum: " << hex((uint8_t)(0x00FF & (checksum >> 8) )) << hex((uint8_t)(0x00FF & checksum )) << std::endl;

    hdr[10] = (uint8_t)(0x00FF & (checksum >> 8) );
    hdr[11] = (uint8_t)(0x00FF & checksum );
}

std::vector<uint8_t> GenerateIPHeader(
        std::vector<uint8_t> src_ip,
        std::vector<uint8_t> dst_ip)
{
    std::vector<uint8_t> hdr;
    hdr.reserve(20);

    hdr.push_back(0x45);
    hdr.push_back(0x00);
    hdr.push_back(0x00); // Length
    hdr.push_back(0x20); // ^ (min length is 20)
    hdr.push_back(0x00);
    hdr.push_back(0x01);
    hdr.push_back(0x40);
    hdr.push_back(0x00);
    hdr.push_back(0x62);
    hdr.push_back(0x11);
    hdr.push_back(0x00);
    hdr.push_back(0x00);

    for (std::vector<uint8_t>::iterator it=src_ip.begin(); it!=src_ip.end(); it++) hdr.push_back(*it);
    for (std::vector<uint8_t>::iterator it=dst_ip.begin(); it!=dst_ip.end(); it++) hdr.push_back(*it);

    return hdr;

}

std::vector<uint8_t> GenerateEthernetHeader(
        std::vector<uint8_t> dst_mac,
        std::vector<uint8_t> src_mac)
{
    std::vector<uint8_t> hdr;
    hdr.reserve(14);

    for (std::vector<uint8_t>::iterator it=src_mac.begin(); it!=src_mac.end(); it++) hdr.push_back(*it);
    for (std::vector<uint8_t>::iterator it=dst_mac.begin(); it!=dst_mac.end(); it++) hdr.push_back(*it);
    hdr.push_back(0x08); // Specify protocol type: IP
    hdr.push_back(0x00); // ^

    return hdr;
}

void SetIPHeaderLength(
        std::vector<uint8_t> & ip_hdr,
        std::vector<uint8_t> & udp_hdr,
        std::vector<uint8_t> data)
{
    int16_t size = (uint16_t) (ip_hdr.size()+udp_hdr.size()+data.size());
    ip_hdr[2] = (uint8_t)(0x00FF & (size >> 8) );
    ip_hdr[3] = (uint8_t)(0x00FF & (size ) );
}

void SetUDPHeaderLength(std::vector<uint8_t> & udp_hdr, std::vector<uint8_t> data) {
    int16_t size = (uint16_t) (udp_hdr.size()+data.size());
    udp_hdr[4] = (uint8_t)(0x00FF & (size >> 8) );
    udp_hdr[5] = (uint8_t)(0x00FF & (size ) );
}

void CalculateLengthsAndChecksums(
        std::vector<uint8_t> & ip_hdr,
        std::vector<uint8_t> & udp_hdr,
        std::vector<uint8_t> data)
{
    SetIPHeaderLength(ip_hdr,udp_hdr,data);
    SetUDPHeaderLength(udp_hdr,data);
    CalculateIPHeaderChecksum(ip_hdr);
}

std::vector<uint8_t> IPStringToVector(std::string ip) {
    std::vector<uint8_t> ip_vec;
    ip_vec.reserve(6);
    std::stringstream ss(ip);

    int test;
    while(ss >> test) {
        ip_vec.push_back((uint8_t) test);
        ss.get();
    }
    return ip_vec;
}

std::vector<uint8_t> MACStringToVector(std::string mac) {
    std::vector<uint8_t> mac_vec;
    mac_vec.reserve(6);
    std::stringstream ss(mac);
    int test;
    ss >> std::hex;
    while(ss >> test) {
        mac_vec.push_back((uint8_t) test);
        ss.get();
    }

    return mac_vec;
}

std::vector<uint8_t> PortNumberToVector(int port) {
    std::vector<uint8_t> pv(2);
    pv[0] = (0xFF & (((uint16_t) port) >> 8));
    pv[1] = (0xFF & (((uint16_t) port) ));
    return pv;
}

std::vector<uint8_t> generate_packet(
    std::vector<uint8_t> data,
    std::string dst_mac,
    std::string src_mac,
    std::string src_ip,
    std::string dst_ip,
    int src_port,
    int dst_port)
{
    std::vector<uint8_t> eth_hdr = GenerateEthernetHeader(
            MACStringToVector(dst_mac),MACStringToVector(src_mac));
    std::vector<uint8_t> ip_hdr = GenerateIPHeader(
            IPStringToVector(src_ip),IPStringToVector(dst_ip));
    std::vector<uint8_t> udp_hdr = GenerateUDPHeader(
            PortNumberToVector(src_port),PortNumberToVector(dst_port));

    CalculateLengthsAndChecksums(ip_hdr, udp_hdr, data);

    std::vector<uint8_t> packet;
    packet.reserve(eth_hdr.size()+ip_hdr.size()+udp_hdr.size()+data.size());

    for (std::vector<uint8_t>::iterator it=eth_hdr.begin(); it!=eth_hdr.end(); it++) packet.push_back(*it);
    for (std::vector<uint8_t>::iterator it=ip_hdr.begin(); it!=ip_hdr.end(); it++) packet.push_back(*it);
    for (std::vector<uint8_t>::iterator it=udp_hdr.begin(); it!=udp_hdr.end(); it++) packet.push_back(*it);
    for (std::vector<uint8_t>::iterator it=data.begin(); it!=data.end(); it++) packet.push_back(*it);

    return packet;
}

void UpdateIPHeaderLength(
        char * packet,
        uint16_t size)
{
    // Take off Ethernet header length
    size -= 14;
    packet[16] = (char) (uint8_t)(0x00FF & (size >> 8) );
    packet[17] = (char) (uint8_t)(0x00FF & (size ) );
}

void UpdateUDPHeaderLength(
        char * udp_hdr,
        uint16_t size)
{
    // Take off Ethernet and IP header length
    size -= 20 + 14;
    udp_hdr[38] = (uint8_t)(0x00FF & (size >> 8) );
    udp_hdr[39] = (uint8_t)(0x00FF & (size ) );
}

void UpdateIPHeaderChecksum(char * hdr) {
    // Zero out the checksum first
    hdr[24] = 0;
    hdr[25] = 0;
    // Calculate the checksum, really should just skip checksum section
    int16_t checksum = 0;
    for (int i = 0; i < 10; i++) {
        int16_t word = ((uint16_t) (uint8_t) hdr[14 + i*2 + 0] << 8) |
            ((uint16_t) (uint8_t) hdr[14 + i*2 + 1] );
        int16_t overflow = ((checksum >> 15) & 1) & ( (word >> 15) & 1);
        checksum = checksum + word + overflow;
    }
    checksum = ~checksum;

    // Put it back into the vector
    hdr[24] = (uint8_t)(0x00FF & (checksum >> 8) );
    hdr[25] = (uint8_t)(0x00FF & checksum );
}

/*!
 * \brief Thread specific buffer for udp packet header
 *
 * A buffer for the header for a packet to be stored so that the header isn't
 * generated every time the packet needs to be sent.  __thread keyword is a gcc
 * extension to make the variable specific to every thread instance.
 */
__thread char packet_buffer[UDP_HEADER_LENGTH + MAX_UDP_PAYLOAD];

/*!
 * \brief Thread specific flag indicating udp packet header was generated
 *
 * A flag to indicate if the header for a packet has been generated and stored
 * in packet_buffer.  __thread keyword is a gcc extension to make the variable
 * specific to every thread instance.
 */
__thread bool generic_vector_generated(false);
// End of anonymous namespace to limit functions to local file
}

int RawSocket::send(const std::vector<char> & packet) {
    return(send(packet.begin(), packet.end()));
}

int RawSocket::send(
        std::vector<char>::const_iterator start,
        std::vector<char>::const_iterator stop)
{
    if (!generic_vector_generated) {
        std::vector<uint8_t> generic_vector_uint8_t =
                GeneratePacket(std::vector<uint8_t>(MAX_UDP_PAYLOAD, 0));
        std::copy(generic_vector_uint8_t.begin(),
                  generic_vector_uint8_t.end(),
                  packet_buffer);
        generic_vector_generated = true;
    }
    int bytes_sent(0);
    size_t packet_size(std::distance(start,stop));
    for (size_t ii = 0; ii < packet_size; ii++) {
        size_t ii_local = ii % MAX_UDP_PAYLOAD;
        packet_buffer[ii_local + UDP_HEADER_LENGTH] = *(start + ii);
        if ((ii == (packet_size - 1)) || (ii_local == (MAX_UDP_PAYLOAD - 1))) {
            size_t local_packet_size = UDP_HEADER_LENGTH + ii_local + 1;
            UpdateIPHeaderLength(packet_buffer, local_packet_size);
            UpdateUDPHeaderLength(packet_buffer, local_packet_size);
            UpdateIPHeaderChecksum(packet_buffer);
            int err_or_size = ::send(fd, packet_buffer, local_packet_size, 0);
            if (err_or_size < 0) {
                return(err_or_size);
            } else {
                bytes_sent += err_or_size - UDP_HEADER_LENGTH;
            }
        }
    }
    return(bytes_sent);
}

std::vector<uint8_t> RawSocket::GeneratePacket(std::vector<uint8_t> data) {
    return(generate_packet(data,
                           dst_mac,
                           src_mac,
                           recv_address,
                           send_address,
                           recv_port,
                           send_port));
}

void RawSocket::setDstMac(const std::string & dst_mac) {
    this->dst_mac = dst_mac;
}

void RawSocket::setSrcMac(const std::string & src_mac) {
    this->src_mac = src_mac;
}
