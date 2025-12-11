#include "gaeactor_event_engine_processor.h"
#include "gaeactor_event_engine_interface.h"
#include <QDebug>
namespace gaeactoreventengine
{
GaeactorEventEngine::GaeactorEventEngine(QObject *parent)
    :QObject(parent)
{

}

GaeactorEventEngine::~GaeactorEventEngine()
{

}

tagPathInfo GaeactorEventEngine::getPath(const TYPE_ULID &uildsrc, const TYPE_ULID &dst)
{
    return GaeactorEventEngineProcessor::getInstance().getPath(uildsrc, dst);
}

void GaeactorEventEngine::setCheckEnable(bool newBChecking)
{
    GaeactorEventEngineProcessor::getInstance().setCheckEnable(newBChecking);
}

bool GaeactorEventEngine::isCheckable() const
{
    return GaeactorEventEngineProcessor::getInstance().isCheckable();
}

void GaeactorEventEngine::setEnableDeal(bool bEnbale)
{
    return GaeactorEventEngineProcessor::getInstance().setEnableDeal(bEnbale);
}

bool GaeactorEventEngine::isEnableDeal() const
{
    return GaeactorEventEngineProcessor::getInstance().isEnableDeal();
}

void GaeactorEventEngine::registEventUpdateCallback(event_update_callback func)
{
    GaeactorEventEngineProcessor::getInstance().registEventUpdateCallback(func);
}

void GaeactorEventEngine::registSensorUpdateCallback(sensor_update_callback func)
{
    GaeactorEventEngineProcessor::getInstance().registSensorUpdateCallback(func);
}

void GaeactorEventEngine::refreshEvents()
{
    return GaeactorEventEngineProcessor::getInstance().refreshEvents();
}

void GaeactorEventEngine::refreshEvents_by_cores(const TYPE_ULID &entityid, const H3INDEX &h3Indexsrc, const FLOAT64 &hgt, const transdata_entityposinfo& eninfo)
{
    return GaeactorEventEngineProcessor::getInstance().refreshEvents_by_cores(entityid, h3Indexsrc, hgt, eninfo);
}

void GaeactorEventEngine::refreshEvents_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslistret)
{
    return GaeactorEventEngineProcessor::getInstance().refreshEvents_by_sensors(sensorid,sensingmediaid,hexidxslistret);
}

GaeactorEventEngine &GaeactorEventEngine::getInstance()
{
    static GaeactorEventEngine gaeactoreventengine;
    return gaeactoreventengine;
}
}
