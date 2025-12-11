#include "gaeactor_agent_cores_processor.h"
#include "gaeactor_agent_cores_interface.h"

namespace gaeactoragentcores {
GaeactorAgentCores::GaeactorAgentCores(QObject *parent)
    :QObject(parent)
{

}

GaeactorAgentCores::~GaeactorAgentCores()
{

}

void GaeactorAgentCores::inputPosData(const gaeactoragentcores::pos_hexidx &pos_hexidx_data)
{
    GaeactorAgentCoresProcessor::getInstance().dealPosData(pos_hexidx_data);
}

void GaeactorAgentCores::inputPosattData(const posatt_hexidx &pos_hexidx_data)
{
    GaeactorAgentCoresProcessor::getInstance().dealPosattData(pos_hexidx_data);
}

void GaeactorAgentCores::registDisplayCallback(display_pos_update_callback func)
{
    GaeactorAgentCoresProcessor::getInstance().registDisplayCallback(func);
}


void GaeactorAgentCores::registEventUpdateCallback(event_update_callback func)
{
    GaeactorAgentCoresProcessor::getInstance().registEventUpdateCallback(func);
}
}
