#ifndef GAEACTOR_COMM_PUBLISH_STRUCT_H
#define GAEACTOR_COMM_PUBLISH_STRUCT_H

#include "../gaeactor_comm_base.h"

#include <ecal/ecal.h>
// a binary payload object that handles
// SSimpleStruct WriteFull and WriteModified functionality
template <typename T>
class CStructPayload : public eCAL::CPayloadWriter
{
public:
    // Write the complete SSimpleStruct to the shared memory
    bool WriteFull(void* buf_, size_t len_) override
    {
        // check available size and pointer
        if (len_ < GetSize() || buf_ == nullptr) return false;

        // create a new struct and update its content
        // copy complete struct into the memory
        *static_cast<T*>(buf_) = m_trans_struct;

        return true;
    };

    // Modify the SSimpleStruct in the shared memory
    bool WriteModified(void* buf_, size_t len_) override
    {
        // check available size and pointer
        if (len_ < GetSize() || buf_ == nullptr) return false;

        // update the struct in memory
        *static_cast<T*>(buf_) = m_trans_struct;

        return true;
    };

    size_t GetSize() override { return sizeof(T); };
public:
    T m_trans_struct;
};

namespace gaeactorcomm {
template <typename T>
class GaeactorCommPublishStruct:public GaeactorCommBase
{
public:
    explicit GaeactorCommPublishStruct(const COMM_CHANNEL_INFO &_channel_info,const char* structTypeName)
        : GaeactorCommBase(_channel_info)
    {
        m_pPublisher= new eCAL::CPublisher(_channel_info.m_topic.c_str(),{ structTypeName, "custom", "" });
#ifdef USING_ZERO_COPY
        m_pPublisher->ShmEnableZeroCopy(true);
#endif
    }
    virtual ~GaeactorCommPublishStruct()
    {

    }

    bool sendStructData(const T &pData)
    {        
        CStructPayload<T> _struct_payload;
        _struct_payload.m_trans_struct = pData;
        size_t isendlen = m_pPublisher->Send(_struct_payload);
//        return isendlen == pData.size() ? true : false;
        return isendlen == sizeof(T) ? true : false;
    }

};
}
#endif // GAEACTOR_COMM_PUBLISH_TEXT_H
