#include <miil/standard_socket.h>
#include <errno.h>

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

    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd == -1) {
        return(ETH_ERR_SOCK);
    }
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    int flags = fcntl(fd, F_GETFL);
    if (fcntl(fd, F_SETFL, O_NONBLOCK | flags) < 0) {
        return(ETH_ERR_BLOCK);
    }
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
    struct sockaddr_in remote_addr;
    socklen_t address_len = sizeof(remote_addr);

    char buf[DATALENGTH];
    ssize_t recv_rc(0);

    if (poll(&fds, 1, timeout_ms) > 0) {
        if (fds.revents & (POLLIN)) {
            recv_rc = recvfrom(fd, buf, sizeof(buf), 0,
                               (struct sockaddr*)&remote_addr, &address_len);
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
        if (remote_addr.sin_addr.s_addr == receive_address_compare) {
            data.assign(buf,buf+recv_rc);
        }
    }

    return(ETH_NO_ERR);
}
