/*
   Copyright © 2017-2020, orcaer@yeah.net  All rights reserved.

   Author: orcaer@yeah.net

   Last modified: 2019-12-31

   Description: https://github.com/wlgq2/uv-cpp
*/


#include <functional>
#include <memory>
#include <string>

#include "gaeactor_comm_tcp_server.hpp"
#include "TcpAcceptor.hpp"
#include <iostream>

using namespace std;
using namespace gaeactorcomm;


GaeactorCommTcpServer::GaeactorCommTcpServer(bool tcpNoDelay)
    :GaeactorCommCommBase(),
    m_tcp_no_delay(tcpNoDelay),
    m_accetper(nullptr),
    m_on_message_callback(nullptr),
    m_new_connect_callback(nullptr),
    m_close_connect_callback(nullptr),
    m_timerWheel(m_loop)
{

}

GaeactorCommTcpServer:: ~GaeactorCommTcpServer()
{

}

void GaeactorCommTcpServer::set_timeout(unsigned int seconds)
{
    m_timerWheel.setTimeout(seconds);
}

void GaeactorCommTcpServer::on_accept(EventLoop * loop, UVTcpPtr client)
{
    string key;
    SocketAddr::AddrToStr(client.get(), key, m_socketaddr.Ipv());

    std::cout<<"new connect  " << key<<"\n";
    shared_ptr<TcpConnection> connection(new TcpConnection(loop, key, client));
    if (connection)
    {
        connection->setMessageCallback(std::bind(&GaeactorCommTcpServer::on_message, this, placeholders::_1, placeholders::_2, placeholders::_3));
        connection->setConnectCloseCallback(std::bind(&GaeactorCommTcpServer::close_connection, this, placeholders::_1));
        add_connection(key, connection);
        if (m_timerWheel.getTimeout() > 0)
        {
            auto wrapper = std::make_shared<ConnectionWrapper>(connection);
            connection->setWrapper(wrapper);
            m_timerWheel.insert(wrapper);
        }
        if (m_new_connect_callback)
        {
            m_new_connect_callback(connection);
        }
    }
    else
    {
        std::cout<<"create connection fail. :" << key<<"\n";
    }
}

int GaeactorCommTcpServer::bind_and_listen(const std::string &ip, const UINT16 &port)
{
    initaddr(ip,port);
    m_accetper = std::make_shared<TcpAcceptor>(m_loop, m_tcp_no_delay);
    auto rst = m_accetper->bind(m_socketaddr);
    if (0 != rst)
    {
        return rst;
    }
    m_accetper->setNewConnectionCallback(std::bind(&GaeactorCommTcpServer::on_accept, this, std::placeholders::_1, std::placeholders::_2));
    m_timerWheel.start();
    return m_accetper->listen();
}

void GaeactorCommTcpServer::close(DefaultCallback callback)
{
    if (m_accetper)
        m_accetper->close([this, callback]()
    {
        for (auto& connection : m_connnections)
        {
            connection.second->onSocketClose();
        }
        callback();
    });
}

void GaeactorCommTcpServer::add_connection(std::string& name, TcpConnectionPtr connection)
{
    m_connnections.insert(pair<string,shared_ptr<TcpConnection>>(std::move(name),connection));
}

void GaeactorCommTcpServer::remove_connection(string& name)
{
    m_connnections.erase(name);
}

shared_ptr<TcpConnection> GaeactorCommTcpServer::get_connection(const string &name)
{
    auto rst = m_connnections.find(name);
    if(rst == m_connnections.end())
    {
        return nullptr;
    }
    return rst->second;
}

void GaeactorCommTcpServer::close_connection(const string& name)
{
    auto connection = get_connection(name);
    if (nullptr != connection)
    {
        connection->close([this](std::string& name)
        {
            auto connection = get_connection(name);
            if (nullptr != connection)
            {
                if (m_close_connect_callback)
                {
                    m_close_connect_callback(connection);
                }
                remove_connection(name);
            }

        });
    }
}

void GaeactorCommTcpServer::set_callback(OnMessageCallback _message_receive_callback, connection_status_callback _new_connect_status_callback, connection_status_callback _close_connect_status_callback)
{

    m_on_message_callback = std::move(_message_receive_callback);
    m_new_connect_callback = std::move(_new_connect_status_callback);
    m_close_connect_callback = std::move(_close_connect_status_callback);
}


void GaeactorCommTcpServer::on_message(TcpConnectionPtr connection,const char* buf,ssize_t size)
{
    if(m_on_message_callback)
    {
        m_on_message_callback(connection,buf,size);
    }
    if (m_timerWheel.getTimeout() > 0)
    {
        m_timerWheel.insert(connection->getWrapper());
    }
}

void GaeactorCommTcpServer::send(shared_ptr<TcpConnection> connection,const char* buf,unsigned int size, AfterWriteCallback callback)
{
    if(nullptr != connection)
    {
        connection->write(buf,size, callback);
    }
    else if (callback)
    {
        WriteInfo info = { WriteInfo::Disconnected,const_cast<char*>(buf),size };
        callback(info);
    }
}

void GaeactorCommTcpServer::send(string& name,const char* buf,unsigned int size,AfterWriteCallback callback)
{
    auto connection = get_connection(name);
    send(connection, buf, size, callback);
}

void GaeactorCommTcpServer::send_in_loop(shared_ptr<TcpConnection> connection,const char* buf,unsigned int size,AfterWriteCallback callback)
{
    if(nullptr != connection)
    {
        connection->writeInLoop(buf,size,callback);
    }
    else if (callback)
    {
        std::cout<<"try write a disconnect connection.\n";
        WriteInfo info = { WriteInfo::Disconnected,const_cast<char*>(buf),size };
        callback(info);
    }
}

void GaeactorCommTcpServer::send_in_loop(string& name,const char* buf,unsigned int size,AfterWriteCallback callback)
{
    auto connection = get_connection(name);
    send_in_loop(connection, buf, size, callback);
}

