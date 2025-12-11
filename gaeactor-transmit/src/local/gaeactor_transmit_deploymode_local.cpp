#include "gaeactor_transmit_deploymode_local.h"

#include <QDebug>
#include "gaeactor_transmit_local_publisher.h"
#include "gaeactor_transmit_local_subscriber.h"

namespace gaeactortransmit
{
GaeactorTransmitDeployModeLocal::GaeactorTransmitDeployModeLocal(QObject *parent)
    :GaeactorTransmitDeployModeBase(parent)
{

}

GaeactorTransmitDeployModeLocal::~GaeactorTransmitDeployModeLocal()
{

}

GaeactorTransmitReceiver *GaeactorTransmitDeployModeLocal::createReceiver()
{
    return new GaeactorTransmitLocalSubscriber(this);
}

GaeactorTransmitSender *GaeactorTransmitDeployModeLocal::createSender()
{
    return new GaeactorTransmitLocalPublisher(this);
}


}
