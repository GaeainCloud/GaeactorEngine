/*
   Copyright © 2017-2019, orcaer@yeah.net  All rights reserved.

   Author: orcaer@yeah.net

   Last modified: 2019-12-31

   Description: https://github.com/wlgq2/uv-cpp
*/

#ifndef GAEACTOR_COMM_TCP_SERVER_H

#include <functional>
#include <memory>
#include <set>
#include <map>

#include "TcpConnection.hpp"
#include "TimerWheel.hpp"
#include "gaeactor_comm_commbase.h"

namespace gaeactorcomm
{
class TcpAcceptor;
using connection_status_callback =  std::function<void (std::weak_ptr<TcpConnection> )> ;
//no thread safe.
class GAEACTOR_COMM_EXPORT GaeactorCommTcpServer : public GaeactorCommCommBase
{
public:
    GaeactorCommTcpServer(bool tcpNoDelay = true);
    virtual ~GaeactorCommTcpServer();
    int bind_and_listen(const std::string &ip, const UINT16 &port);
    void close(DefaultCallback callback);
    
    TcpConnectionPtr get_connection(const std::string& name);
    void close_connection(const std::string& name);


    void set_callback(OnMessageCallback _message_receive_callback, connection_status_callback _new_connect_status_callback, connection_status_callback _close_connect_status_callback);

    void send(TcpConnectionPtr connection,const char* buf,unsigned int size, AfterWriteCallback callback = nullptr);
    void send(std::string& name,const char* buf,unsigned int size, AfterWriteCallback callback =nullptr);
    void send_in_loop(TcpConnectionPtr connection,const char* buf,unsigned int size,AfterWriteCallback callback);
    void send_in_loop(std::string& name,const char* buf,unsigned int size,AfterWriteCallback callback);

    void set_timeout(unsigned int);
private:
    void on_accept(EventLoop* loop, UVTcpPtr client);

    void add_connection(std::string& name, TcpConnectionPtr connection);
    void remove_connection(std::string& name);
    void on_message(TcpConnectionPtr connection, const char* buf, ssize_t size);

private:
    bool m_tcp_no_delay;
    std::shared_ptr <TcpAcceptor> m_accetper;
    std::map<std::string ,TcpConnectionPtr>  m_connnections;


    OnMessageCallback m_on_message_callback;
    connection_status_callback m_new_connect_callback;
    connection_status_callback m_close_connect_callback;
    TimerWheel<ConnectionWrapper> m_timerWheel;
};


}
#endif
