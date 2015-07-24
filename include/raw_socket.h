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

		int set_receive_timeout(double seconds);
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


	private:
        std::vector<uint8_t> GenerateUDPHeader(
                std::vector<uint8_t> src_port,
                std::vector<uint8_t> dst_port);
		void CalculateIPHeaderChecksum(std::vector<uint8_t> & hdr);
        std::vector<uint8_t> GenerateIPHeader(
                std::vector<uint8_t> src_ip,
                std::vector<uint8_t> dst_ip);
        std::vector<uint8_t> GenerateEthernetHeader(
                std::vector<uint8_t> dst_mac,
                std::vector<uint8_t> src_mac);
		void SetIPHeaderLength(
				std::vector<uint8_t> & ip_hdr,
				std::vector<uint8_t> & udp_hdr,
				std::vector<uint8_t> data);
        void SetUDPHeaderLength(
                std::vector<uint8_t> & udp_hdr,
                std::vector<uint8_t> data);
		void CalculateLengthsAndChecksums(
				std::vector<uint8_t> & ip_hdr,
				std::vector<uint8_t> & udp_hdr,
				std::vector<uint8_t> data);
		std::vector<uint8_t> IPStringToVector(std::string ip);
		std::vector<uint8_t> MACStringToVector(std::string mac);
		std::vector<uint8_t> PortNumberToVector(int port);
        std::vector<uint8_t> GeneratePacket(
                std::vector<uint8_t> data,
                std::string dst_mac,
                std::string src_mac,
                std::string src_ip,
                std::string dst_ip,
                int src_port,
                int dst_port);

		std::vector<uint8_t> GeneratePacket(std::vector<uint8_t> data);

		std::string dst_mac;
		std::string src_mac;

		uint16_t protocol;
		int fd;

		bool port_status;

        struct pollfd fds;
};


#endif /* SOCKET_H */
