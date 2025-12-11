#ifndef GAEACTOR_COMM_PUBLISH_BINARY_H
#define GAEACTOR_COMM_PUBLISH_BINARY_H

#include "../gaeactor_comm_base.h"

namespace gaeactorcomm {

class GaeactorCommPublishBinary:public GaeactorCommBase
{
public:
    explicit GaeactorCommPublishBinary(const COMM_CHANNEL_INFO &_channel_info);
    virtual ~GaeactorCommPublishBinary();
    virtual bool sendData(const BYTE *pData, UINT32 iLen) override;

};
}
#endif // GAEACTOR_COMM_PUBLISH_BINARY_H
