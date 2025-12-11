#ifndef GAEACTOR_TRANSMIT_DEFINE_H
#define GAEACTOR_TRANSMIT_DEFINE_H

#include "base_define.h"

#pragma pack(1)
enum E_DEPLOYMODE_TYPE : INT32
{
    E_DEPLOYMODE_TYPE_NULL = 0x00,
    E_DEPLOYMODE_TYPE_LOCAL_RECV = 0x01,
    E_DEPLOYMODE_TYPE_LOCAL_SEND = 0x02,
    E_DEPLOYMODE_TYPE_LOCAL_RECV_SEND = E_DEPLOYMODE_TYPE_LOCAL_RECV|E_DEPLOYMODE_TYPE_LOCAL_SEND,
    E_DEPLOYMODE_TYPE_REMOTE = 0x04,
};

enum E_TRANSMIT_TYPE
{
    E_TRANSMIT_TYPE_NULL,
    E_TRANSMIT_TYPE_RECEIVER,
    E_TRANSMIT_TYPE_SENDER
};


enum E_CHANNEL_TRANSMITDATA_TYPE : INT32
{
    E_CHANNEL_TRANSMITDATA_TYPE_NULL = 0x00,
    E_CHANNEL_TRANSMITDATA_TYPE_CUSTOM_ARRAY = 0x01,
    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS = 0x02,
    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS_ARRAY = 0x03,

    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POSATT = 0x04,
    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POSATT_ARRAY = 0x05,

    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR = 0x06,
    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_HEXIDX = 0x07,
    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX = 0x08,
    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_INTERSECTION = 0x09,
    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ECHOWAVE = 0x0a,
    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT = 0x0b,

    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_HEXIDX_ARRAY = 0x0c,
    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX_ARRAY = 0x0d,
    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_INTERSECTION_ARRAY = 0x0e,
    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ECHOWAVE_ARRAY = 0x0f,
    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY = 0x10,

    E_CHANNEL_TRANSMITDATA_TYPE_PATH = 0x11,
    E_CHANNEL_TRANSMITDATA_TYPE_PATH_ARRAY = 0x12,

    E_CHANNEL_TRANSMITDATA_TYPE_SENSOR_UPDATE = 0x13,
    E_CHANNEL_TRANSMITDATA_TYPE_SENSOR_UPDATE_ARRAY = 0x14,

    E_CHANNEL_TRANSMITDATA_TYPE_SENSOR_SENSINGMEDIA_UPDATE = 0x15,

    E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR_ARRAY = 0x16,

    E_CHANNEL_TRANSMITDATA_TYPE_AGENT_RELATION = 0x17,
    E_CHANNEL_TRANSMITDATA_TYPE_AGENT_SNR = 0x18,
    E_CHANNEL_TRANSMITDATA_TYPE_AGENT_COMMSTACK_RESULT = 0x19,
    E_CHANNEL_TRANSMITDATA_TYPE_SIMPARAMS = 0x20,
    E_CHANNEL_TRANSMITDATA_TYPE_SMDINFO = 0x21,
    E_CHANNEL_TRANSMITDATA_TYPE_CUSTOM_MSG = 0x22,
    E_CHANNEL_TRANSMITDATA_TYPE_PREJUSTMENTLINE = 0x23,
};

typedef struct tagChannelInfo
{
    std::string m_service;
    std::string m_instance;
    std::string m_event;
    tagChannelInfo()
    {
        m_service= "";
        m_instance= "";
        m_event = "";
    }

    inline bool operator == (const tagChannelInfo &other) const
    {
        if(this == &other)
            return true;
        return ((m_service == other.m_service) &&
                (m_instance == other.m_instance) &&
                (m_event == other.m_event));
    }

    inline bool operator != (const tagChannelInfo &other) const
    {
        if(this == &other)
            return false;
        return !((m_service == other.m_service) &&
                 (m_instance == other.m_instance) &&
                 (m_event == other.m_event));

    }

    inline tagChannelInfo& operator = (const tagChannelInfo &other)
    {
        if(this == &other)
            return *this;
        m_service = other.m_service;
        m_instance = other.m_instance;
        m_event = other.m_event;
        return *this;
    }
}CHANNEL_INFO;


namespace std {
template<> struct hash<tagChannelInfo> {
    size_t operator()(const tagChannelInfo& p)const {
        return hash<std::string>()(p.m_service)+
               hash<std::string>()(p.m_instance)+
               hash<std::string>()(p.m_event);
    }
};
};

typedef std::function<void (const E_CHANNEL_TRANSMITDATA_TYPE &, const BYTE*, const UINT32&, const BYTE*, const UINT32&)> receive_callback;



#define ECAL_CHANNEL_BINARYDATA_STR ("gaeactor_binarydata")

#pragma pack()

#endif // GAEACTOR_TRANSMIT_DEFINE_H
