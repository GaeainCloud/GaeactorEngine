#ifndef GAEACTOR_COMM_UDP_H
#define GAEACTOR_COMM_UDP_H

#include <uv.h>
#include <string>
#include "gaeactor_comm_global.h"
#include "gaeactor_comm_commbase.h"
namespace stdutils
{
class OriThread;
};
namespace gaeactorcomm {

class EventLoop;
class GAEACTOR_COMM_EXPORT GaeactorCommUdp : public GaeactorCommCommBase
{
public:

    enum E_TYPE{
        E_TYPE_CLIENT,
        E_TYPE_SERVER
    };
    GaeactorCommUdp();
    GaeactorCommUdp(const E_TYPE& _type,const std::string& ip, const uint16_t& port);
    virtual ~GaeactorCommUdp();

    void init(const E_TYPE &_type, const std::string &ip, const uint16_t &port);

    virtual void stop() override;
    int send(const char* msg, uint32_t ilen);
    int sendto(SocketAddr& to, const char* msg, uint32_t ilen);
    void close();
    void set_callback(message_receive_callback _message_receive_callback, close_callback _close_callback);
private:

    void init_server();
    void init_client();
    void start_recv();

    void on_close_completed();
    void on_message(const sockaddr* from, const char* data, unsigned size);

    static void on_recv_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags);
private:
    uv_udp_t* m_socket_handle;
    E_TYPE m_type;
    message_receive_callback m_message_receive_callback;
    close_callback m_close_callback;
};
}
#endif // GAEACTOR_COMM_UDP_H
