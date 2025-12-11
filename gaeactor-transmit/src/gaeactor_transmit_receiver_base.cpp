#include "gaeactor_transmit_receiver_base.h"

#include <QDebug>

namespace gaeactortransmit
{
GaeactorTransmitReceiver::GaeactorTransmitReceiver(QObject *parent)
    :GaeactorTransmitBase(E_TRANSMIT_TYPE_RECEIVER, parent)
{

}

GaeactorTransmitReceiver::~GaeactorTransmitReceiver()
{

}


}
