#ifndef GAEACTOR_TRANSMIT_REMOTE_SENDER_H
#define GAEACTOR_TRANSMIT_REMOTE_SENDER_H

#include <QObject>
#include "gaeactor_transmit_define.h"
#include <QHash>
#include <QList>
#include <QVector>
#include "../gaeactor_transmit_sender_base.h"
namespace gaeactortransmit {

class GaeactorTransmitRemoteSender : public GaeactorTransmitSender
{
    Q_OBJECT
public:
    explicit GaeactorTransmitRemoteSender(QObject *parent = nullptr);
    virtual ~GaeactorTransmitRemoteSender();
private:


};
}
#endif // GAEACTOR_TRANSMIT_REMOTE_SENDER_H
