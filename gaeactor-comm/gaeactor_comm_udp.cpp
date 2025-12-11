#include "gaeactor_comm_udp.h"
#include "src/OriginalThread.h"
#include "src/OriginalDateTime.h"
#include "EventLoop.hpp"
#if (defined(_WIN32)||defined(_WIN64)) /* WINDOWS */
#include <winsock2.h>
#else /* UNIX */
#endif
namespace gaeactorcomm
{
GaeactorCommUdp::GaeactorCommUdp()
    :GaeactorCommCommBase(),
    m_socket_handle(new uv_udp_t()),
    m_message_receive_callback(nullptr),
    m_close_callback(nullptr)
{
    m_socket_handle->data = this;
}

GaeactorCommUdp::GaeactorCommUdp(const E_TYPE &_type, const std::string &ip, const UINT16 &port)
    :GaeactorCommCommBase(),
    m_socket_handle(new uv_udp_t()),
    m_message_receive_callback(nullptr),
    m_close_callback(nullptr)
{
    m_socket_handle->data = this;
    init(_type, ip, port);
}


GaeactorCommUdp::~GaeactorCommUdp()
{
    if(m_socket_handle)
    {
        delete m_socket_handle;
    }
}

void GaeactorCommUdp::stop()
{
    close();
    GaeactorCommCommBase::stop();
}



void GaeactorCommUdp::init(const E_TYPE &_type, const std::string &ip, const UINT16 &port)
{
    initaddr(ip,port);
    m_type = _type;
    switch (m_type) {
    case E_TYPE_CLIENT:
    {
        init_client();
    }break;
    case E_TYPE_SERVER:
    {
        init_server();
    }
    break;
    default:
        break;
    }
}

void GaeactorCommUdp::on_recv_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
{
    if (nread < 0)
    {
        std::cout << "error" <<uv_strerror(nread)<< "\n";
    }
    else if (nread > 0)
    {
        GaeactorCommUdp* obj = static_cast<GaeactorCommUdp*>(handle->data);
        obj->on_message(addr, buf->base, (unsigned)nread);
    }
    delete[](buf->base);
//    uv_close((uv_handle_t*)handle, [](uv_handle_t* handle)
//             {
//                 uv_is_closing(handle);
//             });
}

void GaeactorCommUdp::on_message(const sockaddr *from, const char *data, unsigned int size)
{
    if (nullptr != m_message_receive_callback)
    {
        SocketAddr addr(from, m_socketaddr.Ipv());
        m_message_receive_callback(addr, data, size);
    }
}

void GaeactorCommUdp::init_server()
{
    uv_udp_init(m_loop->handle(), m_socket_handle);
    uv_udp_bind(m_socket_handle, m_socketaddr.Addr(), UV_UDP_REUSEADDR);
    start_recv();
}

void GaeactorCommUdp::init_client()
{
    uv_udp_init(m_loop->handle(), m_socket_handle);
    start_recv();
}

void GaeactorCommUdp::start_recv()
{
    auto on_alloc_buffer_cb=[](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
    {
        buf->base = new char[suggested_size];
#if _MSC_VER
        buf->len = (ULONG)suggested_size;
#else
        buf->len = suggested_size;
#endif
    };

    uv_udp_recv_start(m_socket_handle,  on_alloc_buffer_cb, &GaeactorCommUdp::on_recv_cb);
}


int GaeactorCommUdp::send(const char *msg, UINT32 ilen)
{
    auto on_send_cb=[](uv_udp_send_t *handle, int status)
    {
        if (status)
        {
            std::cout << "udp send error : " <<uv_strerror(status)<< "\n";
        }
//        uv_close((uv_handle_t*)handle->handle, [](uv_handle_t* handle)
//                 {
//                     uv_is_closing(handle);
//                 });
        delete handle;
    };
    uv_udp_send_t *send_req = new uv_udp_send_t();
    uv_buf_t buffer = uv_buf_init(const_cast<char*>(msg), ilen);
    return uv_udp_send(send_req, m_socket_handle, &buffer, 1, m_socketaddr.Addr(), on_send_cb);
}

int GaeactorCommUdp::sendto(SocketAddr &to, const char *msg, UINT32 ilen)
{
    auto on_send_cb=[](uv_udp_send_t *handle, int status)
    {
        if (status)
        {
            std::cout << "udp send error : " <<uv_strerror(status)<< "\n";
        }
//        uv_close((uv_handle_t*)handle->handle, [](uv_handle_t* handle)
//                 {
//                     uv_is_closing(handle);
//                 });
        delete handle;
    };
    uv_udp_send_t *send_req = new uv_udp_send_t();
    uv_buf_t buffer = uv_buf_init(const_cast<char*>(msg), ilen);
    return uv_udp_send(send_req, m_socket_handle, &buffer, 1, to.Addr(), on_send_cb);
}

void GaeactorCommUdp::close()
{
    if (uv_is_active((uv_handle_t*)m_socket_handle))
    {
        uv_udp_recv_stop(m_socket_handle);
    }
    if (uv_is_closing((uv_handle_t*)m_socket_handle) == 0)
    {
        uv_close((uv_handle_t*)m_socket_handle, [](uv_handle_t* handle)
                 {
                     GaeactorCommUdp* ptr = static_cast<GaeactorCommUdp*>(handle->data);
                     ptr->on_close_completed();
                 });
    }
    else
    {
        on_close_completed();
    }
}

void GaeactorCommUdp::set_callback(message_receive_callback _message_receive_callback, close_callback _close_callback)
{
    m_message_receive_callback = std::move(_message_receive_callback);
    m_close_callback = std::move(_close_callback);
}


void GaeactorCommUdp::on_close_completed()
{
    if (m_close_callback)
    {
        m_close_callback();
    }
}

}
