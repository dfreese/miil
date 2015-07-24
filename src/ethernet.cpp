#include <ethernet.h>

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
    is_open(false)
{
}
