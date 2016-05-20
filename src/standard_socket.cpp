#include <miil/standard_socket.h>
#include <errno.h>

StandardSocket::StandardSocket(
        const std::string & if_name,
        const std::string & recv_a,
        const std::string & send_a,
        int recv_p,
        int send_p) :
    Ethernet(if_name, recv_a, send_a, recv_p, send_p)
{
}

StandardSocket::StandardSocket(
	const std::string & send_a,
        int recv_p) :
    Ethernet("", "", send_a, recv_p, 21845)
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
    if (is_open) {
        int status = Close();
        if (status != ETH_NO_ERR) {
            return(status);
        }
    }
    interface = if_name;

    // Run the list command to force the class instance to populate the list of
    // interfaces and then set the send address to this interface's ip address
    // so that we can bind to it.
    std::vector<std::string> interface_name_list;
    list(interface_name_list);
    recv_address = std::string(inet_ntoa(interface_list[interface]->sin_addr));


    struct sockaddr_in serv_addr;
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(recv_address.c_str());
    serv_addr.sin_port = htons(recv_port);

    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd == -1) {
        return(ETH_ERR_SOCK);
    }
    // Set the SO_REUSEADDR so that we don't block other programs from binding
    // to the same port.  Should be a problem in practice, but often if there
    // is not a correct shutdown, the port becomes blocked for quite some time.
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    int flags = fcntl(fd, F_GETFL);
    if (fcntl(fd, F_SETFL, O_NONBLOCK | flags) < 0) {
        return(ETH_ERR_BLOCK);
    }

    // Set the buffer size to net.core.rmem_max = 26214400 so that the socket
    // doesn't drop so many UDP packets (in theory).
    int sockbufsize = 26214400;
    int ret = setsockopt(
            fd, SOL_SOCKET, SO_RCVBUF,
            (char *)&sockbufsize,  (int)sizeof(sockbufsize));
    if (ret < 0) {
        return(ETH_ERR_RCVBUF);
    }

    // Bind the socket to the port
    int return_val = bind(fd,
                          (struct sockaddr *)&serv_addr,
                          sizeof(serv_addr));
    if (return_val < 0) {
        return(ETH_ERR_BIND);
    }
    is_open = true;

    memset(&fds, 0, sizeof(fds));
    fds.fd = fd;
    fds.events = POLLIN;

    return(ETH_NO_ERR);
}

int StandardSocket::Open() {
    return(Open(interface));
}

int StandardSocket::Close() {
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

int StandardSocket::send(
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
    // Maintain compatability with previous ethernet, though it's probably not
    // necessary as no send command checks for a return value.
	return(err_or_size);
	//return(0);
}

int StandardSocket::send(const std::vector<char> & data) {
    return(send(send_address, send_port, data));
}

int StandardSocket::recv(std::vector<char> & data) {
    char buf[DATALENGTH];
    ssize_t recv_rc(0);

    if (poll(&fds, 1, timeout_ms) > 0) {
        if (fds.revents & (POLLIN)) {
            recv_rc = ::recv(fd, buf, sizeof(buf), 0);
        }
    } else {
        return(ETH_NO_ERR);
    }

    if (recv_rc < 0) {
        if (errno == EAGAIN) {
            errno = 0;
            return(ETH_NO_ERR);
        } else {
            errno = 0;
            return(ETH_ERR_RX);
        }
    } else if (recv_rc == 0) {
        return(ETH_NO_ERR);
    } else {
        data.insert(data.end(), buf, buf + recv_rc);
        return(recv_rc);
    }
}
