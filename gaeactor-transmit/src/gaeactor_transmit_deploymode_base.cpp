#include "gaeactor_transmit_deploymode_base.h"

#include <QDebug>

namespace gaeactortransmit
{
GaeactorTransmitDeployModeBase::GaeactorTransmitDeployModeBase(QObject *parent)
    :m_transmitType(E_DEPLOYMODE_TYPE_NULL)
{

}

GaeactorTransmitDeployModeBase::~GaeactorTransmitDeployModeBase()
{

}

void GaeactorTransmitDeployModeBase::initTransmitType(const E_DEPLOYMODE_TYPE &transmitType)
{
    m_transmitType = transmitType;
}

}
