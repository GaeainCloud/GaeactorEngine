#if 0
#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>
#include "gaeactor_transmit_interface.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}
#else
#include <QApplication>
#include <iostream>

#include "KlusterWebSocketClient.h"
#include <iomanip>
#include "gaeactor_transmit_interface.h"


#include "./proto/protoc/CustomTransInfo.pb.h"

typedef std::function<void (const BYTE*pdata, const UINT32& ilen)> BINARY_RECIVE_DATA_CALLBACK_FUNC;

typedef std::function<void (const QString& pdata)> TEXT_RECIVE_DATA_CALLBACK_FUNC;

class Test
{
public:
    Test(const QString& urlstr)
    {
        m_wsclient_ = new KlusterWebSocketClient(QUrl(urlstr));
        m_wsclient_->setBinaryDataCallback(std::bind(&Test::binary_receive_data_call_back,this,std::placeholders::_1,std::placeholders::_2));
        m_wsclient_->Connect();
    }
    ~Test()
    {

    }
    void binary_receive_data_call_back(const BYTE*pdata, const UINT32& ilen)
    {
        const char *frame_head = (const char*)pdata;
        uint32_t *crcval_recv = (uint32_t*)(pdata + sizeof(uint32_t));

        const char *dataptr = (const char *)(pdata + sizeof(uint32_t) + sizeof(uint32_t));

        E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = *((E_CHANNEL_TRANSMITDATA_TYPE*)(dataptr));
        uint32_t irecvlen = *(uint32_t*)(dataptr + sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
        const char *protocoldata_ptr = dataptr + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(uint32_t);

        uint32_t ipayloadlen = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + irecvlen;
//        std::cout<<"-----------------------------------------------------------------------------\n";
//        std::cout<<"recv data "<<ilen<<" datatype 0x"<<std::hex<<std::setw(2)<<channelTransmitDataType<<" recv len "<<std::dec<< irecvlen<<"\n";

        switch (channelTransmitDataType) {
        case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_HEXIDX:
        {
            //        dealtransformHexDataSlot(by);
        }break;
        case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX:
        {
            //        ::msg::AgentPositionInfo::msg_transdata_entityposinfo _AgentRelationInfo;
            //        _AgentRelationInfo.ParseFromArray(protocoldata_ptr, irecvlen);
            ////        if (irecvlen == _AgentRelationInfo.ByteSizeLong())
            //        {

            //            INT32   PARAM_reserved[PARAM_RESERVED_SIZE] = {0};
            //            PARAM_reserved[0] = _AgentRelationInfo.param_reserved1();
            //            PARAM_reserved[1] = _AgentRelationInfo.param_reserved2();
            //            PARAM_reserved[2] = _AgentRelationInfo.param_reserved3();
            //            PARAM_reserved[3] = _AgentRelationInfo.param_reserved4();
            //            PARAM_reserved[4] = _AgentRelationInfo.param_reserved5();
            //            PARAM_reserved[5] = _AgentRelationInfo.param_reserved6();
            //            BYTE* PARAM_reserved_PTR = (BYTE*)(PARAM_reserved);
            //            UINT64* PARAM_agentEntityId = (UINT64*)(PARAM_reserved_PTR + sizeof(INT32) * 2);

            //            auto usingflightid = *PARAM_agentEntityId;

            ////            std::cout<<" flight id "<<usingflightid <<" "<<_AgentRelationInfo.DebugString()<<"\n";
            //        }

//            ::msg::AgentPositionInfo::msg_transdata_entityposinfo _AgentRelationInfo;
//            _AgentRelationInfo.ParseFromArray(protocoldata_ptr, irecvlen);
//            if (irecvlen == _AgentRelationInfo.ByteSizeLong())
//            {
//                std::cout<<_AgentRelationInfo.DebugString()<<"\n";
//            }
        }break;
        case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX_ARRAY:
        {
            //        ::msg::AgentPositionInfo::msg_transentityhexidxpostdata_array _AgentRelationInfo_array;
            //        _AgentRelationInfo_array.ParseFromArray(protocoldata_ptr, irecvlen);
            //        //std::cout <<"protobuf size "<<size<<" len "<<cc<<"\n";
            //        if (irecvlen == _AgentRelationInfo_array.ByteSizeLong())
            //        {
            //            std::cout<<_AgentRelationInfo_array.DebugString()<<"\n";

            //            for(int i = 0; i < _AgentRelationInfo_array.transentityhexidxpostdata_size();i++)
            //            {
            //                const ::msg::AgentPositionInfo::msg_transdata_entityposinfo& _AgentRelationInfo = _AgentRelationInfo_array.transentityhexidxpostdata().at(i).entityinfo();
            //                INT32   PARAM_reserved[PARAM_RESERVED_SIZE] = {0};
            //                PARAM_reserved[0] = _AgentRelationInfo.param_reserved1();
            //                PARAM_reserved[1] = _AgentRelationInfo.param_reserved2();
            //                PARAM_reserved[2] = _AgentRelationInfo.param_reserved3();
            //                PARAM_reserved[3] = _AgentRelationInfo.param_reserved4();
            //                PARAM_reserved[4] = _AgentRelationInfo.param_reserved5();
            //                PARAM_reserved[5] = _AgentRelationInfo.param_reserved6();
            //                BYTE* PARAM_reserved_PTR = (BYTE*)(PARAM_reserved);
            //                UINT64* PARAM_agentEntityId = (UINT64*)(PARAM_reserved_PTR + sizeof(INT32) * 2);

            //                auto usingflightid = *PARAM_agentEntityId;

            //                std::cout<<" flight id "<<usingflightid <<"\n";
            //            }
            //        }

//            ::msg::AgentPositionInfo::msg_transentityhexidxpostdata_array _AgentRelationInfo_array;
//            _AgentRelationInfo_array.ParseFromArray(protocoldata_ptr, irecvlen);
//            if (irecvlen == _AgentRelationInfo_array.ByteSizeLong())
//            {
//                std::cout<<_AgentRelationInfo_array.DebugString()<<"\n";
//            }

        }break;
        case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_INTERSECTION:
        {
            //        dealtransformHexIntersectionDataSlot(by);
        }break;
        case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ECHOWAVE:
        {
            //        dealtransformEchowaveDataSlot(by);
        }break;
        case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ECHOWAVE_ARRAY:
        {
            //        dealtransformEchowaveArrayDataSlot(by);
        }break;
        case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT:
        {
            //        dealtransformEventlistDataSlot(by);
        }
        break;
        case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY:
        {
            //        dealtransformEventlistArrayDataSlot(by);
        }break;
        case E_CHANNEL_TRANSMITDATA_TYPE_PATH_ARRAY:
        {
            //        dealtransformSensorPathArrayDataSlot(by);
        }break;
        case E_CHANNEL_TRANSMITDATA_TYPE_SENSOR_UPDATE:
        {
            //        dealtransformSensorUpdateDataSlot(by);
        }
        break;
        case E_CHANNEL_TRANSMITDATA_TYPE_SENSOR_UPDATE_ARRAY:
        {
            //        dealtransformSensorUpdateArrayDataSlot(by);
        }
        break;
        case E_CHANNEL_TRANSMITDATA_TYPE_AGENT_RELATION:
        {
            //        dealtransformAgentRelationDataSlot(by);
        }
        break;
        case E_CHANNEL_TRANSMITDATA_TYPE_AGENT_SNR:
        {
            //        ::msg::AgentCommSnrInfo::msg_AgentCommSnrInfo  _AgentpositionInfo;
            //        _AgentpositionInfo.ParseFromArray(protocoldata_ptr, irecvlen);
            //        if(irecvlen == _AgentpositionInfo.ByteSizeLong())
            //        {
            //            std::cout<<_AgentpositionInfo.DebugString()<<"\n";
            //        }
        }
        break;
        case E_CHANNEL_TRANSMITDATA_TYPE_AGENT_COMMSTACK_RESULT:
        {
            //        dealtransformAgentCommStckRelustDataSlot(by);
        }
        break;
        case E_CHANNEL_TRANSMITDATA_TYPE_SIMPARAMS:
        {
            //        ::msg::SimParamsInfo::msg_SimParamsInfo simparamsinfo;
            //        simparamsinfo.ParseFromArray(protocoldata_ptr, irecvlen);
            //        if (irecvlen == simparamsinfo.ByteSizeLong())
            //        {
            ////            std::cout<<simparamsinfo.DebugString()<<"\n";
            //        }
        }
        break;
        case E_CHANNEL_TRANSMITDATA_TYPE_SMDINFO:
        {
            //        ::msg::AgentPositionInfo::msg_transdata_smd _msg_transdata_smd;
            //        _msg_transdata_smd.ParseFromArray(protocoldata_ptr, irecvlen);
            //        if(irecvlen == _msg_transdata_smd.ByteSizeLong())
            //        {

            //            //std::cout<<_msg_transdata_smd.DebugString()<<"\n";
            //        }
        }
        break;
        case E_CHANNEL_TRANSMITDATA_TYPE_CUSTOM_MSG:
        {
            ::msg::CustomTransInfo::msg_Agent_Trans_Array _msg_Agent_Trans_Array;
            _msg_Agent_Trans_Array.ParseFromArray(protocoldata_ptr, irecvlen);
            if (irecvlen == _msg_Agent_Trans_Array.ByteSizeLong())
            {
                std::cout<<_msg_Agent_Trans_Array.DebugString()<<"\n";
            }
        }break;
        case E_CHANNEL_TRANSMITDATA_TYPE_PREJUSTMENTLINE:
        {
            //        ::msg::AgentPositionInfo::msg_transprejusdgmentline  _AgentpositionInfo;
            //        _AgentpositionInfo.ParseFromArray(protocoldata_ptr, irecvlen);
            //        if(irecvlen == _AgentpositionInfo.ByteSizeLong())
            //        {
            //            std::cout<<_AgentpositionInfo.DebugString()<<"\n";
            //        }
        }
        break;
        default:
            break;
        }
    }
private:
    KlusterWebSocketClient * m_wsclient_;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString urlstr = QString("ws://127.0.0.1:31769");


    switch (argc)
    {
    case 2:
    {
        urlstr = a.arguments().at(1);
    }
    break;
    default:
        break;
    }
    std::cout << "using remote url" << urlstr.toStdString() << std::endl;
    Test t(urlstr);

    return a.exec();
}

#endif
