#ifndef GAEACTOR_TRANSMIT_LOCAL_PUBLISHER_H
#define GAEACTOR_TRANSMIT_LOCAL_PUBLISHER_H

#include <QObject>
#include "gaeactor_transmit_define.h"
#include <QHash>
#include <QList>
#include <QVector>
#include <QMutex>
#include "../gaeactor_transmit_sender_base.h"
namespace iox {
namespace popo
{
class UntypedPublisher;
}
}
namespace gaeactortransmit {

class GaeactorTransmitLocalPublisher : public GaeactorTransmitSender
{
    Q_OBJECT
public:
    explicit GaeactorTransmitLocalPublisher(QObject *parent = nullptr);
    virtual ~GaeactorTransmitLocalPublisher();
    virtual void init(CHANNEL_INFO *channelinfo ) override;
    virtual void transmitData(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType,const BYTE * pData, UINT32 iLen) override;
    virtual void* loanTransmitBuffer(UINT32 iLen) override;
    virtual void publish() override;
    std::string getEventItemName(TYPE_ULID ulid);
    bool appendPublisherItem(const std::string & item);
    bool removePbulisherItem(const std::string & item);
    bool clearPublisherItems();
    UINT32 getPublisherCount();
    void printMempoolInfo();
#ifdef USING_SINGLE_CHANNEL
    void data_deal_func_callback(const BYTE* pData, UINT32 iLen);
#else

    bool isPushing() const;

    bool applying();
    void setApplying(const bool &newApplying);
private:

    std::atomic<bool> m_pushing;
    QMutex m_applyingMutex;
    std::atomic<bool> m_applying;
    void * m_usrpayload;
#endif

private:
    iox::popo::UntypedPublisher *m_pPublisher;
    std::list<std::string> m_publisherItems;
};
}
#endif // GAEACTOR_TRANSMIT_LOCAL_PUBLISHER_H
