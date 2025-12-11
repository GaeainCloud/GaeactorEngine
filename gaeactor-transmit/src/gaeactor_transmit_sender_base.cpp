#include "gaeactor_transmit_sender_base.h"

#include <QDebug>

namespace gaeactortransmit
{
GaeactorTransmitSender::GaeactorTransmitSender(QObject *parent)
    :GaeactorTransmitBase(E_TRANSMIT_TYPE_SENDER, parent)
{

}

GaeactorTransmitSender::~GaeactorTransmitSender()
{

}

void GaeactorTransmitSender::transmitData(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType,const BYTE *pData, UINT32 iLen)
{

}

void *GaeactorTransmitSender::loanTransmitBuffer(UINT32 iLen)
{
    return nullptr;
}

void GaeactorTransmitSender::publish()
{

}


}
