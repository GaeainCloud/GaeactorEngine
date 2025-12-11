
#ifndef AGENT_SERVICE_HPP
#define AGENT_SERVICE_HPP

#include "../dto/AgentDto.hpp"

#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"


namespace oatpp {
namespace parser {
namespace json {
namespace mapping {

class ObjectMapper;
}
}
}
}
class AgentService
{
public:
    AgentService();
    ~AgentService();
private:
    typedef oatpp::web::protocol::http::Status Status;
public:
    oatpp::Object<GeoJson_Return_Dto> inputCheckRoutePath(const oatpp::Object<GeoJson_linestring_Dto>& dto);
    oatpp::Object<GeoJson_Return_Dto> inputCheckRoutePathEx(const oatpp::Object<GeoJson_FeatureCollections_Dto> &dto);

    oatpp::Object<GeoJson_Return_Dto> inputCheckRoutePathAccumulate(const oatpp::Object<GeoJson_linestring_Dto>& dto);
    oatpp::Object<GeoJson_Return_Dto> inputCheckRoutePathExAccumulate(const oatpp::Object<GeoJson_FeatureCollections_Dto> &dto);

    oatpp::Object<GeoJson_linestring_opearte_Dto> inputCheckRoutePathSettings(const oatpp::Object<GeoJson_linestring_opearte_Dto> &dto);


    oatpp::Object<GeoJson_Return_Dto> getLineConflictsInfo(const oatpp::Object<GeoJson_query_line_id_Dto> &dto);
    oatpp::Object<GeoJson_Return_Dto> getConflictsPtsInfo(const oatpp::Object<GeoJson_query_pt_id_Dto> &dto);
    oatpp::Object<GeoJson_clear_pt_id_Dto> clearConflictsPtsInfo(const oatpp::Object<GeoJson_clear_pt_id_Dto> &dto);
    oatpp::Object<GeoJson_tasks_Dto> getTaskInfo();

    oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> LatLngToHex(const oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> &dto);
    oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> HexToLatLng(const oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> &dto);
#ifdef USING_GUI_SHOW
    oatpp::Object<GeoJson_Return_Dto> inputDisplayCheckRoutePathResult(const oatpp::Object<GeoJson_Return_Dto> &dto);
#endif
public:
    void setDataCallback(http_receive_callback func);
private:
    http_receive_callback m_httpreceive_callback;
    std::shared_ptr<oatpp::parser::json::mapping::ObjectMapper> m_jsonmapper;
};


#endif //AGENT_SERVICE_HPP
