#ifndef GAEACTOR_AGENT_CORES_INTERFACE_H
#define GAEACTOR_AGENT_CORES_INTERFACE_H

#include "gaeactor_agent_cores_global.h"
#include <QObject>
#include "gaeactor_agent_cores_define.h"

namespace gaeactoragentcores {
class GAEACTOR_AGENT_CORES_EXPORT GaeactorAgentCores : public QObject
{
    Q_OBJECT
public:
    explicit GaeactorAgentCores(QObject *parent = nullptr);
    virtual ~GaeactorAgentCores();
    void inputPosData(const gaeactoragentcores::pos_hexidx &pos_hexidx_data);
    void inputPosattData(const gaeactoragentcores::posatt_hexidx &pos_hexidx_data);
    void registDisplayCallback(display_pos_update_callback func);
    void registEventUpdateCallback(event_update_callback func);
};
}
#endif // GAEACTOR_AGENT_CORES_INTERFACE_H
