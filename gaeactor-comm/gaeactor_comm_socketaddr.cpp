#include "gaeactor_comm_socketaddr.h"
#include "src/OriginalThread.h"
#include "src/OriginalDateTime.h"
#if (defined(_WIN32)||defined(_WIN64)) /* WINDOWS */
#include <winsock2.h>
#else /* UNIX */
#endif
namespace gaeactorcomm
{


SocketAddr::SocketAddr()
{

}

SocketAddr::SocketAddr(const std::string&& ip, unsigned short port, IPV ipv)
{
    init(std::move(ip),port,ipv);
}

SocketAddr::SocketAddr(const std::string& ip, unsigned short port, IPV ipv)
    :SocketAddr(std::move(ip), port, ipv)
{

}

SocketAddr::SocketAddr(const sockaddr* addr, IPV ipv)
    :ipv_(ipv)
{
    if (ipv_ == Ipv4)
    {
        ipv4_ = *(reinterpret_cast<const sockaddr_in*>(addr));
    }
    else
    {
        ipv6_ = *(reinterpret_cast<const sockaddr_in6*>(addr));
    }
    port_ = GetIpAndPort((const sockaddr_storage *)(addr), ip_, ipv);
}

void SocketAddr::init(const std::string &&ip, unsigned short port, IPV ipv)
{
    ip_=ip;
    port_=port;
    ipv_=ipv;


    if (ipv == Ipv6)
    {
        ::uv_ip6_addr(ip.c_str(), port, &ipv6_);
    }
    else
    {
        ::uv_ip4_addr(ip.c_str(), port, &ipv4_);
    }
}

const sockaddr * SocketAddr::Addr()
{
    return (ipv_ == Ipv6) ? reinterpret_cast<const sockaddr*>(&ipv6_) : reinterpret_cast<const sockaddr*>(&ipv4_);
}

void SocketAddr::toStr(std::string & str)
{
    str = ip_ + ":" + std::to_string(port_);
}

std::string SocketAddr::toStr()
{
    std::string str = ip_ + ":" + std::to_string(port_);
    return str;
}

SocketAddr::IPV SocketAddr::Ipv()
{
    return ipv_;
}

void SocketAddr::AddrToStr(uv_tcp_t* client, std::string& addrStr, IPV ipv)
{
    struct sockaddr_storage addr;
    int len = sizeof(struct sockaddr_storage);
    ::uv_tcp_getpeername(client, (struct sockaddr *)&addr, &len);

    uint16_t port = GetIpAndPort(&addr, addrStr, ipv);
    addrStr += ":" + std::to_string(port);
}

uint16_t SocketAddr::GetIpAndPort(const sockaddr_storage* addr, std::string& out, IPV ipv)
{
    auto inet = (Ipv6 == ipv) ? AF_INET6 : AF_INET;
    if (Ipv6 == ipv)
    {
        char dst[64];
        wchar_t  wip[INET6_ADDRSTRLEN];
        int len = INET6_ADDRSTRLEN;
        struct sockaddr_in6* addr6 = (struct sockaddr_in6 *)addr;
        //低版本windows可能找不到inet_ntop函数。
#if    _MSC_VER
        DWORD size = sizeof(wip);
        WSAAddressToString((LPSOCKADDR)addr6, sizeof(sockaddr_in6), NULL, wip, &size);
        int cnt = 64;
        if (!WideCharToMultiByte(CP_UTF8, 0, wip, len, dst, cnt, NULL, NULL))
        {
            return NULL;
        }
        out = std::string(dst);
        auto index = out.rfind(":");
        if (index >= 0)
        {
            out.resize(index);
        }
        return (htons(addr6->sin6_port));
#else
        std::string str(inet_ntop(inet, (void *)&(addr6->sin6_addr), dst, 64));
        out.swap(str);
        return(htons(addr6->sin6_port));
#endif
    }
    else
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        std::string str(inet_ntoa(addr4->sin_addr));
        out.swap(str);
        return htons(addr4->sin_port);
    }
}
}
