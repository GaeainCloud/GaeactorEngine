#ifndef GAEACTOR_TRANSMIT_REMOTE_RECEIVER_H
#define GAEACTOR_TRANSMIT_REMOTE_RECEIVER_H

#include <QObject>
#include "gaeactor_transmit_define.h"
#include <QHash>
#include <QList>
#include <QVector>
#include "../gaeactor_transmit_receiver_base.h"
namespace gaeactortransmit {

class GaeactorTransmitRemoteReceiver : public GaeactorTransmitReceiver
{
    Q_OBJECT
public:
    explicit GaeactorTransmitRemoteReceiver(QObject *parent = nullptr);
    virtual ~GaeactorTransmitRemoteReceiver();

private:


};
}
#endif // GAEACTOR_TRANSMIT_REMOTE_RECEIVER_H
