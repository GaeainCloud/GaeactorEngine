#include "listviewitem.h"
#include <QSize>
#include <QPainter>
#include <QMutexLocker>
#include <QEvent>
#include <QMouseEvent>


DataSrcListViewModel::DataSrcListViewModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

DataSrcListViewModel::~DataSrcListViewModel()
{
}

int DataSrcListViewModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_dataSourceList.size();
}

int DataSrcListViewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

const LISTVIEW_ITEM_TYPE &DataSrcListViewModel::getItem(int indexrow) const
{
    return m_dataSourceList.at(m_dataSourceList.size() - indexrow-1);
}

QVariant DataSrcListViewModel::data(const QModelIndex &index, int role) const
{
    const LISTVIEW_ITEM_TYPE& iteminfo = m_dataSourceList.at(m_dataSourceList.size() - index.row()-1);

    switch (role) {
    case Qt::SizeHintRole:
        {
                return QSize(0, 128);
        }
    case Qt::BackgroundRole:
        {
            const bool &bIdentifi = std::get<0>(iteminfo);

            if(bIdentifi)
            {
                return QColor(0,255,0,64);
            }
            else
            {
                return QColor(128,128,128,64);
            }
        }
    default:
        break;
    }
    return QVariant();
}

//#define SHOW_EVENT_DETAIL
#define SHOW_EVENT_DETAIL_REMOVE
void DataSrcListViewModel::add(const EVENT_INFO& mapitem, const QString &sendorname, const QString &entityname, bool bIdentifi)
{
#ifdef SHOW_EVENT_DETAIL
    m_dataSourceList.push_back(std::make_tuple(bIdentifi, mapitem));
#else
    if(bIdentifi)
    {
        auto entity_ulid_listitor = std::find_if(m_dataSourceList.begin(),
                                                 m_dataSourceList.end(),
                                                 [&](const LISTVIEW_ITEM_TYPE & vl){
                                                     const EVENT_INFO &info = std::get<3>(vl);
                                                     return (info.m_sensorid == mapitem.m_sensorid) && (info.m_entityid == mapitem.m_entityid) && (info.m_sensingmediaid == mapitem.m_sensingmediaid);
                                                 });
        if(entity_ulid_listitor != m_dataSourceList.end())
        {
            bool &bIdentifiold = std::get<0>(*entity_ulid_listitor);
            EVENT_INFO &info = std::get<3>(*entity_ulid_listitor);

            LAT_LNG &latlng_sensor = std::get<4>(*entity_ulid_listitor);
            LAT_LNG &latlng_entity = std::get<5>(*entity_ulid_listitor);

            info = mapitem;
            bIdentifiold = true;

#if 0
            LAT_LNG sensorlatlng = LocationHelper::doCell(info.m_sensorposinfo.PARAM_pos_hexidx);
            LAT_LNG entitylatlng = LocationHelper::doCell(info.m_entityposinfo.PARAM_pos_hexidx);
            latlng_sensor.lat = LocationHelper::radianToDegree(sensorlatlng.lat);
            latlng_sensor.lng = LocationHelper::radianToDegree(sensorlatlng.lng);

            latlng_entity.lat = LocationHelper::radianToDegree(entitylatlng.lat);
            latlng_entity.lng = LocationHelper::radianToDegree(entitylatlng.lng);
#else
            latlng_sensor.lat = (double)(mapitem.m_sensorposinfo.PARAM_latitude)/LON_LAT_ACCURACY;
            latlng_sensor.lng = (double)(mapitem.m_sensorposinfo.PARAM_longitude)/LON_LAT_ACCURACY;

            latlng_entity.lat = (double)(mapitem.m_entityposinfo.PARAM_latitude)/LON_LAT_ACCURACY;
            latlng_entity.lng = (double)(mapitem.m_entityposinfo.PARAM_longitude)/LON_LAT_ACCURACY;
#endif

        }
        else
        {
#if 0
            const H3INDEX& sensorentituhexidxold = mapitem.m_sensorposinfo.PARAM_pos_hexidx;
            const H3INDEX& entityhexidxsrcold = mapitem.m_entityposinfo.PARAM_pos_hexidx;
            LAT_LNG sensorlatlng = LocationHelper::doCell(sensorentituhexidxold);
            LAT_LNG entitylatlng = LocationHelper::doCell(entityhexidxsrcold);

            m_dataSourceList.push_back(std::make_tuple(bIdentifi,\
                                                       sendorname,\
                                                       entityname, \
                                                       mapitem,\
                                                       LAT_LNG{LocationHelper::radianToDegree(sensorlatlng.lat),LocationHelper::radianToDegree(sensorlatlng.lng)},LAT_LNG{LocationHelper::radianToDegree(entitylatlng.lat),LocationHelper::radianToDegree(entitylatlng.lng)}));
#else
            m_dataSourceList.push_back(std::make_tuple(bIdentifi,\
                                                       sendorname,\
                                                       entityname, \
                                                       mapitem,\
                                                       LAT_LNG{(double)(mapitem.m_sensorposinfo.PARAM_latitude)/LON_LAT_ACCURACY, (double)(mapitem.m_sensorposinfo.PARAM_longitude)/LON_LAT_ACCURACY},\
                                                       LAT_LNG{(double)(mapitem.m_entityposinfo.PARAM_latitude)/LON_LAT_ACCURACY, (double)(mapitem.m_entityposinfo.PARAM_longitude)/LON_LAT_ACCURACY}));
            std::cout <<"add event"<<std::endl;
#endif
        }
    }
    else
    {
        auto entity_ulid_listitor = std::find_if(m_dataSourceList.begin(),
                                                 m_dataSourceList.end(),
                                                 [&](const LISTVIEW_ITEM_TYPE & vl){
                                                     const bool &bIdentifi = std::get<0>(vl);
                                                     const EVENT_INFO &info = std::get<3>(vl);
                                                     return bIdentifi & (info.m_sensorid == mapitem.m_sensorid) && (info.m_entityid == mapitem.m_entityid) && (info.m_sensingmediaid == mapitem.m_sensingmediaid);
                                                 });
        if(entity_ulid_listitor != m_dataSourceList.end())
        {
#ifdef SHOW_EVENT_DETAIL_REMOVE
            m_dataSourceList.erase(entity_ulid_listitor);
#else
            bool &bIdentifiold = std::get<0>(*entity_ulid_listitor);
            EVENT_INFO &info = std::get<3>(*entity_ulid_listitor);

            info = mapitem;
            bIdentifiold = false;

            LAT_LNG &latlng_sensor = std::get<4>(*entity_ulid_listitor);
            LAT_LNG &latlng_entity = std::get<5>(*entity_ulid_listitor);

#if 0
            LAT_LNG sensorlatlng = LocationHelper::doCell(info.m_sensorposinfo.PARAM_pos_hexidx);
            LAT_LNG entitylatlng = LocationHelper::doCell(info.m_entityposinfo.PARAM_pos_hexidx);
            latlng_sensor.lat = LocationHelper::radianToDegree(sensorlatlng.lat);
            latlng_sensor.lng = LocationHelper::radianToDegree(sensorlatlng.lng);

            latlng_entity.lat = LocationHelper::radianToDegree(entitylatlng.lat);
            latlng_entity.lng = LocationHelper::radianToDegree(entitylatlng.lng);
#else
            latlng_sensor.lat = (double)(mapitem.m_sensorposinfo.PARAM_latitude)/LON_LAT_ACCURACY;
            latlng_sensor.lng = (double)(mapitem.m_sensorposinfo.PARAM_longitude)/LON_LAT_ACCURACY;

            latlng_entity.lat = (double)(mapitem.m_entityposinfo.PARAM_latitude)/LON_LAT_ACCURACY;
            latlng_entity.lng = (double)(mapitem.m_entityposinfo.PARAM_longitude)/LON_LAT_ACCURACY;
#endif
#endif
            std::cout <<"remove event"<<std::endl;
        }
    }
#endif
    emit layoutChanged();
}

void DataSrcListViewModel::updateItem(const EVENT_INFO& mapitem, const QString &sendorname, const QString &entityname)
{
#ifndef SHOW_EVENT_DETAIL
    auto entity_ulid_listitor = std::find_if(m_dataSourceList.begin(),
                                             m_dataSourceList.end(),
                                             [&](const LISTVIEW_ITEM_TYPE & vl){
                                                 const bool &bIdentifi = std::get<0>(vl);
                                                 const EVENT_INFO &info = std::get<3>(vl);
                                                 return bIdentifi & (info.m_sensorid == mapitem.m_sensorid) && (info.m_entityid == mapitem.m_entityid) && (info.m_sensingmediaid == mapitem.m_sensingmediaid);
                                             });
    if(entity_ulid_listitor != m_dataSourceList.end())
    {
        EVENT_INFO &info = std::get<3>(*entity_ulid_listitor);
        info = mapitem;

        LAT_LNG &latlng_sensor = std::get<4>(*entity_ulid_listitor);
        LAT_LNG &latlng_entity = std::get<5>(*entity_ulid_listitor);

#if 0
        LAT_LNG sensorlatlng = LocationHelper::doCell(info.m_sensorposinfo.PARAM_pos_hexidx);
        LAT_LNG entitylatlng = LocationHelper::doCell(info.m_entityposinfo.PARAM_pos_hexidx);
        latlng_sensor.lat = LocationHelper::radianToDegree(sensorlatlng.lat);
        latlng_sensor.lng = LocationHelper::radianToDegree(sensorlatlng.lng);

        latlng_entity.lat = LocationHelper::radianToDegree(entitylatlng.lat);
        latlng_entity.lng = LocationHelper::radianToDegree(entitylatlng.lng);
#else
        latlng_sensor.lat = (double)(mapitem.m_sensorposinfo.PARAM_latitude)/LON_LAT_ACCURACY;
        latlng_sensor.lng = (double)(mapitem.m_sensorposinfo.PARAM_longitude)/LON_LAT_ACCURACY;

        latlng_entity.lat = (double)(mapitem.m_entityposinfo.PARAM_latitude)/LON_LAT_ACCURACY;
        latlng_entity.lng = (double)(mapitem.m_entityposinfo.PARAM_longitude)/LON_LAT_ACCURACY;
#endif
//        std::cout <<"update event"<<std::endl;
    }
    emit layoutChanged();
#endif
}


const QList<LISTVIEW_ITEM_TYPE > &DataSrcListViewModel::getData() const
{
    return m_dataSourceList;
}

void DataSrcListViewModel::remove(int index)
{
    m_dataSourceList.removeAt(index);
    emit layoutChanged();
}


void DataSrcListViewModel::refresh()
{
    emit layoutChanged();
}

void DataSrcListViewModel::clearData()
{
    m_dataSourceList.clear();
    emit layoutChanged();
}


DataSrcItemDelegate::DataSrcItemDelegate(DataSrcListViewModel *studentListViewModel, QObject *parent)
    : QStyledItemDelegate(parent)
{
    this->m_mapListViewModel=studentListViewModel;
}

DataSrcItemDelegate::~DataSrcItemDelegate()
{
}


bool DataSrcItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(index.column()==0)
    {
        const LISTVIEW_ITEM_TYPE& iteminfo = m_mapListViewModel->getItem(index.row());

        const bool &bIdentifi = std::get<0>(iteminfo);
        const EVENT_INFO &info = std::get<3>(iteminfo);

        const TYPE_ULID& sensorsulidold = info.m_sensorid;
        const TYPE_ULID& entityulidold = info.m_entityid;
        const TYPE_ULID& sensingmediaid = info.m_sensingmediaid;
        const H3INDEX& sensorentituhexidxold = info.m_sensorposinfo.PARAM_pos_hexidx;
        const H3INDEX& entityhexidxsrcold = info.m_entityposinfo.PARAM_pos_hexidx;

        if((QEvent::MouseButtonPress == event->type()))
        {

            QMouseEvent *pEvent = static_cast<QMouseEvent *> (event);
            QPoint mousePoint = pEvent->pos();

//            tagDataSourceStatus::tagStatus status = tagDataSourceStatus::STATUS_EDITING;
//            bool bSet = false;
//            if(ConfigManager::GetInstance()->getSimpleMode())
//            {
//                if(ConfigManager::isPointInRect(mousePoint,mapitem.rcEnable,option.rect))
//                {
//                    mapitem.m_data->setEnable(!mapitem.m_data->getEnable());
//                    status = tagDataSourceStatus::STATUS_ENBALE;
//                    bSet = true;
//                }
//                else
//                {
//                    mapitem.m_bExpandStatus = false;
//                    status = tagDataSourceStatus::STATUS_EXPAND;
//                    bSet = true;
//                }
//            }
//            else
//            {
//                bool bOldExpand = mapitem.m_bExpandStatus;

//                if(ConfigManager::isPointInRect(mousePoint,mapitem.rcModifiy,option.rect))
//                {
//                    status = tagDataSourceStatus::STATUS_EDITING;
//                    bSet = true;
//                }
//                else if(ConfigManager::isPointInRect(mousePoint,mapitem.rcDelete,option.rect))
//                {
//                    status = tagDataSourceStatus::STATUS_DELETE;
//                    bSet = true;
//                }
//                else if(ConfigManager::isPointInRect(mousePoint,mapitem.rcEnable,option.rect))
//                {
//                    mapitem.m_data->setEnable(!mapitem.m_data->getEnable());
//                    status = tagDataSourceStatus::STATUS_ENBALE;
//                    bSet = true;
//                }
//                else
//                {
//                    mapitem.m_bExpandStatus = !bOldExpand;
//                    status = tagDataSourceStatus::STATUS_EXPAND;
//                    bSet = true;
//                }

//                if((bOldExpand != mapitem.m_bExpandStatus))
//                {
//                    if((m_mapItemCtrl->getSourceType() == E_SOURCECTRL_TYPE_DATASOURCE)||
//                        (m_mapItemCtrl->getSourceType() == E_SOURCECTRL_TYPE_VIDEOSOURCE))
//                    {
//                        m_mapListViewModel->updateModelItemExpand(index.row(),mapitem);
//                    }
//                }
//            }

//            if(bSet)
//            {
//                tagDataSourceStatus p;
//                p.status = status;
//                if(status != tagDataSourceStatus::STATUS_EXPAND)
//                {
//                    p.pObject = mapitem;
//                    p.modelindex = index;
//                    tagDataSourceInfo * ptagDataSourceInfo = m_mapListViewModel->getDataSourceInfo(mapitem.m_data);
//                    if(ptagDataSourceInfo)
//                    {
//                        p.m_userDefinedOld = ptagDataSourceInfo->m_userDefined;
//                    }
//                }

//                QVariant v;
//                v.setValue(p);
//                emit btnOperateSignal(v);
//            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}



void DataSrcItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column()==0)
    {
        const LISTVIEW_ITEM_TYPE& iteminfo = m_mapListViewModel->getItem(index.row());

        const bool &bIdentifi = std::get<0>(iteminfo);
        const QString &sendorname = std::get<1>(iteminfo);
        const QString &entityname = std::get<2>(iteminfo);
        const EVENT_INFO &info = std::get<3>(iteminfo);
        const LAT_LNG &latlng_sensor = std::get<4>(iteminfo);
        const LAT_LNG &latlng_entity = std::get<5>(iteminfo);

        if(bIdentifi)
        {
            painter->fillRect(option.rect,QColor(0,255,0,64));
        }
        else
        {
            painter->fillRect(option.rect,QColor(128,128,128,64));
        }

        const TYPE_ULID& sensorsulidold = info.m_sensorid;
        const TYPE_ULID& entityulidold = info.m_entityid;
        const TYPE_ULID& sensingmediaid = info.m_sensingmediaid;
        const H3INDEX& sensorentituhexidxold = info.m_sensorposinfo.PARAM_pos_hexidx;
        const H3INDEX& entityhexidxsrcold = info.m_entityposinfo.PARAM_pos_hexidx;





        QString sensorulidstr = QString::number(sensorsulidold);
        QString entityulidstr = QString::number(entityulidold);
        QString sensingmediaidstr = QString::number(sensingmediaid);

//        QString tracestr = "sensor:"+sendorname +"\nentity:"+entityname+"\nsensor Location:"+QString::number(sensorentituhexidxold)+"\nentity Location: "+QString::number(entityhexidxsrcold);
        QString tracestr = "sensor:"+sendorname +" "+ sensorulidstr +"\nentity:"+entityname+" "+ entityulidstr +"\nsensingmedia:"+sensingmediaidstr +"\nsensor Location:("+QString::number(latlng_sensor.lat)+","+QString::number(latlng_sensor.lng)+")\nentity Location: ("+QString::number(latlng_entity.lat)+","+QString::number(latlng_entity.lng)+")";
        painter->drawText(option.rect, tracestr);
    }
    QStyledItemDelegate::paint(painter, option, index);
}


