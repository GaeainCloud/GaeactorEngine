#include <QApplication>
#include <iostream>
#include <QThread>

#include "gaeactorInterface.h"


void position_update_callback(const gaeactor_engine::ENTITY_INFO & data)
{
    //    std::cout<<"recv entity pos update "<<data.entity_id<<"\n";
}

void sensor_update_callback(const gaeactor_engine::SENSOR_INFO & data)
{
    //    std::cout<<"recv entity sensor update "<<data.entity_id<<" polygon pt size "<<data.polygon_lnglat_degs.size()<<"\n";
}

void event_update_callback(const std::vector<gaeactor_engine::EVENT_INFO> & data)
{
    std::cout.precision(17);
    //    std::cout<<"-------------------------recv event update "<< data.size()<<"-------------------------\n";
    for(auto item : data)
    {
        switch (item.m_event_mode ) {
        case gaeactor_engine::E_EVENT_TYPE_ADD:
        {
            std::cout<<" event type: add "/*<< " src "<<item.m_sensorid<<" dst "<<item.m_entityid<<" polygon id "<<item.m_polygon_id*/<<" ["<<item.m_entityposinfo.lng<<" , "<<item.m_entityposinfo.lat<<"]"<<"\n";
        }break;
        case gaeactor_engine::E_EVENT_TYPE_UPDATE:
        {
            //            std::cout<<" event type: update "/*<< " src "<<item.m_sensorid<<" dst "<<item.m_entityid<<" polygon id "<<item.m_polygon_id*/<<" ["<<item.m_entityposinfo.lng<<" , "<<item.m_entityposinfo.lat<<"]"<<"\n";
        }break;
        case gaeactor_engine::E_EVENT_TYPE_REMOVE:
        {
            std::cout<<" event type: remove "/*<< " src "<<item.m_sensorid<<" dst "<<item.m_entityid<<" polygon id "<<item.m_polygon_id*/<<" ["<<item.m_entityposinfo.lng<<" , "<<item.m_entityposinfo.lat<<"]"<<"\n";
        }break;
        default:
            break;
        }
    }
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    gaeactor_engine::GaeactorInferface _GaeactorInferface;

    std::function<void(const gaeactor_engine::ENTITY_INFO &)> position_update_callback_ = std::bind(&position_update_callback, std::placeholders::_1);
    std::function<void(const gaeactor_engine::SENSOR_INFO &)> sensor_update_callback_ = std::bind(&sensor_update_callback, std::placeholders::_1);
    std::function<void(const std::vector<gaeactor_engine::EVENT_INFO> &)> event_update_callback_ = std::bind(&event_update_callback, std::placeholders::_1);

    _GaeactorInferface.set_position_update_callback(position_update_callback_);
    _GaeactorInferface.set_sensor_update_callback(sensor_update_callback_);
    _GaeactorInferface.set_event_update_callback(event_update_callback_);

    gaeactor_engine::ENTITY_INFO entityinfo_a;
    gaeactor_engine::E_ENTITY_PROPERTY entity_type_a = AGENT_ENTITY_PROPERTY_NORMAL;


    /////////////////////////////////////////////////////////////////////////////////////////////
    entityinfo_a.entity_id = 1;
    entityinfo_a.lng = 112.77005648593638;
    entityinfo_a.lat = 21.70197542818204;
    _GaeactorInferface.updateEntityInfo(entityinfo_a, entity_type_a);

    /////////////////////////////////////////////////////////////////////////////////////////////
    gaeactor_engine::ENTITY_INFO entityinfo_b;
    gaeactor_engine::E_ENTITY_PROPERTY entity_type_b = AGENT_ENTITY_PROPERTY_SENSOR;

    gaeactor_engine::SENSOR_INFO sensorinfo_b;
    entityinfo_b.entity_id = 2;
    entityinfo_b.lng = 112.6376803640515;
    entityinfo_b.lat = 21.7959087111101;

    sensorinfo_b.entity_id = 2;
    sensorinfo_b.polygon_info.polygon_id = 1;
    sensorinfo_b.polygon_info.polygon_usage_type = 1;
    sensorinfo_b.polygon_lnglat_degs.push_back(gaeactor_engine::LNG_LAT_HGT_REF{112.71603688713071,
                                                                                21.936443021932973,
                                                                                0,
                                                                                0});
    sensorinfo_b.polygon_lnglat_degs.push_back(gaeactor_engine::LNG_LAT_HGT_REF{112.38271652196522,
                                                                                21.74069434775616,
                                                                                0,
                                                                                0});
    sensorinfo_b.polygon_lnglat_degs.push_back(gaeactor_engine::LNG_LAT_HGT_REF{112.66192647127161,
                                                                                21.48428459363528,
                                                                                0,
                                                                                0});
    sensorinfo_b.polygon_lnglat_degs.push_back(gaeactor_engine::LNG_LAT_HGT_REF{112.99731492219706,
                                                                                21.62204358112291,
                                                                                0,
                                                                                0});
    sensorinfo_b.polygon_lnglat_degs.push_back(gaeactor_engine::LNG_LAT_HGT_REF{112.71603688713071,
                                                                                21.936443021932973,
                                                                                0,
                                                                                0});




    _GaeactorInferface.updateEntityInfo(entityinfo_b, entity_type_b);

    _GaeactorInferface.updateSensorInfo(sensorinfo_b);

    _GaeactorInferface.deal_step_refresh_event();

    entityinfo_a.entity_id = 1;
    entityinfo_a.lng = 112.77005648593638;
    entityinfo_a.lat = 21.75197542818204;

    _GaeactorInferface.updateEntityInfo(entityinfo_a, entity_type_a);

    for(int i = 0; i < 500 ;i++)
    {
        entityinfo_a.lat = entityinfo_a.lat+0.001;
        _GaeactorInferface.updateEntityInfo(entityinfo_a, entity_type_a);
        _GaeactorInferface.deal_step_refresh_event();
        QThread::msleep(1);
    }

    return a.exec();
}
