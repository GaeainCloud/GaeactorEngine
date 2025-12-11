#ifndef GAEACTOR_TRANSMIT_DEPLOYMODE_REMOTE_H
#define GAEACTOR_TRANSMIT_DEPLOYMODE_REMOTE_H

#include <QObject>
#include "gaeactor_transmit_define.h"
#include <QHash>
#include <QList>
#include <QVector>

#include "../gaeactor_transmit_deploymode_base.h"
namespace gaeactortransmit {

class GaeactorTransmitDeployModeRemote : public GaeactorTransmitDeployModeBase
{
    Q_OBJECT
public:
    explicit GaeactorTransmitDeployModeRemote(QObject *parent = nullptr);
    virtual ~GaeactorTransmitDeployModeRemote();
    virtual GaeactorTransmitReceiver* createReceiver() override;
    virtual GaeactorTransmitSender* createSender() override;
private:

};
}
#endif // GAEACTOR_TRANSMIT_DEPLOYMODE_REMOTE_H
