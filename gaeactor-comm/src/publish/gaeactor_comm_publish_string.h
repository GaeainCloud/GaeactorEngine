#ifndef GAEACTOR_COMM_PUBLISH_STRING_H
#define GAEACTOR_COMM_PUBLISH_STRING_H

#include "../gaeactor_comm_base.h"


namespace gaeactorcomm {

class GaeactorCommPublishString:public GaeactorCommBase
{
public:
    explicit GaeactorCommPublishString(const COMM_CHANNEL_INFO &_channel_info);
    virtual ~GaeactorCommPublishString();
    virtual bool sendData(const std::string &pData) override;
//    virtual void deal_process();
};
}
#endif // GAEACTOR_COMM_PUBLISH_TEXT_H
