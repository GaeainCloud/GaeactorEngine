#ifndef GAEACTOR_TRANSMIT_DEPLOYMODE_LOCAL_H
#define GAEACTOR_TRANSMIT_DEPLOYMODE_LOCAL_H

#include <QObject>
#include "gaeactor_transmit_define.h"
#include <QHash>
#include <QList>
#include <QVector>

#include "../gaeactor_transmit_deploymode_base.h"
namespace gaeactortransmit {

class GaeactorTransmitDeployModeLocal : public GaeactorTransmitDeployModeBase
{
    Q_OBJECT
public:
    explicit GaeactorTransmitDeployModeLocal(QObject *parent = nullptr);
    virtual ~GaeactorTransmitDeployModeLocal();
    virtual GaeactorTransmitReceiver* createReceiver() override;
    virtual GaeactorTransmitSender* createSender() override;
private:

};
}
#endif // GAEACTOR_TRANSMIT_DEPLOYMODE_LOCAL_H
