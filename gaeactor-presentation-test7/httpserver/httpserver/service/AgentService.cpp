#include "../../../ConcurrentHashMapManager.h"
#include "AgentService.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "../../../mainwindow.h"


AgentService::AgentService()
{
    m_jsonmapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    m_jsonmapper->getSerializer()->getConfig()->useBeautifier = false;
}

AgentService::~AgentService()
{

}

oatpp::Object<GeoJson_Return_Dto> AgentService::inputCheckRoutePath(const oatpp::Object<GeoJson_linestring_Dto> &dto)
{
    return MainWindow::getInstance().pConcurrentHashMapManager()->inputCheckRoutePath(dto);
}

oatpp::Object<GeoJson_Return_Dto> AgentService::inputCheckRoutePathEx(const oatpp::Object<GeoJson_FeatureCollections_Dto> &dto)
{
    return MainWindow::getInstance().pConcurrentHashMapManager()->inputCheckRoutePathEx(dto);
}

oatpp::Object<GeoJson_Return_Dto> AgentService::inputCheckRoutePathAccumulate(const oatpp::Object<GeoJson_linestring_Dto> &dto)
{
    return MainWindow::getInstance().pConcurrentHashMapManager()->inputCheckRoutePathAccumulate(dto);
}

oatpp::Object<GeoJson_Return_Dto> AgentService::inputCheckRoutePathExAccumulate(const oatpp::Object<GeoJson_FeatureCollections_Dto> &dto)
{
    return MainWindow::getInstance().pConcurrentHashMapManager()->inputCheckRoutePathExAccumulate(dto);
}

oatpp::Object<GeoJson_linestring_opearte_Dto> AgentService::inputCheckRoutePathSettings(const oatpp::Object<GeoJson_linestring_opearte_Dto> &dto)
{
    return MainWindow::getInstance().pConcurrentHashMapManager()->inputCheckRoutePathSettings(dto);
}

oatpp::Object<GeoJson_Return_Dto> AgentService::getLineConflictsInfo(const oatpp::Object<GeoJson_query_line_id_Dto> &dto)
{

    return MainWindow::getInstance().pConcurrentHashMapManager()->getLineConflictsInfo(dto);
}

oatpp::Object<GeoJson_Return_Dto> AgentService::getConflictsPtsInfo(const oatpp::Object<GeoJson_query_pt_id_Dto> &dto)
{

    return MainWindow::getInstance().pConcurrentHashMapManager()->getConflictsPtsInfo(dto);
}

oatpp::Object<GeoJson_clear_pt_id_Dto> AgentService::clearConflictsPtsInfo(const oatpp::Object<GeoJson_clear_pt_id_Dto> &dto)
{
    return MainWindow::getInstance().pConcurrentHashMapManager()->clearConflictsPtsInfo(dto);
}

oatpp::Object<GeoJson_tasks_Dto> AgentService::getTaskInfo()
{
    return MainWindow::getInstance().pConcurrentHashMapManager()->getTaskInfo();
}

oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> AgentService::LatLngToHex(const oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> &dto)
{
    return MainWindow::getInstance().pConcurrentHashMapManager()->LatLngToHex(dto);
}

oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> AgentService::HexToLatLng(const oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> &dto)
{
    return MainWindow::getInstance().pConcurrentHashMapManager()->HexToLatLng(dto);
}
#ifdef USING_GUI_SHOW
oatpp::Object<GeoJson_Return_Dto> AgentService::inputDisplayCheckRoutePathResult(const oatpp::Object<GeoJson_Return_Dto> &dto)
{
    return MainWindow::getInstance().pConcurrentHashMapManager()->inputDisplayCheckRoutePathResult(dto);
}
#endif

void AgentService::setDataCallback(http_receive_callback func)
{
    m_httpreceive_callback = std::move(func);
}


