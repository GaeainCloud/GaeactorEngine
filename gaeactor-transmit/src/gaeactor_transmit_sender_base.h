#ifndef GAEACTOR_TRANSMIT_SENDER_BASE_H
#define GAEACTOR_TRANSMIT_SENDER_BASE_H

#include <QObject>
#include "gaeactor_transmit_define.h"
#include <QHash>
#include <QList>
#include <QVector>

#include "gaeactor_transmit_base.h"
namespace gaeactortransmit {

class GaeactorTransmitSender : public GaeactorTransmitBase
{
    Q_OBJECT
public:
    explicit GaeactorTransmitSender(QObject *parent = nullptr);
    virtual ~GaeactorTransmitSender();
    virtual void transmitData(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE * pData, UINT32 iLen) override;
    virtual void* loanTransmitBuffer(UINT32 iLen) override;
    virtual void publish() override;
private:
};
}
#endif // GAEACTOR_TRANSMIT_SENDER_BASE_H
