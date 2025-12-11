#ifndef AGENTDTO_H_
#define AGENTDTO_H_
#include <functional>
#include <QJsonObject>
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

///////////////////////////////////////////////////////////////////////////////////////////////////
class GeoJson_FeatureCollection_properties_Dto : public oatpp::DTO {
    DTO_INIT(GeoJson_FeatureCollection_properties_Dto, DTO)
    DTO_FIELD(String, code);
    DTO_FIELD(String, startTime);
    DTO_FIELD(String, endTime);
    DTO_FIELD(String, createTime);
    DTO_FIELD(String, Priority);

};
class GeoJson_linestring_id_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_linestring_id_Dto, DTO)
    DTO_FIELD(String, agentid);
    DTO_FIELD(Int32, routeid);
};

class GeoJson_linestring_properties_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_linestring_properties_Dto, DTO)
    DTO_FIELD(Object<GeoJson_linestring_id_Dto>, agentrouteid);
    DTO_FIELD(Object<GeoJson_FeatureCollection_properties_Dto>, featurecollectionpros);
    DTO_FIELD(String, color);    
    DTO_FIELD(Vector<Float64>, heights);
    DTO_FIELD(Float64, heightlimitdown);
    DTO_FIELD(Float64, heightlimitup);
    DTO_FIELD(Vector<String>, linehex);
};

class GeoJson_points_linestring_id_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_points_linestring_id_Dto, DTO)
    DTO_FIELD(Object<GeoJson_linestring_id_Dto>, agentrouteids);
    DTO_FIELD(Object<GeoJson_FeatureCollection_properties_Dto>, featurecollectionpros);
    DTO_FIELD(Float64, height);
    DTO_FIELD(Float64, heightlimitdown);
    DTO_FIELD(Float64, heightlimitup);
};

class GeoJson_point_properties_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_point_properties_Dto, DTO)
    DTO_FIELD(Int32, pointid);
    DTO_FIELD(String, color);
    DTO_FIELD(Vector<Object<GeoJson_points_linestring_id_Dto>>, pointlinestrings);
};
///////////////////////////////////////////////////////////
class GeoJson_geometry_linestring_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_geometry_linestring_Dto, DTO)
    DTO_FIELD(String, type);
    DTO_FIELD(Vector<Vector<Float64>>, coordinates);
    DTO_FIELD(Vector<Vector<Vector<Float64>>>,coordinates_polygon);
};


class GeoJson_features_linstring_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_features_linstring_Dto, DTO)
    DTO_FIELD(String, type);
    DTO_FIELD(Object<GeoJson_linestring_properties_Dto>, properties);
    DTO_FIELD(Object<GeoJson_geometry_linestring_Dto>, geometry);
};
///////////////////////////////////////////////////////////
class GeoJson_geometry_point_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_geometry_point_Dto, DTO)
    DTO_FIELD(String, type);
    DTO_FIELD(Vector<Float64>, coordinates);
};

class GeoJson_features_point_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_features_point_Dto, DTO)
    DTO_FIELD(String, type);
    DTO_FIELD(Object<GeoJson_point_properties_Dto>, properties);
    DTO_FIELD(Object<GeoJson_geometry_point_Dto>, geometry);
};
///////////////////////////////////////////////////////////

class GeoJson_linestring_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_linestring_Dto, DTO)
    DTO_FIELD(String, taskid);
    DTO_FIELD(Boolean, conflictbuilding);
    DTO_FIELD(String, type);
    DTO_FIELD(Int32, resolution);
    DTO_FIELD(Vector<Object<GeoJson_features_linstring_Dto>>, features);
    GeoJson_linestring_Dto()
    {
        taskid = "";
        conflictbuilding = true;
        resolution = 9;
    }
};

class GeoJson_point_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_point_Dto, DTO)
    DTO_FIELD(String, type);
    DTO_FIELD(Int32, resolution);
    DTO_FIELD(Vector<Object<GeoJson_features_point_Dto>>, features);
};
///////////////////////////////////////////////////////////

class GeoJson_Return_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_Return_Dto, DTO)
    DTO_FIELD(String, taskid);
    DTO_FIELD(Int32, count);
    DTO_FIELD(Object<GeoJson_linestring_Dto>, routes);
    DTO_FIELD(Object<GeoJson_point_Dto>, points);
};

class GeoJson_query_line_id_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_query_line_id_Dto, DTO)
    DTO_FIELD(String, taskid);
    DTO_FIELD(String, agentid);
    DTO_FIELD(Int32, routeid);
    DTO_FIELD(String, code);
};


class GeoJson_query_pt_id_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_query_pt_id_Dto, DTO)
    DTO_FIELD(String, taskid);
    DTO_FIELD(Int32, beginindex);
    DTO_FIELD(Int32, count);
};

class GeoJson_clear_pt_id_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_clear_pt_id_Dto, DTO)
    DTO_FIELD(String, taskid);
};

class GeoJson_task_item_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_task_item_Dto, DTO)
    DTO_FIELD(String, taskid);
    DTO_FIELD(Int32, linescount);
    DTO_FIELD(Int32, conflictscount);
};

class GeoJson_tasks_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_tasks_Dto, DTO)
    DTO_FIELD(Vector<Object<GeoJson_task_item_Dto>>, tasks);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class GeoJson_FeatureCollections_linestring_properties_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_FeatureCollections_linestring_properties_Dto, DTO)
    DTO_FIELD(Vector<Float64>, heights);
    DTO_FIELD(Float64, heightlimitdown);
    DTO_FIELD(Float64, heightlimitup);
};

class GeoJson_FeatureCollections_linstring_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_FeatureCollections_linstring_Dto, DTO)
    DTO_FIELD(String, type);
    DTO_FIELD(Object<GeoJson_FeatureCollections_linestring_properties_Dto>, properties);
    DTO_FIELD(Object<GeoJson_geometry_linestring_Dto>, geometry);
};

class GeoJson_FeatureCollections_Item_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_FeatureCollections_Item_Dto, DTO)
    DTO_FIELD(String, type);
    DTO_FIELD(Object<GeoJson_FeatureCollection_properties_Dto>, properties);
    DTO_FIELD(Vector<Object<GeoJson_FeatureCollections_linstring_Dto>>, features);
};

class GeoJson_FeatureCollections_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_FeatureCollections_Dto, DTO)
    DTO_FIELD(String, taskid);
    DTO_FIELD(Boolean, conflictbuilding);
    DTO_FIELD(Int32, resolution);
    DTO_FIELD(Vector<Object<GeoJson_FeatureCollections_Item_Dto>>, featurecollections);
    GeoJson_FeatureCollections_Dto()
    {
        taskid = "";
        conflictbuilding = true;
        resolution = 9;
    }
};

class GeoJson_linestring_opearte_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_linestring_opearte_Dto, DTO)
    DTO_FIELD(Boolean, issettype);
    DTO_FIELD(String, paramname);
    DTO_FIELD(Boolean, paramval);
    DTO_FIELD(Int32, paramval2);
    DTO_FIELD(Float64, paramval3);
};


class GeoJson_LATLNG_TO_HEX_Dto : public oatpp::DTO {

    DTO_INIT(GeoJson_LATLNG_TO_HEX_Dto, DTO)
    DTO_FIELD(Float64, LAT);
    DTO_FIELD(Float64, LNG);
    DTO_FIELD(String, HEX);
    DTO_FIELD(Int32, resolution);
};

///////////////////////////////////////////////////////////////////////////////////////////////////


#include OATPP_CODEGEN_END(DTO)

enum E_DATA_TYPE
{
    E_DATA_TYPE_SIM_DATA,
    E_DATA_TYPE_SIM_CTRL,
    E_DATA_TYPE_RECORD_CTRL,
    E_DATA_TYPE_SIM_REVIEW_DATA,
    E_DATA_TYPE_SIM_REVIEW_CTRL,
};
typedef std::function<bool (E_DATA_TYPE, const QJsonObject &)> http_receive_callback;


#endif /* AGENTDTO_H_ */
