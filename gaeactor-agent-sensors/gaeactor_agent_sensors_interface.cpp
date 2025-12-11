#include "gaeactor_agent_sensors_processor.h"
#include "gaeactor_agent_sensors_interface.h"

namespace gaeactoragentsensors {
GaeactorAgentSensors::GaeactorAgentSensors(QObject *parent)
    :QObject(parent)
{

}

GaeactorAgentSensors::~GaeactorAgentSensors()
{

}

void GaeactorAgentSensors::inputWaveData(const wave_smd_hexidx &wave_smd_hexidx_data)
{
    return GaeactorAgentSensorsProcessor::getInstance().dealWaveData(wave_smd_hexidx_data);
}

void GaeactorAgentSensors::registDisplayCallback(display_hexidx_update_callback func)
{
    return GaeactorAgentSensorsProcessor::getInstance().registDisplayCallback(func);
}

void GaeactorAgentSensors::registEventUpdateCallback(event_update_callback func)
{
    GaeactorAgentSensorsProcessor::getInstance().registEventUpdateCallback(func);
}
}
