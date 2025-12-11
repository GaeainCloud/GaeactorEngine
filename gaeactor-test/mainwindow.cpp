#pragma execution_character_set("utf-8")
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include "src/OriginalDateTime.h"
#include "LocationHelper.h"
#include "testdata.h"
#include "ProjectionEPSG3857.h"
#include <QTimer>
#include "head_define.h"
#define ENTITY_COUNT (200000)
#define LINE_PT_NUM (3000)
//#define LINE_PT_NUM (3000)

#define USING_TRANSMIT_LOAD

#include "bspline.h"

#define BEZIER_N    3
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_pGaeactorTransmit = new gaeactortransmit::GaeactorTransmit(this);
    m_pGaeactorTransmit->initDeployType(E_DEPLOYMODE_TYPE_LOCAL_SEND);

    m_pGaeactorTransmit->setDataCallback(std::bind(&MainWindow::receive_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

    m_pGaeactorTransmit->allocShareChannel(2*2);
    m_eventstr = m_pGaeactorTransmit->getReusePublisherChannel();

    int interval = ui->lineEdit_5->text().toInt();
    m_pUpdateTimer = new QTimer(this);
    connect(m_pUpdateTimer, &QTimer::timeout, this, &MainWindow::timeout_slot);
    m_pUpdateTimer->start(interval);
    //    m_pUpdateTimer->start(400);

    //    on_pushButton_2_clicked();
    on_pushButton_8_clicked();
    //    on_pushButton_12_clicked();
    //    on_pushButton_9_clicked();
    stdutils::OriDateTime::sleep(100);
    on_pushButton_10_clicked();
    //    on_pushButton_clicked();

    //    on_pushButton_clicked();
    //    //    on_pushButton_9_clicked();
    //    stdutils::OriDateTime::sleep(100);
    //    on_pushButton_2_clicked();

}

MainWindow::~MainWindow()
{
#ifndef USING_TRANSMIT_LOAD
    if(nullptr != m_pCurrentDealBuffer)
    {
        delete []m_pCurrentDealBuffer;
        m_pCurrentDealBuffer = nullptr;
    }
#endif
}


void MainWindow::receive_callback(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE *pdata, const UINT32 &ilen, const BYTE *pOrignaldata, const UINT32 &iOrignallen)
{
    static int recvcount = 0;
    if(recvcount %10000 == 0)
    {
        std::cout <<" recv count :"<<recvcount << std::endl;

        //m_pGaeactorTransmit->printMempoolInfo();
    }
    recvcount++;

    QByteArray by((const char*)pdata,ilen);
    //    UINT32 val = (*(UINT32*)(pdata));
    //    std::cout <<" service:"<<channel.m_service <<" instance:"<< channel.m_instance <<" event:"<< channel.m_event<< " data received: " << val << std::endl;
}

void MainWindow::timeout_slot()
{
    static int lineindex  = 0;

    if(m_playdata)
    {
        lineindex++;
        lineindex = lineindex%LINE_PT_NUM;
        if(lineindex == 0)
        {
            //            ui->pushButton_4->setText("开始传输实体位置");
            //            m_playdata.store(false);
        }

        int64_t cuts = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();



        uint32_t pre_num = 500;
        uint32_t c = ientitycount%pre_num;
        uint32_t times = ientitycount/pre_num;
        uint32_t j = 0;
        for(j = 0; j < times; j++)
        {
            std::tuple<std::string, CHANNEL_INFO*> publisherchannel = std::make_tuple("",m_pGaeactorTransmit->applyforShareChannel());

            size_t isize = sizeof(transdata_posatt_hexidx)*pre_num;
            void * usrpayload = m_pGaeactorTransmit->loanTransmitBuffer(std::get<1>(publisherchannel), isize);

            if(usrpayload)
            {
                E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POSATT_ARRAY;
                BYTE *pDstData = reinterpret_cast<BYTE*>(usrpayload);
                memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
                memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &isize, sizeof(UINT32));
                transdata_posatt_hexidx* ptransdata_posatt_hexidx = reinterpret_cast<transdata_posatt_hexidx*>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));
                memset(ptransdata_posatt_hexidx,0,isize);
                int index  = 0;
                for(int i = j*pre_num ;i < (j+1)*pre_num;i++)
                {
                    auto & item = m_entitylineposs.at(i);
                    const QVector<transdata_posatt_hexidx> &_trackingpts  = std::get<4>(item);
                    memcpy(ptransdata_posatt_hexidx+index, &_trackingpts.at(lineindex),sizeof(transdata_posatt_hexidx));
                    index++;
                }

                std::cout<<"send size"<< sizeof(transdata_posatt_hexidx)*pre_num<< " kb "<< (double)(sizeof(transdata_posatt_hexidx)*pre_num)/1024.0f<<std::endl;
                m_pGaeactorTransmit->publish(std::get<1>(publisherchannel));
            }
        }

        if(c>0)
        {
            std::tuple<std::string, CHANNEL_INFO*> publisherchannel = std::make_tuple("",m_pGaeactorTransmit->applyforShareChannel());
            size_t isize = sizeof(transdata_posatt_hexidx)*c;
            void * usrpayload = m_pGaeactorTransmit->loanTransmitBuffer(std::get<1>(publisherchannel), isize);
            if(usrpayload)
            {
                E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POSATT_ARRAY;
                BYTE *pDstData = reinterpret_cast<BYTE*>(usrpayload);
                memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
                memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &isize, sizeof(UINT32));
                transdata_posatt_hexidx* ptransdata_posatt_hexidx = reinterpret_cast<transdata_posatt_hexidx*>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));
                memset(ptransdata_posatt_hexidx,0,isize);
                int index  = 0;
                for(int i = j*pre_num ;i < ientitycount;i++)
                {
                    auto & item = m_entitylineposs.at(i);
                    const QVector<transdata_posatt_hexidx> &_trackingpts  = std::get<4>(item);
                    memcpy(ptransdata_posatt_hexidx+index, &_trackingpts.at(lineindex),sizeof(transdata_posatt_hexidx));
                    index++;
                }

                std::cout<<"send size"<< sizeof(transdata_posatt_hexidx)*pre_num<< " kb "<< (double)(sizeof(transdata_posatt_hexidx)*pre_num)/1024.0f<<std::endl;
                m_pGaeactorTransmit->publish(std::get<1>(publisherchannel));
            }
        }
        ui->label->setText("line index:"+QString::number(lineindex));
    }
}


void MainWindow::on_pushButton_5_clicked()
{
    for(int i = 0;i < ientitycount;i++)
    {
        auto & item = m_entitylineposs.at(i);
        const std::tuple<std::string, CHANNEL_INFO*> &_eventstr = std::get<0>(item);
        const TYPE_ULID &_ulid = std::get<1>(item);
        clearentityHexidex(std::get<1>(_eventstr), _ulid);
    }
}



void MainWindow::on_pushButton_clicked()
{
    m_entitylineposs.clear();
    QVector<transdata_posatt_hexidx> _trackingpts;
    _trackingpts.resize(LINE_PT_NUM);


    QGV::GeoPos topleft =targetArea().topLeft();
    topleft.setLat(topleft.latitude()+0.3);
    topleft.setLon(topleft.longitude()-0.8);
    QGV::GeoPos bottomright = targetArea().bottomRight();
    bottomright.setLat(bottomright.latitude()-0.1);
    bottomright.setLon(bottomright.longitude()+0.8);
    ientitycount = ui->lineEdit_2->text().toInt();
    ientitycount = ientitycount>ENTITY_COUNT?ENTITY_COUNT:ientitycount;
    ientitycount = ientitycount<1?1:ientitycount;

    double steplatdiff = (bottomright.latitude() - topleft.latitude()) / ientitycount;
    double steplngdiff = (bottomright.longitude() - topleft.longitude()) / ientitycount;
    for(int i = 0 ;i < ientitycount;i++)
    {

        UInt64 id = 0;
        do
        {
            id = m_snowflake.GetId();
        }
        while(m_existid.contains(id));
        m_existid.insert(id,true);
        TYPE_ULID _ulid = id;
        QGV::GeoPos begin;
        QGV::GeoPos end;

        if(i%2 == 0)
        {
            begin.setLon(topleft.longitude());
            end.setLon(bottomright.longitude());
        }
        else
        {
            begin.setLon(bottomright.longitude());
            end.setLon(topleft.longitude());
        }

        begin.setLat(topleft.latitude()+steplatdiff*i);
        end.setLat(topleft.latitude()+steplatdiff*i);


        double steplat = (end.latitude() - begin.latitude()) / LINE_PT_NUM;
        double steplng = (end.longitude() - begin.longitude()) / LINE_PT_NUM;
        LAT_LNG lstpos;
        for(int j = 0; j < LINE_PT_NUM; j++)
        {
            QGV::GeoPos tmp;
            tmp.setLat(begin.latitude() + steplat*j);
            tmp.setLon(begin.longitude() + steplng*j);
            auto wsgeo = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(tmp.latitude(), tmp.longitude());
            transdata_posatt_hexidx& pos_hexidx_data = _trackingpts[j];
            memset(&pos_hexidx_data, 0, sizeof(transdata_posatt_hexidx));
            pos_hexidx_data.PARAM_protocol_head.PARAM_source_ulid = _ulid;
            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_longitude = wsgeo.lng*LON_LAT_ACCURACY;
            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_latitude = wsgeo.lat*LON_LAT_ACCURACY;
            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_roll = 0.0f;
            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pitch = 0.0f;
            if(j == 0)
            {
                pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw = 0;
            }
            else
            {
                float yaw = std::get<1>(projectionmercator::ProjectionEPSG3857::calculateBraring(lstpos, wsgeo));
                pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw =yaw;
            }
            lstpos = wsgeo;
            PROPERTY_SET_TYPE(pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_sensor_property, AGENT_ENTITY_PROPERTY_NORMAL);
            LocationHelper::getIndexInfo(pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pos_hexidx, wsgeo.lat,wsgeo.lng,INDEX_MAPPING_RESOLUTION_ENTITY_POS);

        }
        m_entitylineposs.push_back(std::make_tuple(m_eventstr,_ulid, begin,end,_trackingpts));
        //        stdutils::OriDateTime::sleep(1);
    }
}
#define M_PI 3.14159265358979323846
#define ROW_NUM (5)
void MainWindow::on_pushButton_2_clicked()
{
    QVector<QPointF> _verts;
    for(int i = 0; i < 361; i++)
    {
        int angle = (i+180)%361;
        _verts.push_back(QPointF(sin(angle * M_PI/ 180.0f),cos(angle * M_PI/ 180.0f)));
    }

    m_sensor.clear();
    int sensorcount = ui->lineEdit->text().toInt();

    QGV::GeoPos topleft =targetArea().topLeft();
    QGV::GeoPos bottomright = targetArea().bottomRight();

    double steplatdiff = (bottomright.latitude() - topleft.latitude()) / ROW_NUM;
    double steplngdiff = (bottomright.longitude() - topleft.longitude()) / ROW_NUM;

    int radius_ = ui->lineEdit_4->text().toInt();
    QVector<QGV::GeoPos> _trackingpts;
    _trackingpts.resize(360);
    double radius = radius_*1000;
    for(int i = 0 ;i < ROW_NUM;i++)
    {

        QGV::GeoPos begin;
        QGV::GeoPos end;

        if(i%2 == 0)
        {
            begin.setLon(topleft.longitude());
            end.setLon(bottomright.longitude());
        }
        else
        {
            begin.setLon(bottomright.longitude());
            end.setLon(topleft.longitude());
        }

        begin.setLat(topleft.latitude()+steplatdiff*i);
        end.setLat(topleft.latitude()+steplatdiff*i);

        double steplat = (end.latitude() - begin.latitude()) / (sensorcount/ROW_NUM);
        double steplng = (end.longitude() - begin.longitude()) / (sensorcount/ROW_NUM);
        for(int j = 0; j < (sensorcount/ROW_NUM); j++)
        {
            UInt64 id = 0;
            do
            {
                id = m_snowflake.GetId();
            }
            while(m_existid.contains(id));
            m_existid.insert(id,true);
            TYPE_ULID _ulid = id;

            QGV::GeoPos _center;
            _center.setLat(begin.latitude() + steplat*j);
            _center.setLon(begin.longitude() + steplng*j);

            QPointF _centerpt = projectionmercator::ProjectionEPSG3857::geoToProj(LAT_LNG{_center.latitude(), _center.longitude()});
            QPolygonF points;
            for (auto item:_verts)
            {
                points.push_back(QPointF(item.x()*radius+_centerpt.x(), item.y()*radius+_centerpt.y()));
            }

            _trackingpts.clear();

            for(auto item:points)
            {
                LAT_LNG pt = projectionmercator::ProjectionEPSG3857::projToGeo(item);
                _trackingpts.push_back(QGV::GeoPos(pt.lat, pt.lng));
            }
            stdutils::OriDateTime::sleep(1);

            m_sensor.push_back(std::make_tuple(m_eventstr, _ulid, _center,_trackingpts));
        }

    }
}

void MainWindow::on_pushButton_3_clicked()
{
    for(auto item :m_sensor)
    {
        std::tuple<std::string, CHANNEL_INFO*> &_eventstr = std::get<0>(item);
        TYPE_ULID &_ulid = std::get<1>(item);
        QGV::GeoPos &_center = std::get<2>(item);
        QVector<QGV::GeoPos> &_trackingpts  = std::get<3>(item);
        clearHexidex(std::get<1>(_eventstr), _ulid);
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    if(!m_playdata.load())
    {
        ui->pushButton_4->setText("停止传输实体位置");
    }
    else
    {
        ui->pushButton_4->setText("开始传输实体位置");
    }
    m_playdata.store(!m_playdata.load());
}

void MainWindow::on_pushButton_7_clicked()
{
    for(auto item :m_sensor)
    {
        std::tuple<std::string, CHANNEL_INFO*> &_eventstr = std::get<0>(item);
        TYPE_ULID &_ulid = std::get<1>(item);
        QGV::GeoPos &_center = std::get<2>(item);
        QVector<QGV::GeoPos> &_trackingpts  = std::get<3>(item);


        auto wsgeopostmp = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(_center.latitude(), _center.longitude());
        LAT_LNG pos{wsgeopostmp.lat, wsgeopostmp.lng};
        std::vector<LatLng> data;
        std::vector<transdata_param_seq_polygon> _polygon;
        for(auto item:_trackingpts)
        {
            auto wsgeo = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(item.latitude(), item.longitude());

            transdata_param_seq_polygon _polygon_pt{(INT32)(wsgeo.lng * 10000000), (INT32)(wsgeo.lat * 10000000), 0};
            _polygon.emplace_back(std::move(_polygon_pt));
            LatLng latlng{LocationHelper::degreeToRadian(wsgeo.lat),LocationHelper::degreeToRadian(wsgeo.lng)};
            data.emplace_back(std::move(latlng));
        }

        HEXIDX_ARRAY indexlist;
        LocationHelper::getPolygonIndex(indexlist, data);

        dealHexidex(std::get<1>(_eventstr), _ulid, pos, indexlist, _polygon, 0);
    }
}

void MainWindow::dealentityHexidex(const CHANNEL_INFO *channelinfo, TYPE_ULID _ulid, const LAT_LNG &pos, INT32 alt, FLOAT32 roll, FLOAT32 pitch, FLOAT32 yaw,bool bSensor, bool bClear)
{
    size_t isize = sizeof(transdata_posatt_hexidx);
    void * usrpayload = m_pGaeactorTransmit->loanTransmitBuffer(channelinfo, isize);

    if(usrpayload)
    {
        E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POSATT;
        BYTE *pDstData = reinterpret_cast<BYTE*>(usrpayload);
        memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
        memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &isize, sizeof(UINT32));
        transdata_posatt_hexidx *pData = reinterpret_cast<transdata_posatt_hexidx *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));
        memset(pData,0,isize);

        pData->PARAM_protocol_head.PARAM_source_ulid = _ulid;
        H3INDEX h3index = 0;
        if(!bClear)
        {
            LocationHelper::getIndexInfo(h3index, pos.lat,pos.lng,INDEX_MAPPING_RESOLUTION_ENTITY_POS);
        }
        PROPERTY_SET_TYPE(pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_sensor_property,bSensor ? AGENT_ENTITY_PROPERTY_SENSOR : AGENT_ENTITY_PROPERTY_NORMAL);
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pos_hexidx = h3index;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_latitude = pos.lat*LON_LAT_ACCURACY;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_longitude = pos.lng*LON_LAT_ACCURACY;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_amsl = alt*1000;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_ref = 0;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_roll = roll;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pitch = pitch;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw = yaw;
        m_pGaeactorTransmit->publish(channelinfo);
    }
}


void MainWindow::clearentityHexidex(const CHANNEL_INFO *channelinfo,TYPE_ULID _ulid)
{
    dealentityHexidex(channelinfo, _ulid,LAT_LNG(), 0, 0.0, 0.0, 0.0,false, true);
}

void MainWindow::dealHexidex(const CHANNEL_INFO *channelinfo,TYPE_ULID _ulid, const LAT_LNG& pos, const HEXIDX_ARRAY &hexidxslist, const std::vector<transdata_param_seq_polygon>& _polygon, int slient_time_gap,bool bClear)
{
    dealentityHexidex(channelinfo, _ulid, pos, 0, 0.0, 0.0, 0.0,true, bClear);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    size_t ibufsize = (hexidxslist.empty() ? 0 : sizeof(UINT16)) + sizeof(transdata_param_seq_hexidx) * hexidxslist.size() + (_polygon.empty() ? 0:sizeof(UINT16)) + sizeof(transdata_param_seq_polygon) * _polygon.size();
    ibufsize = bClear ? 0 : ibufsize;
    size_t isize = sizeof(transdata_wave_smd_hexidx) + ibufsize;

    void * usrpayload = m_pGaeactorTransmit->loanTransmitBuffer(channelinfo, isize);

    if(usrpayload)
    {
        E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR;
        BYTE *pDstData = reinterpret_cast<BYTE*>(usrpayload);
        memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
        memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &isize, sizeof(UINT32));
        transdata_wave_smd_hexidx *pData = reinterpret_cast<transdata_wave_smd_hexidx *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));
        memset(pData,0,isize);
        pData->PARAM_protocol_head.PARAM_source_ulid = _ulid;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_snd_rcv = 0x01;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_silent_time_gap = slient_time_gap;
        pData->PARAM_payload_wave_smd_hexidx.PARAM_payload_buffer.PARAM_buffer_count = ibufsize;

        if(!bClear && ibufsize>0)
        {
            BYTE *p_byte_buffer = (BYTE*)(pData->PARAM_payload_wave_smd_hexidx.PARAM_payload_buffer.PARAM_seq_buffer);
            transdata_payload_hexidx *ptransdata_payload_hexidx = GET_HEXIDX_STRUCT_PTR(p_byte_buffer);
            ptransdata_payload_hexidx->PARAM_qty_hexidx = hexidxslist.size();
            for(int i = 0; i < hexidxslist.size();i++)
            {
                ptransdata_payload_hexidx->PARAM_seq_hexidx[i].PARAM_seq_hexidx_element = hexidxslist.at(i);
            }
            transdata_payload_polygon *ptransdata_payload_polygon = GET_POLYGON_STRUCT_PTR(p_byte_buffer);
            ptransdata_payload_polygon->PARAM_qty_polygon = _polygon.size();
            memcpy(ptransdata_payload_polygon->PARAM_seq_polygon, _polygon.data(), sizeof(transdata_param_seq_polygon) * _polygon.size());
        }
        std::cout<<"count"<<isize<<" kb "<<(double)isize/1024.0f<<std::endl;
        m_pGaeactorTransmit->publish(channelinfo);
    }
}

void MainWindow::clearHexidex(const CHANNEL_INFO *channelinfo,TYPE_ULID _ulid)
{
    std::string sensorulidstr = QString::number(_ulid).toStdString();
    std::cout<<"send clear entity and sensor hexidx: "<<sensorulidstr<<std::endl;
    /////////////////////////////////////////////////////////////////////////////////////////    
    dealentityHexidex(channelinfo, _ulid, LAT_LNG(), 0, 0.0, 0.0, 0.0,true,true);
    //////////////////////////////////////////////////////////////////////////////////////////
    /// \brief isize
    dealHexidex(channelinfo, _ulid, LAT_LNG(), HEXIDX_ARRAY(),std::vector<transdata_param_seq_polygon>(),0,true);
}




QGV::GeoPos MainWindow::randPos(const QGV::GeoRect& targetArea)
{
    const double latRange = targetArea.latTop() - targetArea.latBottom();
    const double lonRange = targetArea.lonRigth() - targetArea.lonLeft();
    static const int range = 1000;
    return { targetArea.latBottom() + latRange * ((qrand()+stdutils::OriDateTime::getCurrentUTCTimeStampMSecs()) % range) / range,
            targetArea.lonLeft() + lonRange * ((qrand()+stdutils::OriDateTime::getCurrentUTCTimeStampMSecs()) % range) / range };
}

QGV::GeoRect MainWindow::randRect(const QGV::GeoRect& targetArea, const QSizeF& size)
{
    const auto baseGeo = randPos(targetArea);
    const auto base = projectionmercator::ProjectionEPSG3857::geoToProj(LAT_LNG{baseGeo.latitude(),baseGeo.longitude()});
    projectionmercator::GeoLatLngRect rc =  projectionmercator::ProjectionEPSG3857::projToGeo({ base, base + QPointF(size.width(), size.height()) });

    return QGV::GeoRect(rc.topLeft().lat, rc.topLeft().lng,
                        rc.bottomRight().lat, rc.bottomRight().lng);
}

QGV::GeoRect MainWindow::randRect(const QGV::GeoRect& targetArea, int baseSize)
{
    const auto size = randSize(baseSize);
    return randRect(targetArea, size);
}

QSizeF MainWindow::randSize(int baseSize)
{
    const int range = -baseSize / 2;
    return QSize(baseSize + ((qrand()+stdutils::OriDateTime::getCurrentUTCTimeStampMSecs()) % range), baseSize + ((qrand()+stdutils::OriDateTime::getCurrentUTCTimeStampMSecs()) % range));
}
QGV::GeoRect MainWindow::targetArea() const
{
    if(ui->radioButton->isChecked())
    {
        return QGV::GeoRect(QGV::GeoPos(36.169599088628914,94.3073663203682), QGV::GeoPos(24.93687000788293,116.05543366634862));
    }
    else
    {
        return QGV::GeoRect(QGV::GeoPos(31.525029614929977,103.09173869852884), QGV::GeoPos(29.544029022607717,106.62621662988863));
    }
}


QGV::GeoRect MainWindow::targetArea2() const
{
    return QGV::GeoRect(QGV::GeoPos(26.756870922727188,104.67109994368235), QGV::GeoPos(25.268406080228914,106.96667439385021));
    //return QGV::GeoRect(QGV::GeoPos(26.06822947982789, 119.60365706952996), QGV::GeoPos(24.41884246847917,122.74837258794958));
}

void MainWindow::on_pushButton_6_clicked()
{
    m_entitylineposs.clear();
    QVector<transdata_posatt_hexidx> _trackingpts;

    ientitycount = ui->lineEdit_2->text().toInt();
    ientitycount = ientitycount>ENTITY_COUNT?ENTITY_COUNT:ientitycount;
    ientitycount = ientitycount<1?1:ientitycount;

    _trackingpts.resize(LINE_PT_NUM);
    for(int i = 0 ;i < ientitycount;i++)
    {
        UInt64 id = 0;
        do
        {
            id = m_snowflake.GetId();
        }
        while(m_existid.contains(id));
        m_existid.insert(id,true);
        TYPE_ULID _ulid = id;

        QGV::GeoPos begin = randPos(targetArea());
        QGV::GeoPos end = randPos(targetArea());

        double steplat = (end.latitude() - begin.latitude()) / LINE_PT_NUM;
        double steplng = (end.longitude() - begin.longitude()) / LINE_PT_NUM;
        LAT_LNG lstpos;
        for(int j = 0; j < LINE_PT_NUM; j++)
        {
            QGV::GeoPos tmp;
            tmp.setLat(begin.latitude() + steplat*j);
            tmp.setLon(begin.longitude() + steplng*j);
            auto wsgeo = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(tmp.latitude(), tmp.longitude());
            transdata_posatt_hexidx& pos_hexidx_data = _trackingpts[j];
            memset(&pos_hexidx_data, 0, sizeof(transdata_posatt_hexidx));
            pos_hexidx_data.PARAM_protocol_head.PARAM_source_ulid = _ulid;
            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_longitude = wsgeo.lng*LON_LAT_ACCURACY;
            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_latitude = wsgeo.lat*LON_LAT_ACCURACY;
            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_roll = 0.0f;
            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pitch = 0.0f;
            if(j == 0)
            {
                pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw = 0;
            }
            else
            {
                float yaw = std::get<1>(projectionmercator::ProjectionEPSG3857::calculateBraring(lstpos, wsgeo));
                pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw =yaw;
            }
            lstpos = wsgeo;
            PROPERTY_SET_TYPE(pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_sensor_property, AGENT_ENTITY_PROPERTY_NORMAL);
            LocationHelper::getIndexInfo(pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pos_hexidx, wsgeo.lat,wsgeo.lng,INDEX_MAPPING_RESOLUTION_ENTITY_POS);

        }
        m_entitylineposs.push_back(std::make_tuple(m_eventstr,_ulid, begin,end,_trackingpts));
    }
}


void MainWindow::on_pushButton_8_clicked()
{
    QVector<QPointF> _verts;
    for(int i = 0; i < 361; i++)
    {
        int angle = (i+180)%361;
        _verts.push_back(QPointF(sin(angle * M_PI/ 180.0f),cos(angle * M_PI/ 180.0f)));
    }

    m_sensor.clear();
    int sensorcount = ui->lineEdit->text().toInt();
    int radius_ = ui->lineEdit_4->text().toInt();
    QVector<QGV::GeoPos> _trackingpts;
    _trackingpts.resize(360);
    double radius = radius_*1000;
    for(int i = 0 ;i < sensorcount;i++)
    {
        UInt64 id = 0;
        do
        {
            id = m_snowflake.GetId();
        }
        while(m_existid.contains(id));
        m_existid.insert(id,true);
        TYPE_ULID _ulid = id;

        QGV::GeoPos _center = randPos(targetArea());

        QPointF _centerpt = projectionmercator::ProjectionEPSG3857::geoToProj(LAT_LNG{_center.latitude(), _center.longitude()});
        QPolygonF points;
        for (auto item:_verts)
        {
            points.push_back(QPointF(item.x()*radius+_centerpt.x(), item.y()*radius+_centerpt.y()));
        }


        _trackingpts.clear();

        for(auto item:points)
        {
            LAT_LNG pt = projectionmercator::ProjectionEPSG3857::projToGeo(item);
            _trackingpts.push_back(QGV::GeoPos(pt.lat, pt.lng));
        }
        m_sensor.push_back(std::make_tuple(m_eventstr, _ulid, _center,_trackingpts));
    }
}


void MainWindow::on_pushButton_9_clicked()
{
    m_entitylineposs.clear();
    QVector<transdata_posatt_hexidx> _trackingpts;
    _trackingpts.resize(LINE_PT_NUM);


    ientitycount = ui->lineEdit_2->text().toInt();
    ientitycount = ientitycount>ENTITY_COUNT?ENTITY_COUNT:ientitycount;
    ientitycount = ientitycount<1?1:ientitycount;


    QGV::GeoPos topleft = targetArea().topLeft();
    topleft.setLat(topleft.latitude()+0.3);
    topleft.setLon(topleft.longitude()-0.8);
    QGV::GeoPos bottomright = targetArea().bottomRight();
    bottomright.setLat(bottomright.latitude()-0.1);
    bottomright.setLon(bottomright.longitude()+0.8);



    double steplatdiff = (bottomright.latitude() - topleft.latitude()) / ientitycount;



    QGV::GeoPos topleft2 = targetArea2().topLeft();
    topleft2.setLat(topleft2.latitude()+0.3);
    topleft2.setLon(topleft2.longitude()-0.8);
    QGV::GeoPos bottomright2 = targetArea2().bottomRight();
    bottomright2.setLat(bottomright2.latitude()-0.1);
    bottomright2.setLon(bottomright2.longitude()+0.8);

    double steplatdiff2 = (bottomright.latitude() - topleft.latitude()) / ientitycount;

    int num  = ui->lineEdit_3->text().toInt();
    int cc = ientitycount/100;


    const int range = ientitycount;
    QList<int> randlist;
    while(randlist.size() != num)
    {
        int rr = (qrand()+stdutils::OriDateTime::getCurrentUTCTimeStampMSecs()) % range;
        if(!randlist.contains(rr))
        {
            randlist.push_back(rr);
        }
    }
    for(int i = 0 ;i < ientitycount;i++)
    {
        UInt64 id = 0;
        do
        {
            id = m_snowflake.GetId();
        }
        while(m_existid.contains(id));
        m_existid.insert(id,true);
        TYPE_ULID _ulid = id;

        QGV::GeoPos begin;
        QGV::GeoPos end;
        if(randlist.contains(i))
        {

            if(i%2 == 0)
            {
                begin.setLon(topleft.longitude());
                end.setLon(bottomright.longitude());
            }
            else
            {
                begin.setLon(bottomright.longitude());
                end.setLon(topleft.longitude());
            }

            begin.setLat(topleft.latitude()+steplatdiff*i);
            end.setLat(topleft.latitude()+steplatdiff*i);


            double steplat = (end.latitude() - begin.latitude()) / LINE_PT_NUM;
            double steplng = (end.longitude() - begin.longitude()) / LINE_PT_NUM;
            for(int j = 0; j < LINE_PT_NUM; j++)
            {
                QGV::GeoPos tmp;
                tmp.setLat(begin.latitude() + steplat*j);
                tmp.setLon(begin.longitude() + steplng*j);

                transdata_posatt_hexidx& pos_hexidx_data = _trackingpts[j];
                memset(&pos_hexidx_data, 0, sizeof(transdata_posatt_hexidx));
                pos_hexidx_data.PARAM_protocol_head.PARAM_source_ulid = _ulid;
            PROPERTY_SET_TYPE(pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_sensor_property, AGENT_ENTITY_PROPERTY_NORMAL);
                LocationHelper::getIndexInfo(pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pos_hexidx, tmp.latitude(),tmp.longitude(),INDEX_MAPPING_RESOLUTION_ENTITY_POS);

            }
        }
        else
        {

            if(i%2 == 0)
            {
                begin.setLon(topleft2.longitude());
                end.setLon(bottomright2.longitude());
            }
            else
            {
                begin.setLon(bottomright2.longitude());
                end.setLon(topleft2.longitude());
            }

            begin.setLat(topleft2.latitude()+steplatdiff2*i);
            end.setLat(topleft2.latitude()+steplatdiff2*i);


            double steplat = (end.latitude() - begin.latitude()) / LINE_PT_NUM;
            double steplng = (end.longitude() - begin.longitude()) / LINE_PT_NUM;
            LAT_LNG lstpos;
            for(int j = 0; j < LINE_PT_NUM; j++)
            {
                QGV::GeoPos tmp;
                tmp.setLat(begin.latitude() + steplat*j);
                tmp.setLon(begin.longitude() + steplng*j);
                auto wsgeo = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(tmp.latitude(), tmp.longitude());
                transdata_posatt_hexidx& pos_hexidx_data = _trackingpts[j];
                memset(&pos_hexidx_data, 0, sizeof(transdata_posatt_hexidx));
                pos_hexidx_data.PARAM_protocol_head.PARAM_source_ulid = _ulid;
                pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_longitude = wsgeo.lng*LON_LAT_ACCURACY;
                pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_latitude = wsgeo.lat*LON_LAT_ACCURACY;
                pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_roll = 0.0f;
                pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pitch = 0.0f;
                if(j == 0)
                {
                    pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw = 0;
                }
                else
                {
                    float yaw = std::get<1>(projectionmercator::ProjectionEPSG3857::calculateBraring(lstpos, wsgeo));
                    pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw =yaw;
                }
                lstpos = wsgeo;
                PROPERTY_SET_TYPE(pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_sensor_property, AGENT_ENTITY_PROPERTY_NORMAL);
                LocationHelper::getIndexInfo(pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pos_hexidx, wsgeo.lat,wsgeo.lng,INDEX_MAPPING_RESOLUTION_ENTITY_POS);

            }
        }

        m_entitylineposs.push_back(std::make_tuple(m_eventstr,_ulid, begin,end,_trackingpts));
        //        stdutils::OriDateTime::sleep(1);
    }
}



void MainWindow::on_pushButton_10_clicked()
{
    auto generatebezier = [&]()->std::vector<QGV::GeoPos>{
        std::vector<tagPoint> _pts;
        std::vector<QGV::GeoPos> ret_pts;
        for(int index = 0; index < BEZIER_N; index++)
        {
            QGV::GeoPos begin = randPos(targetArea());
            _pts.push_back(tagPoint(begin.latitude(),begin.longitude(),0));
        }

        const float step = 1.0f/LINE_PT_NUM; // 步长
        for (float t = 0; t <= 1; t += step) {
            tagPoint p = bezier_curve(_pts, t);
            ret_pts.push_back(QGV::GeoPos(p.x, p.y));
        }

        return ret_pts;
    };

    m_entitylineposs.clear();
    QVector<transdata_posatt_hexidx> _trackingpts;

    ientitycount = ui->lineEdit_2->text().toInt();
    ientitycount = ientitycount>ENTITY_COUNT?ENTITY_COUNT:ientitycount;
    ientitycount = ientitycount<1?1:ientitycount;

    _trackingpts.resize(LINE_PT_NUM);
    for(int i = 0 ;i < ientitycount;i++)
    {
        UInt64 id = 0;
        do
        {
            id = m_snowflake.GetId();
        }
        while(m_existid.contains(id));
        m_existid.insert(id,true);
        TYPE_ULID _ulid = id;

        auto bezier_result = generatebezier();
        LAT_LNG lstpos;
        for(int j = 0; j < LINE_PT_NUM && j < bezier_result.size(); j++)
        {
            QGV::GeoPos tmp = bezier_result.at(j);
            auto wsgeo = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(tmp.latitude(), tmp.longitude());
            transdata_posatt_hexidx& pos_hexidx_data = _trackingpts[j];
            memset(&pos_hexidx_data, 0, sizeof(transdata_posatt_hexidx));
            pos_hexidx_data.PARAM_protocol_head.PARAM_source_ulid = _ulid;
            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_longitude = wsgeo.lng*LON_LAT_ACCURACY;
            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_latitude = wsgeo.lat*LON_LAT_ACCURACY;
            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_roll = 0.0f;
            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pitch = 0.0f;
            if(j == 0)
            {
                pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw = 0;
            }
            else
            {
                float yaw = std::get<1>(projectionmercator::ProjectionEPSG3857::calculateBraring(lstpos, wsgeo));
                pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw =yaw;
            }
            lstpos = wsgeo;
            PROPERTY_SET_TYPE(pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_sensor_property, AGENT_ENTITY_PROPERTY_NORMAL);
            LocationHelper::getIndexInfo(pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pos_hexidx, wsgeo.lat,wsgeo.lng,INDEX_MAPPING_RESOLUTION_ENTITY_POS);

        }
        m_entitylineposs.push_back(std::make_tuple(m_eventstr,_ulid, bezier_result.at(0),bezier_result.at(LINE_PT_NUM-1),_trackingpts));
    }
}



void MainWindow::on_pushButton_11_clicked()
{
    auto generatebspline = [&]()->std::vector<QGV::GeoPos>{
        std::vector<Point> _pts;
        std::vector<QGV::GeoPos> ret_pts;
        //        for(int index = 0; index < BEZIER_N; index++)
        //        {
        //            QGV::GeoPos begin = randPos(targetArea());
        //            _pts.push_back(Point{begin.lat,begin.lng});
        //        }

        //        for(int index = 0; index < BEZIER_N; index++)
        //        {
        //            QGV::GeoPos begin = randPos(targetArea());
        //            _pts.push_back(Point{begin.lat,begin.lng});
        //            ,


            //                ,


            //                ,

            //        }

            std::cout.precision(17);
        Point pt;
        pt.y = 113.31157599235331;
        pt.x = 23.391391318204782;
        _pts.push_back(pt);

        pt.y = 113.31146130104395;
        pt.x = 23.391032458664753;
        _pts.push_back(pt);

        pt.y = 113.31119542573543;
        pt.x = 23.391080306659845;
        _pts.push_back(pt);


        pt.y = 113.30807790740721;
        pt.x = 23.391821948370733;
        _pts.push_back(pt);

        pt.y = 113.30771298051326;
        pt.x = 23.39192721330899;
        _pts.push_back(pt);

        pt.y = 113.30779117913283;
        pt.x = 23.392257361891595;
        _pts.push_back(pt);

        pt.y = 113.30835839540003;
        pt.x = 23.39444980704809;
        _pts.push_back(pt);

        pt.y = 113.30849393967526;
        pt.x = 23.394832580646735;
        _pts.push_back(pt);

        pt.y = 113.30751385030078;
        pt.x = 23.39506224427585;
        _pts.push_back(pt);




        const float step = 1.0f/LINE_PT_NUM; // 步长
        Bspline test(3, quniform, _pts, step);

        std::vector<Point> resulet = test.creatBspline();

        for (auto it = resulet.begin(); it != resulet.end(); it++)
        {
            ret_pts.push_back(QGV::GeoPos(it->x, it->y));
            std::cout<<"["<<it->y<<","<<it->x<<"],"<<std::endl;
        }
        return ret_pts;
    };

    generatebspline();
    //    m_entitylineposs.clear();
    //    QVector<transdata_posatt_hexidx> _trackingpts;

    //    ientitycount = ui->lineEdit_2->text().toInt();
    //    ientitycount = ientitycount>ENTITY_COUNT?ENTITY_COUNT:ientitycount;
    //    ientitycount = ientitycount<1?1:ientitycount;

    //    _trackingpts.resize(LINE_PT_NUM);
    //    for(int i = 0 ;i < ientitycount;i++)
    //    {
    //        UInt64 id = 0;
    //        do
    //        {
    //            id = m_snowflake.GetId();
    //        }
    //        while(m_existid.contains(id));
    //        m_existid.insert(id,true);
    //        TYPE_ULID _ulid = id;

    //        auto bezier_result = generatebspline();
    //        LAT_LNG lstpos;
    //        for(int j = 0; j < LINE_PT_NUM && j < bezier_result.size(); j++)
    //        {
    //            QGV::GeoPos tmp = bezier_result.at(j);
    //            auto wsgeo = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(tmp.lat, tmp.lng);
    //            transdata_posatt_hexidx& pos_hexidx_data = _trackingpts[j];
    //            memset(&pos_hexidx_data, 0, sizeof(transdata_posatt_hexidx));
    //            pos_hexidx_data.PARAM_protocol_head.PARAM_source_ulid = _ulid;
    //            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_longitude = wsgeo.lng*LON_LAT_ACCURACY;
    //            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_latitude = wsgeo.lat*LON_LAT_ACCURACY;
    //            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_roll = 0.0f;
    //            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pitch = 0.0f;
    //            if(j == 0)
    //            {
    //                pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw = 0;
    //            }
    //            else
    //            {
    //                float yaw = std::get<1>(projectionmercator::ProjectionEPSG3857::calculateBraring(lstpos, wsgeo));
    //                pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw =yaw;
    //            }
    //            lstpos = wsgeo;
    //            PROPERTY_SET_TYPE(pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_sensor_property, AGENT_ENTITY_PROPERTY_NORMAL);
    //            pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pos_hexidx = LocationHelper::getIndexInfo(wsgeo.lat,wsgeo.lng,INDEX_MAPPING_RESOLUTION_ENTITY_POS);

    //        }
    //        m_entitylineposs.push_back(std::make_tuple(m_eventstr,_ulid, bezier_result.at(0),bezier_result.at(LINE_PT_NUM-1),_trackingpts));
    //    }
}


void MainWindow::on_pushButton_12_clicked()
{
    QVector<QPointF> _verts;
    for(int i = 0; i < 361; i++)
    {
        int angle = (i+180)%361;
        _verts.push_back(QPointF(sin(angle * M_PI/ 180.0f),cos(angle * M_PI/ 180.0f)));
    }

    m_sensor.clear();
    int sensorcount = ui->lineEdit->text().toInt();
    int radius_ = ui->lineEdit_4->text().toInt();
    QVector<QGV::GeoPos> _trackingpts;
    _trackingpts.resize(360);
    double radius = radius_*1000;
    srand(time(0));
    int range_min = 40;
    int range = 40;

    for(int i = 0 ;i < sensorcount;i++)
    {
        double radius = ((rand()%range)+range_min)*1000;
        UInt64 id = 0;
        do
        {
            id = m_snowflake.GetId();
        }
        while(m_existid.contains(id));
        m_existid.insert(id,true);
        TYPE_ULID _ulid = id;

        QGV::GeoPos _center = randPos(targetArea());

        QPointF _centerpt = projectionmercator::ProjectionEPSG3857::geoToProj(LAT_LNG{_center.latitude(), _center.longitude()});
        QPolygonF points;
        for (auto item:_verts)
        {
            points.push_back(QPointF(item.x()*radius+_centerpt.x(), item.y()*radius+_centerpt.y()));
        }


        _trackingpts.clear();

        for(auto item:points)
        {
            LAT_LNG pt = projectionmercator::ProjectionEPSG3857::projToGeo(item);
            _trackingpts.push_back(QGV::GeoPos(pt.lat, pt.lng));
        }
        m_sensor.push_back(std::make_tuple(m_eventstr, _ulid, _center,_trackingpts));
    }
}


void MainWindow::on_radioButton_clicked()
{
    if(ui->radioButton->isChecked())
    {
        ui->radioButton_2->setChecked(false);
    }
    else
    {
        ui->radioButton_2->setChecked(true);
    }

}


void MainWindow::on_radioButton_2_clicked()
{
    if(ui->radioButton_2->isChecked())
    {
        ui->radioButton->setChecked(false);
    }
    else
    {
        ui->radioButton->setChecked(true);
    }

}


void MainWindow::on_lineEdit_5_editingFinished()
{
    int interval = ui->lineEdit_5->text().toInt();
    if(m_pUpdateTimer->isActive())
    {
        m_pUpdateTimer->stop();
    }
    m_pUpdateTimer->start(interval);
}

