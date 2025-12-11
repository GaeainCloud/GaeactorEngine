#ifndef GAEACTOR_COMM_DEFINE_H
#define GAEACTOR_COMM_DEFINE_H

#include "base_define.h"

#pragma pack(1)

enum E_COMM_CHANNEL_DATE_TYPE:UINT8
{
    E_COMM_CHANNEL_DATE_TYPE_STRING,
    E_COMM_CHANNEL_DATE_TYPE_BINARY,
    E_COMM_CHANNEL_DATE_TYPE_STRUCT,
    E_COMM_CHANNEL_DATE_TYPE_PROTOBUF
};

enum E_COMM_CHANNEL_TYPE:UINT8
{
    E_COMM_CHANNEL_TYPE_NULL,
    E_COMM_CHANNEL_TYPE_PUBLISHER,
    E_COMM_CHANNEL_TYPE_SUBSCRIBER,
};

typedef struct tagCommChannelInfo
{
    std::string m_service;
    std::string m_topic;
    E_COMM_CHANNEL_DATE_TYPE m_eCommChannelDateType;
    E_COMM_CHANNEL_TYPE m_eCommChannelType;
    tagCommChannelInfo()
    {
        m_service= "";
        m_topic= "";
        m_eCommChannelDateType = E_COMM_CHANNEL_DATE_TYPE_STRING;
        m_eCommChannelType = E_COMM_CHANNEL_TYPE_NULL;
    }

    tagCommChannelInfo(const std::string& _service,
                       const std::string& _topic,
                       E_COMM_CHANNEL_DATE_TYPE _eCommChannelDateType,
                       E_COMM_CHANNEL_TYPE _eCommChannelType)
        :m_service(_service),
        m_topic(_topic),
        m_eCommChannelDateType(_eCommChannelDateType),
        m_eCommChannelType(_eCommChannelType)
    {

    }


    inline bool operator == (const tagCommChannelInfo &other) const
    {
        if(this == &other)
            return true;
        return ((m_service == other.m_service) &&
                (m_topic == other.m_topic) &&
                (m_eCommChannelDateType == other.m_eCommChannelDateType) &&
                (m_eCommChannelType == other.m_eCommChannelType));
    }

    inline bool operator != (const tagCommChannelInfo &other) const
    {
        if(this == &other)
            return false;
        return !((m_service == other.m_service) &&
                 (m_topic == other.m_topic) &&
                 (m_eCommChannelDateType == other.m_eCommChannelDateType) &&
                 (m_eCommChannelType == other.m_eCommChannelType));
    }

    inline tagCommChannelInfo& operator = (const tagCommChannelInfo &other)
    {
        if(this == &other)
            return *this;
        m_service = other.m_service;
        m_topic = other.m_topic;
        m_eCommChannelDateType = other.m_eCommChannelDateType;
        m_eCommChannelType = other.m_eCommChannelType;
        return *this;
    }
}COMM_CHANNEL_INFO;


namespace std {
template<> struct hash<tagCommChannelInfo> {
    size_t operator()(const tagCommChannelInfo& p)const {
        return hash<std::string>()(p.m_service)+
               hash<std::string>()(p.m_topic)+
               hash<UINT8>()(p.m_eCommChannelDateType)+
               hash<UINT8>()(p.m_eCommChannelType);
    }
};
};


enum E_COMM_CHANNEL_TRANSE_TYPE:UINT8
{
    E_COMM_CHANNEL_TRANSE_TYPE_NULL
};

struct tagCommChannelTransBase
{
    E_COMM_CHANNEL_TRANSE_TYPE m_eCommChannelTransType;
    tagCommChannelTransBase()
        :m_eCommChannelTransType(E_COMM_CHANNEL_TRANSE_TYPE_NULL)
    {

    }
};


typedef std::function<void (const COMM_CHANNEL_INFO &, const BYTE*, const UINT32&)> comm_binary_receive_callback;
typedef std::function<void (const COMM_CHANNEL_INFO &, const std::string&)> comm_string_receive_callback;

#pragma pack()


struct WriteInfo
{
    static const int Disconnected = -1;
    int status;
    char* buf;
    unsigned long size;
};

using AfterWriteCallback =  std::function<void (WriteInfo& )> ;
using DefaultCallback = std::function<void()>;



//#include <ecal/ecal.h>


// a simple struct to demonstrate
// zero copy modifications
struct alignas(4) SSimpleStruct
{
    uint32_t version      = 1;
    uint16_t rows         = 5;
    uint16_t cols         = 3;
    uint32_t clock        = 0;
    uint8_t  bytes[5 * 3] = { 0 };
};

#endif // GAEACTOR_COMM_DEFINE_H
