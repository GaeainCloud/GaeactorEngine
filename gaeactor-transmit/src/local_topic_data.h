
#ifndef IOX_LOCAL_TOPIC_DATA_H
#define IOX_LOCAL_TOPIC_DATA_H
#include "head_define.h"
#include "iceoryx_posh/iceoryx_posh_types.hpp"

#define CHANNEL_TITLE_SIZE (100)
#pragma pack(1)
enum E_CHANNEL_OPERATE_TYPE : INT32
{
    E_CHANNEL_OPERATE_NULL = 0x1,
    E_CHANNEL_OPERATE_ADD = 0x2,
    E_CHANNEL_OPERATE_REMOVE = 0x4,
};

struct ChannelObject
{
    TYPE_ULID m_channelsrc_ulid;
    E_CHANNEL_OPERATE_TYPE m_channel_operate;
    char m_service[CHANNEL_TITLE_SIZE];
    char m_instance[CHANNEL_TITLE_SIZE];
    char m_event[CHANNEL_TITLE_SIZE];
};

#pragma pack()

const std::string transmit_channel_service("transmit_channel_service");
const std::string transmit_channel_instance("transmit_channel_instance");
const std::string transmit_channel_event("transmit_channel_event");

const iox::capro::IdString_t declare_channel_service("declare_channel_service");
const iox::capro::IdString_t declare_channel_instance("declare_channel_instance");
const iox::capro::IdString_t declare_channel_event("declare_channel_event");

#endif // IOX_LOCAL_TOPIC_DATA_H
