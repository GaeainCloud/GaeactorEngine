#ifndef GAEACTOR_TRANSMIT_LOCAL_SUBSCRIBER_H
#define GAEACTOR_TRANSMIT_LOCAL_SUBSCRIBER_H

#include <QObject>
#include "gaeactor_transmit_define.h"
#include <QHash>
#include <QList>
#include <QVector>

#include "../gaeactor_transmit_receiver_base.h"
namespace iox {
namespace popo
{
class UntypedSubscriber;
class Listener;
}
}


namespace gaeactortransmit {

class GaeactorTransmitLocalSubscriber : public GaeactorTransmitReceiver
{
    Q_OBJECT
public:
    explicit GaeactorTransmitLocalSubscriber(QObject *parent = nullptr);
    virtual ~GaeactorTransmitLocalSubscriber();
    virtual void init(CHANNEL_INFO *channelinfo ) override;
    void printMempoolInfo();
    static void onSampleReceivedCallback(iox::popo::UntypedSubscriber* subscriber,GaeactorTransmitLocalSubscriber* self);
    iox::popo::UntypedSubscriber *pSubscriber() const;

    void attachListenerEvent(iox::popo::Listener *newPListener);
    void detachListenerEvent();

    void dealSubscriberInfo(iox::popo::UntypedSubscriber * const subscriber);
#ifdef USING_SINGLE_CHANNEL
    void data_deal_func_callback(const BYTE* pData, UINT32 iLen);
#endif
private:

    iox::popo::Listener* m_pListener;
    iox::popo::UntypedSubscriber* m_pSubscriber;
};
}
#endif // GAEACTOR_TRANSMIT_LOCAL_SUBSCRIBER_H
