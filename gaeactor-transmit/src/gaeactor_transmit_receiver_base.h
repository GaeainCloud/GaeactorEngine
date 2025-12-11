#ifndef GAEACTOR_TRANSMIT_RECEIVER_BASE_H
#define GAEACTOR_TRANSMIT_RECEIVER_BASE_H

#include <QObject>
#include "gaeactor_transmit_define.h"
#include <QHash>
#include <QList>
#include <QVector>
#include "gaeactor_transmit_base.h"

namespace gaeactortransmit {

class GaeactorTransmitReceiver : public GaeactorTransmitBase
{
    Q_OBJECT
public:
    explicit GaeactorTransmitReceiver(QObject *parent = nullptr);
    virtual ~GaeactorTransmitReceiver();

private:
};
}
#endif // GAEACTOR_TRANSMIT_RECEIVER_BASE_H
