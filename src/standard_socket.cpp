#include "standard_socket.h"
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>

StandardSocket::StandardSocket(
        const std::string & _interface,
        const std::string & recv_a,
        const std::string & send_a,
        int recv_p,
        int send_p) :
    Ethernet(_interface, recv_a, send_a, recv_p, send_p)
{
}
StandardSocket::StandardSocket(
	const std::string & send_a,
        int recv_p) :
    Ethernet("", "", send_a, recv_p, 21845),
    receive_address_compare(inet_addr(send_a.c_str()))
{
}

StandardSocket::~StandardSocket()
{
    int status = Close();
    if (status < 0) {
        throw(status);
    }
}

int StandardSocket::Open(const std::string & if_name) {
    /*
    if (is_open) {
        if (if_name == interface) {
            return(ETH_NO_ERR);
        } else {
            Close();
        }
    }

    std::vector<std::string> unused_vector;
    list(unused_vector);
    std::map <std::string, struct sockaddr_in * >::iterator it = 
        interface_list.find(if_name);
    if (it == interface_list.end()) {
        return(ETH_ERR_INTERFACE);
    } else {
    */
    if (is_open) {
        int status = Close();
        if (status != ETH_NO_ERR) {
            return(status);
        }
    }
    interface = if_name;
    //serv_addr = *(it->second);
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
     //long save_file_flags;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //serv_addr.sin_addr.s_addr = inet_addr(recv_address.c_str());
    serv_addr.sin_port = htons(recv_port);
    receive_address_compare = inet_addr(send_address.c_str());

    socket_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_descriptor == -1) {
        return(ETH_ERR_SOCK);
    }
    int val = 1;
    setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    int flags = fcntl(socket_descriptor, F_GETFL);
    if (fcntl(socket_descriptor, F_SETFL, O_NONBLOCK | flags) < 0) {
        std::cout << "Non-blocking socket setup failed" << std::endl;
        return(ETH_ERR_BLOCK);
    }
    int return_val = bind(socket_descriptor,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
    if ( return_val < 0 ) {
        std::cout << "Socket did not bind, probably in use: " << return_val << std::endl;
        std::cout << "Errno: " << errno << " - " << strerror(errno) << std::endl;
        return(ETH_ERR_BIND);
    }
    is_open = true;
    return(ETH_NO_ERR);
    //}
}

int StandardSocket::Open() {
    return(Open(interface));
}

int StandardSocket::Close() {
    if (is_open) {
        ssize_t close_rc;
        close_rc = close(socket_descriptor);
        if(close_rc == -1) {
            //std::cerr<<"Error: close call failed"<<std::endl;
            //return ETH_ERR_CLOSE;
            return(ETH_ERR_CLOSE);
        }
        is_open = false;
    }
    return(ETH_NO_ERR);
}

int StandardSocket::send(const std::string & send_address, int port, const std::vector<char> & data)
{
	int socket_descriptor;
	struct sockaddr_in address;

	memset(&address,0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(send_address.c_str());
	address.sin_port = htons(port);
	socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    int val(1);
    setsockopt(socket_descriptor, SOL_SOCKET, SO_BROADCAST, &val, sizeof(val));

	int err_or_size = sendto(socket_descriptor, 
                             (char *)(&data[0]),
                             data.size(),
                             0,
                             (struct sockaddr*)&address, sizeof(address));
	close(socket_descriptor);
    // Maintain compatability with previous ethernet, though it's probably not
    // necessary as no send command checks for a return value.
	return(err_or_size);
	//return(0);
}

int StandardSocket::send(const std::vector<char> & data) {
    return(send(send_address, send_port, data));
}

int StandardSocket::recv(std::deque<char> & data)
{
    struct sockaddr_in remote_addr;
    socklen_t address_len = sizeof(remote_addr);

    ssize_t recv_rc; 
    char buf[DATALENGTH]; //Datalength is currently 1024, but UDP max should be 576
    //recv_rc = recvfrom(socket_descriptor, buf, sizeof(buf), 0, (struct sockaddr*)&serv_addr, (socklen_t *)&len);
    recv_rc = recvfrom(socket_descriptor, buf, sizeof(buf), 0, (struct sockaddr*)&remote_addr, &address_len);

    if(recv_rc == -1 && errno != EAGAIN) { 
        return ETH_ERR_RX; //receive error
    } 
    else if( (recv_rc == 0) | (errno == EAGAIN)) {  // no data
        errno = 0;   // clear the error
    }
    else {
        errno = 0;	// clear the error //necessary?
        if (remote_addr.sin_addr.s_addr == receive_address_compare) {
            data.insert(data.end(),buf,buf+recv_rc);
        }
    }

    return(ETH_NO_ERR);
}

int StandardSocket::recv(std::vector<char> & data)
{
    struct sockaddr_in remote_addr;
    socklen_t address_len = sizeof(remote_addr);

    ssize_t recv_rc; 
    char buf[DATALENGTH]; //Datalength is currently 1024, but UDP max should be 576
    //recv_rc = recvfrom(socket_descriptor, buf, sizeof(buf), 0, (struct sockaddr*)&serv_addr, (socklen_t *)&len);
    recv_rc = recvfrom(socket_descriptor, buf, sizeof(buf), 0, (struct sockaddr*)&remote_addr, &address_len);

    if(recv_rc == -1 && errno != EAGAIN) { 
        return ETH_ERR_RX; //receive error
    } 
    else if( (recv_rc == 0) | (errno == EAGAIN)) {  // no data
        errno = 0;   // clear the error
    }
    else {
        errno = 0;	// clear the error //necessary?
        if (remote_addr.sin_addr.s_addr == receive_address_compare) {
            data.assign(buf,buf+recv_rc);
        }
    }

    return(ETH_NO_ERR);
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
bool StandardSocket::list(std::vector<std::string> & list) {
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
        //return(std::vector<std::string>());
        return(false);
    }

    ifr         = ifc.ifc_req;
    nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
    for(int i = 0; i < nInterfaces; i++) {
        struct ifreq *item = &ifr[i];
        list.push_back(std::string(item->ifr_name));
        interface_list[item->ifr_name] = (struct sockaddr_in *) &(item->ifr_addr);
	}
	return true;
}
