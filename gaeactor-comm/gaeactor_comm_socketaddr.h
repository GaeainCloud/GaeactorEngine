#ifndef GAEACTOR_COMM_SPCKETADDR_H
#define GAEACTOR_COMM_SPCKETADDR_H

#include <uv.h>
#include <string>
#include <functional>
#include <memory>
#include "gaeactor_comm_global.h"

namespace gaeactorcomm {


using UVTcpPtr = std::shared_ptr<uv_tcp_t>;
class GAEACTOR_COMM_EXPORT SocketAddr
{
public:
    enum IPV
    {
        Ipv4 = 0,
        Ipv6
    };
    SocketAddr();
    SocketAddr(const std::string&& ip, unsigned short port, IPV ipv = Ipv4);
    SocketAddr(const std::string& ip, unsigned short port, IPV ipv = Ipv4);
    SocketAddr(const sockaddr* addr, IPV ipv = Ipv4);

    void init(const std::string&& ip, unsigned short port, IPV ipv = Ipv4);
    const sockaddr* Addr();
    void toStr(std::string& str);
    std::string toStr();

    IPV Ipv();

    static  void AddrToStr(uv_tcp_t* client, std::string& addrStr, IPV ipv = Ipv4);
    static  uint16_t  GetIpAndPort(const sockaddr_storage* addr, std::string& out, IPV ipv = Ipv4);

private:
    std::string ip_;
    unsigned short port_;
    IPV ipv_;
    sockaddr_in ipv4_;
    sockaddr_in6 ipv6_;
};


typedef std::function<void()> close_callback;
typedef std::function<void(SocketAddr&, const char*, uint32_t)> message_receive_callback;
}
#endif // GAEACTOR_COMM_SPCKETADDR_H
