#ifndef GAEACTOR_AGENT_SENSORS_INTERFACE_H
#define GAEACTOR_AGENT_SENSORS_INTERFACE_H

#include "gaeactor_agent_sensors_global.h"
#include <QObject>
#include "gaeactor_agent_sensors_define.h"

namespace gaeactoragentsensors {
class GAEACTOR_AGENT_SENSORS_EXPORT GaeactorAgentSensors : public QObject
{
    Q_OBJECT
public:
    explicit GaeactorAgentSensors(QObject *parent = nullptr);
    virtual ~GaeactorAgentSensors();
    void inputWaveData(const gaeactoragentsensors::wave_smd_hexidx & wave_smd_hexidx_data);
    void registDisplayCallback(display_hexidx_update_callback func);
    void registEventUpdateCallback(event_update_callback func);

};
}
#endif // GAEACTOR_AGENT_SENSORS_INTERFACE_H
