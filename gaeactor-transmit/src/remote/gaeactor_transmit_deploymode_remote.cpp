#include "gaeactor_transmit_deploymode_remote.h"

#include <QDebug>

#include "gaeactor_transmit_remote_receiver.h"
#include "gaeactor_transmit_remote_sender.h"
namespace gaeactortransmit
{
GaeactorTransmitDeployModeRemote::GaeactorTransmitDeployModeRemote(QObject *parent)
    :GaeactorTransmitDeployModeBase(parent)
{

}

GaeactorTransmitDeployModeRemote::~GaeactorTransmitDeployModeRemote()
{

}

GaeactorTransmitReceiver *GaeactorTransmitDeployModeRemote::createReceiver()
{
    return new GaeactorTransmitRemoteReceiver(this);
}

GaeactorTransmitSender *GaeactorTransmitDeployModeRemote::createSender()
{

    return new GaeactorTransmitRemoteSender(this);
}


}
