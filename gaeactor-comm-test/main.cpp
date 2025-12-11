
#include <QCoreApplication>
#include <iostream>
#include "src/OriginalDateTime.h"

#pragma pack(1)
struct pack_info
{
    int32_t pack_index;
    char data[1024*1024*100];
};
#pragma pack()
int64_t total_len = 0;
int32_t pack_index = 0;
#if 0
#include "gaeactor_comm_tcp_server.hpp"
#include "gaeactor_comm_tcp_client.h"

void on_new_connect(std::weak_ptr<gaeactorcomm::TcpConnection> status)
{
    std::cout <<" on_new_connect :  \n";
}
void on_close_connect(std::weak_ptr<gaeactorcomm::TcpConnection> status)
{

    std::cout <<" on_close_connect : \n";
}

void on_connect(gaeactorcomm::GaeactorCommTcpClient::E_CONNECT_STATUS_TYPE status)
{
    std::cout <<" on_connect : " <<status<<" \n";
}

void on_receive_msg(gaeactorcomm::TcpConnectionPtr,const char* msg,ssize_t len)
{
    total_len += len;
    pack_info * _pack_info = reinterpret_cast<pack_info*>(const_cast<char*>(msg));

    static UINT64 datasize = 0;
    UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    static UINT64 lasttimestampmapms = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    UINT64 intervalms = fabs(currentTimeStamp - lasttimestampmapms);
    datasize+=len;
    static double KB_s = 0.0;
    static double MB_s = 0.0;
    static double GB_s = 0.0;
    if(intervalms > 1000)
    {
        lasttimestampmapms = currentTimeStamp;
        KB_s = datasize/1024;
        MB_s = KB_s/1024;
        GB_s = MB_s/1024;
        datasize = 0;
    }
    std::cout <<_pack_info->pack_index<< " received total len: " <<total_len<<" cur len "<< len <<"  "<<KB_s<<" KB/S "<<MB_s<< " MB/S "<<GB_s<< " GB/S\n";

}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString groupname="server";
    switch (argc)
    {
    case 2:
    {
        groupname = a.arguments().at(1);
    }
    break;
    }

    gaeactorcomm::GaeactorCommCommBase* pGaeactorCommCommBase = nullptr ;
    if(groupname == "server")
    {
        gaeactorcomm::GaeactorCommTcpServer* pGaeactorCommTcpServer = new gaeactorcomm::GaeactorCommTcpServer();
        pGaeactorCommCommBase = pGaeactorCommTcpServer;
        pGaeactorCommTcpServer->set_callback(on_receive_msg,on_new_connect,on_close_connect);
        pGaeactorCommTcpServer->bind_and_listen("127.0.0.1", 12345);
        pGaeactorCommTcpServer->start();

    }
    else
    {
        gaeactorcomm::GaeactorCommTcpClient* pGaeactorCommTcpClient = new gaeactorcomm::GaeactorCommTcpClient();
        pGaeactorCommCommBase = pGaeactorCommTcpClient;
        pGaeactorCommTcpClient->set_callback(on_receive_msg,on_connect);
        pGaeactorCommTcpClient->connect("127.0.0.1", 12345);
        pGaeactorCommTcpClient->start();
        pack_info *_pack_info = new pack_info;

        while (1) {
            if(!pGaeactorCommTcpClient->is_connected())
            {
                stdutils::OriDateTime::sleep(1);
                continue;
            }
            _pack_info->pack_index = pack_index++;
            UINT32 len = sizeof(pack_info);
            total_len += len;
            static UINT64 datasize = 0;
            UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
            static UINT64 lasttimestampmapms = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
            UINT64 intervalms = fabs(currentTimeStamp - lasttimestampmapms);
            datasize+=len;
            static double KB_s = 0.0;
            static double MB_s = 0.0;
            static double GB_s = 0.0;
            if(intervalms > 1000)
            {
                lasttimestampmapms = currentTimeStamp;
                KB_s = datasize/1024;
                MB_s = KB_s/1024;
                GB_s = MB_s/1024;
                datasize = 0;
            }
            std::cout <<_pack_info->pack_index<< " sended total len: " <<total_len<<" cur len "<< len <<"  "<<KB_s<<" KB/S "<<MB_s<< " MB/S "<<GB_s<< " GB/S\n";

            pGaeactorCommTcpClient->send((const char *)_pack_info, len,nullptr);
            stdutils::OriDateTime::sleep(1);
        }
        delete _pack_info;

    }
    return a.exec();
}
#else

#include "gaeactor_comm_udp.h"

#include "./proto/protoc/DebugInfo.pb.h"
void close()
{
    std::cout << " close \n";
}
void message_receive_callback(gaeactorcomm::SocketAddr& addr, const char* msg, UINT32 len)
{
    total_len += len;
    msg::DebugInfo::msg_Debug_Info_Array _msg_Debug_Info_Array;
    _msg_Debug_Info_Array.ParseFromArray(msg, len);
    static UINT64 datasize = 0;
    UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    static UINT64 lasttimestampmapms = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    UINT64 intervalms = fabs(currentTimeStamp - lasttimestampmapms);
    datasize+=len;
    static double KB_s = 0.0;
    static double MB_s = 0.0;
    static double GB_s = 0.0;
    if(intervalms > 1000)
    {
        lasttimestampmapms = currentTimeStamp;
        KB_s = datasize/1024;
        MB_s = KB_s/1024;
        GB_s = MB_s/1024;
        datasize = 0;
    }
    static uint64_t recvpackindex = 0;
    std::cout <<_msg_Debug_Info_Array.packindex()<<" "<<recvpackindex<<" "<<(_msg_Debug_Info_Array.packindex() - recvpackindex)<< " received total len: " <<total_len<<" cur len "<< len <<"  "<<KB_s<<" KB/S "<<MB_s<< " MB/S "<<GB_s<< " GB/S\n";
    recvpackindex++;
}





int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString groupname="server";
    switch (argc)
    {
    case 2:
    {
        groupname = a.arguments().at(1);
    }
    break;
    }

    gaeactorcomm::GaeactorCommUdp udp;
    if(groupname == "server")
    {
        pack_info *_pack_info = new pack_info;
        udp.init(gaeactorcomm::GaeactorCommUdp::E_TYPE_SERVER, "127.0.0.1", 12345);
        udp.set_callback(&message_receive_callback, nullptr);
        udp.start();
        while (1) {
            _pack_info->pack_index = pack_index++;
            UINT32 len = sizeof(pack_info);
            total_len += len;
            static UINT64 datasize = 0;
            UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
            static UINT64 lasttimestampmapms = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
            UINT64 intervalms = fabs(currentTimeStamp - lasttimestampmapms);
            datasize+=len;
            static double KB_s = 0.0;
            static double MB_s = 0.0;
            static double GB_s = 0.0;
            if(intervalms > 1000)
            {
                lasttimestampmapms = currentTimeStamp;
                KB_s = datasize/1024;
                MB_s = KB_s/1024;
                GB_s = MB_s/1024;
                datasize = 0;
            }
            std::cout <<_pack_info->pack_index<< " sended total len: " <<total_len<<" cur len "<< len <<"  "<<KB_s<<" KB/S "<<MB_s<< " MB/S "<<GB_s<< " GB/S\n";

            gaeactorcomm::SocketAddr addr("127.0.0.1",55555);
            udp.sendto(addr, (const char *)_pack_info, len);
            stdutils::OriDateTime::sleep(1);
        }
        delete _pack_info;
    }
    else
    {
        udp.init(gaeactorcomm::GaeactorCommUdp::E_TYPE_SERVER, "127.0.0.1", 55555);
        udp.set_callback(&message_receive_callback, nullptr);
        udp.start();
    }
    return a.exec();
}
#endif

