#ifndef PARAMS_DEFINE_H
#define PARAMS_DEFINE_H
#include "LocationHelper.h"
#include "head_define.h"
#include <QJsonObject>

struct tagPtLatLngHgtInfo{
    LatLng _latlng;
    double _hgt;
    double _hgt_up;
    double _hgt_down;
};


struct tagLinePros{
    ////////////////////////////////////////////////
    QString code;
    QString startTime;
    QString endTime;
    QString createTime;
    QString Priority;
    ////////////////////////////////////////////////
    UINT32 dt_startTime;
    UINT32 dt_endTime;
    UINT32 dt_createTime;
    INT32 iPriority;
    bool bBuiling;
    ////////////////////////////////////////////////
    void prase()
    {
        dt_startTime = QDateTime::fromString(startTime,"yyyy-MM-dd hh:mm:ss").toTime_t();
        dt_endTime = QDateTime::fromString(endTime,"yyyy-MM-dd hh:mm:ss").toTime_t();
        dt_createTime = QDateTime::fromString(createTime,"yyyy-MM-dd hh:mm:ss").toTime_t();
        iPriority = Priority.toInt();
        bBuiling = false;
    }
};

struct tagLineInfo{
    UINT64 _id;
    QJsonObject featuresitemobj;
    QJsonObject featurecollectionpros;
    std::vector<tagPtLatLngHgtInfo> _line_latlnghgts;
    std::vector<std::vector<tagPtLatLngHgtInfo>> _polygon_latlnghgts;
    std::unordered_map<H3INDEX,double> _hex_hgt;

    tagLinePros m_tagLinePros;
    double m_distance;

    HEXIDX_ARRAY  m_hexidxs;

    HEXIDX_HGT_ARRAY  _hexidxs;
};

struct tagLineInfoEx;
struct tagSensorConflictInfo{
    std::tuple<TYPE_ULID,TYPE_ULID> m_sensorid;
    transdata_param_seq_hexidx_hgt m_hgtrange;
    tagLineInfoEx* m_ptagLineInfos;

};


namespace gaeactorenvironment_ex {
class GaeactorProcessorInterfaceInstance;
}

typedef gaeactorenvironment_ex::GaeactorProcessorInterfaceInstance *  GAEAPROCESSORINTERFACEINSTANCE_PTR;

#endif // PARAMS_DEFINE_H
