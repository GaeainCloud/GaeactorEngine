/*
   Copyright © 2017-2019, orcaer@yeah.net  All rights reserved.

   Author: orcaer@yeah.net

   Last modified: 2019-12-31

   Description: https://github.com/wlgq2/uv-cpp
*/

#ifndef   GAEACTOR_COMM_TCP_CLIENT_H
#define   GAEACTOR_COMM_TCP_CLIENT_H

#include  <functional>
#include  <memory>

#include "gaeactor_comm_commbase.h"
#include "gaeactor_comm_define.h"
#include "TcpConnection.hpp"

namespace gaeactorcomm
{
class Timer;
class GAEACTOR_COMM_EXPORT GaeactorCommTcpClient : public GaeactorCommCommBase
{
public:
    enum E_CONNECT_STATUS_TYPE
    {
        E_CONNECT_STATUS_TYPE_SUCCESSED,
        E_CONNECT_STATUS_TYPE_FAILED,
        E_CONNECT_STATUS_TYPE_CLOSED
    };
    using connect_status_callback = std::function<void(E_CONNECT_STATUS_TYPE)>;
public:
    GaeactorCommTcpClient(bool tcpNoDelay = true);
    virtual ~GaeactorCommTcpClient();

    bool is_tcp_no_delay();
    void set_tcp_no_delay(bool isNoDelay);
    void connect(const std::string &ip, const UINT16 &port);
    void close(std::function<void(GaeactorCommTcpClient*)> callback);

    int send(const char* buf, unsigned int size, AfterWriteCallback callback = nullptr);
    void send_in_loop(const char* buf, unsigned int size, AfterWriteCallback callback);

    void set_callback(OnMessageCallback _message_receive_callback, connect_status_callback _connect_status_callback);
    TcpConnectionPtr get_connection();
    bool is_connected();
protected:

    void on_connect(bool success);
    void on_connect_close(std::string& name);
    void on_message(TcpConnectionPtr connection, const char* buf, ssize_t size);
    void trigger_connect_fail();
    void reconnect();
private:

    void update();
    void run_connect_callback(GaeactorCommTcpClient::E_CONNECT_STATUS_TYPE success);
    void on_close(std::string& name);
    void timeout_callback(Timer* ptimer);
private:
    std::shared_ptr<uv_tcp_t> m_socket;
    uv_connect_t* m_connect;
    bool m_tcp_no_delay;

    connect_status_callback m_connect_status_callback;
    OnMessageCallback m_message_receive_callback;

    TcpConnectionPtr m_connection;
    Timer* m_pTimer;
};

using TcpClientPtr = std::shared_ptr<GaeactorCommTcpClient>;
}
#endif
