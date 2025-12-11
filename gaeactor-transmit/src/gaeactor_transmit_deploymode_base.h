#ifndef GAEACTOR_TRANSMIT_DEPLOYMODE_BASE_H
#define GAEACTOR_TRANSMIT_DEPLOYMODE_BASE_H

#include <QObject>
#include "gaeactor_transmit_define.h"
#include <QHash>
#include <QList>
#include <QVector>

namespace gaeactortransmit {

class GaeactorTransmitReceiver;
class GaeactorTransmitSender;

class GaeactorTransmitDeployModeBase : public QObject
{
    Q_OBJECT

public:
    explicit GaeactorTransmitDeployModeBase(QObject *parent = nullptr);
    virtual ~GaeactorTransmitDeployModeBase();

    virtual GaeactorTransmitReceiver* createReceiver() = 0;
    virtual GaeactorTransmitSender* createSender() = 0;
    void initTransmitType(const E_DEPLOYMODE_TYPE& transmitType);
private:

    E_DEPLOYMODE_TYPE m_transmitType;

};
}
#endif // GAEACTOR_TRANSMIT_DEPLOYMODE_BASE_H
