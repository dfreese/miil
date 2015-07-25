#include <ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>

Ethernet::Ethernet(
        const std::string & _interface,
        const std::string & recv_a,
        const std::string & send_a,
        int recv_p,
        int send_p) :
    interface(_interface),
    recv_address(recv_a),
    send_address(send_a),
    recv_port(recv_p),
    send_port(send_p),
    is_open(false),
    timeout_ms(150)
{
}


/* \brief Return a list of all of the available interfaces on the machine
 *
 * The function polls the system to find the available interfaces that can be used
 * for communication
 *
 * \param A vector in which the strings of the interface names are placed
 *
 * \return A bool indicating whether the list retrieval was successful
 */
bool Ethernet::list(std::vector<std::string> & list) {
    char          buf[1024];
    struct ifconf ifc;
    struct ifreq *ifr;
    int           sck;
    int           nInterfaces;

    sck = socket(AF_INET, SOCK_DGRAM, 0);
    if(sck < 0) {
        // Error Opening Socket
        //return(std::vector<std::string>());
        return(false);
    }

    /* Query available interfaces. */
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if(ioctl(sck, SIOCGIFCONF, &ifc) < 0) {
        // Error Querying the Interfaces
        return(false);
    }

    ifr         = ifc.ifc_req;
    nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
    for(int i = 0; i < nInterfaces; i++) {
        struct ifreq *item = &ifr[i];
        list.push_back(std::string(item->ifr_name));
        interface_list[item->ifr_name] =
                (struct sockaddr_in *) &(item->ifr_addr);
	}
	return true;
}

void Ethernet::setReceiveTimeout(int milliseconds) {
    timeout_ms = milliseconds;
}
