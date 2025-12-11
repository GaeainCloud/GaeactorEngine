/*
   Copyright © 2017-2020, orcaer@yeah.net  All rights reserved.

   Author: orcaer@yeah.net

   Last modified: 2019-12-31

   Description: https://github.com/wlgq2/uv-cpp
*/

#include <string>

#include "gaeactor_comm_tcp_client.h"
#include "TcpConnection.hpp"
#include "EventLoop.hpp"
#include <iostream>
#include "Timer.hpp"

using namespace gaeactorcomm;
using namespace std;


GaeactorCommTcpClient::GaeactorCommTcpClient(bool tcpNoDelay)
    :GaeactorCommCommBase(),
    m_connect(new uv_connect_t()),
    m_tcp_no_delay(tcpNoDelay),
    m_connect_status_callback(nullptr),
    m_message_receive_callback(nullptr),
    m_connection(nullptr),
    m_pTimer(new Timer(m_loop, 1, 3000, std::bind(&GaeactorCommTcpClient::timeout_callback,this,std::placeholders::_1)))
{
    m_connect->data = static_cast<void*>(this);
}

GaeactorCommTcpClient::~GaeactorCommTcpClient()
{
    delete m_connect;
}

bool GaeactorCommTcpClient::is_tcp_no_delay()
{
    return m_tcp_no_delay;
}

void GaeactorCommTcpClient::set_tcp_no_delay(bool isNoDelay)
{
    m_tcp_no_delay = isNoDelay;
}


void GaeactorCommTcpClient::connect(const std::string &ip, const UINT16 &port)
{
    initaddr(ip,port);
    reconnect();
}

void GaeactorCommTcpClient::on_connect(bool success)
{
    if(success)
    {
        string name;
        SocketAddr::AddrToStr(m_socket.get(),name,m_socketaddr.Ipv());

        m_connection = make_shared<TcpConnection>(m_loop, name, m_socket);
        m_connection->setMessageCallback(std::bind(&GaeactorCommTcpClient::on_message,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
        m_connection->setConnectCloseCallback(std::bind(&GaeactorCommTcpClient::on_connect_close,this,std::placeholders::_1));
        run_connect_callback(GaeactorCommTcpClient::E_CONNECT_STATUS_TYPE_SUCCESSED);
    }
    else
    {
        if (::uv_is_active((uv_handle_t*)m_socket.get()))
        {
            ::uv_read_stop((uv_stream_t*)m_socket.get());
        }
        if (::uv_is_closing((uv_handle_t*)m_socket.get()) == 0)
        {
            m_socket->data = static_cast<void*>(this);
            ::uv_close((uv_handle_t*)m_socket.get(),
                       [](uv_handle_t* handle)
                       {
                           auto client = static_cast<GaeactorCommTcpClient*>(handle->data);
                           client->trigger_connect_fail();
                       });
        }
    }
}
void GaeactorCommTcpClient::on_connect_close(string& name)
{
    if (m_connection)
    {
        m_connection->close(std::bind(&GaeactorCommTcpClient::on_close,this,std::placeholders::_1));
    }

}
void GaeactorCommTcpClient::on_message(shared_ptr<TcpConnection> connection,const char* buf,ssize_t size)
{
    if(m_message_receive_callback)
    {
        m_message_receive_callback(connection, buf,size);
    }
}

void GaeactorCommTcpClient::close(std::function<void(GaeactorCommTcpClient*)> callback)
{
    if (m_connection)
    {
        m_connection->close([this, callback](std::string&)
                           {
                               //onClose(name);
                               if (callback)
                                   callback(this);
                           });

    }
    else if(callback)
    {
        callback(this);
    }
}

void GaeactorCommTcpClient::trigger_connect_fail()
{
    run_connect_callback(GaeactorCommTcpClient::E_CONNECT_STATUS_TYPE_FAILED);
}


void GaeactorCommTcpClient::reconnect()
{
    update();
    ::uv_tcp_connect(m_connect, m_socket.get(), m_socketaddr.Addr(), [](uv_connect_t* req, int status)
                     {
                         auto handle = static_cast<GaeactorCommTcpClient*>((req->data));
                         if (0 != status)
                         {
                             handle->on_connect(false);
                             return;
                         }

                         handle->on_connect(true);
                     });
}

int GaeactorCommTcpClient::send(const char* buf, unsigned int size, AfterWriteCallback callback)
{
    if (m_connection)
    {
        return m_connection->write(buf, size, callback);
    }
    else if(callback)
    {
        std::cout<<"try write a disconnect connection. \n";
        WriteInfo info = { WriteInfo::Disconnected,const_cast<char*>(buf),size };
        callback(info);
    }
    return -1;
}

void GaeactorCommTcpClient::send_in_loop(const char * buf, unsigned int size, AfterWriteCallback callback)
{
    if (m_connection)
    {
        m_connection->writeInLoop(buf, size, callback);
    }
    else if(callback)
    {
        std::cout<<"try write a disconnect connection. \n";
        WriteInfo info = { WriteInfo::Disconnected,const_cast<char*>(buf),size };
        callback(info);
    }
}

void GaeactorCommTcpClient::set_callback(OnMessageCallback _message_receive_callback, connect_status_callback _connect_status_callback)
{
    m_message_receive_callback = std::move(_message_receive_callback);
    m_connect_status_callback = std::move(_connect_status_callback);
}

TcpConnectionPtr GaeactorCommTcpClient::get_connection()
{
    return m_connection;
}

bool GaeactorCommTcpClient::is_connected()
{
    return m_connection!=nullptr && m_connection->isConnected();
}

void GaeactorCommTcpClient::update()
{
    m_socket = make_shared<uv_tcp_t>();
    ::uv_tcp_init(m_loop->handle(), m_socket.get());
    if (m_tcp_no_delay)
    {
        ::uv_tcp_nodelay(m_socket.get(), 1 );
    }
}

void GaeactorCommTcpClient::run_connect_callback(GaeactorCommTcpClient::E_CONNECT_STATUS_TYPE status)
{
    if (m_connect_status_callback)
    {
        m_connect_status_callback(status);
    }
    if(status == GaeactorCommTcpClient::E_CONNECT_STATUS_TYPE_CLOSED ||
        status == GaeactorCommTcpClient::E_CONNECT_STATUS_TYPE_FAILED)
    {
        if(!m_pTimer->is_started())
        {
            std::cout<<" start timer to reconnect."<<m_socketaddr.toStr()<<" \n";
            m_pTimer->start();
        }
    }
    else if(status == GaeactorCommTcpClient::E_CONNECT_STATUS_TYPE_SUCCESSED )
    {
        if(m_pTimer->is_started())
        {
            std::cout<<" stop timer to reconnect. \n";
            m_pTimer->stop();
        }
    }
}

void GaeactorCommTcpClient::on_close(std::string& name)
{
    //connection_ = nullptr;
    std::cout<<"Close tcp client connection complete."<<m_socketaddr.toStr()<<" \n";
    run_connect_callback(GaeactorCommTcpClient::E_CONNECT_STATUS_TYPE_CLOSED);
}

void GaeactorCommTcpClient::timeout_callback(Timer *ptimer)
{
    if(!this->is_connected())
    {
        std::cout<<" timer trigger reconnect. "<<m_socketaddr.toStr()<<"\n";
        reconnect();
    }
    else
    {
        std::cout<<" stop timer to reconnect. \n";
        m_pTimer->stop();
    }
}
