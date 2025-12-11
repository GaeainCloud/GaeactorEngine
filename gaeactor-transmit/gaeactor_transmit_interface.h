#ifndef GAEACTOR_TRANSMIT_INTERFACE_H
#define GAEACTOR_TRANSMIT_INTERFACE_H

#include "gaeactor_transmit_global.h"
#include <QObject>
#include "gaeactor_transmit_define.h"
#include <QHash>
#include <QList>
#include <QVector>

//#define USING_GAEACTOR_TRANSMIT
#include "./proto/protoc/AgentPositionInfo.pb.h"
#include "./proto/protoc/AgentCommSnrInfo.pb.h"
#include "./proto/protoc/AgentRelationInfo.pb.h"


#include "internal_transformdata_define.h"
#include "transformdata_define.h"
#include "head_define.h"

namespace gaeactortransmit {
class GaeactorTransmitProcessor;
class GAEACTOR_TRANSMIT_EXPORT GaeactorTransmit : public QObject
{
    Q_OBJECT
public:
    explicit GaeactorTransmit(QObject *parent = nullptr);
    virtual ~GaeactorTransmit();
    void initDeployType(const E_DEPLOYMODE_TYPE& deployType,const std::vector<std::tuple<std::string,std::string>> & servicenamelist=std::vector<std::tuple<std::string,std::string>>());
    
    QString transmit_channel_id() const;
    void set_transmit_channel_id(const QString &new_transmit_channel_id);

    std::tuple<std::string, CHANNEL_INFO*>  getReusePublisherChannel();
    std::tuple<std::string, CHANNEL_INFO*>  requireIndependentPublisherChannel();

    void allocShareChannel(UINT32 iCount);
    CHANNEL_INFO *applyforShareChannel();
    bool removePublisherUseItem(const std::tuple<std::string, CHANNEL_INFO *> &itemChannelInfo);

    bool transmitData(const CHANNEL_INFO* channelinfo,const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE *pData, UINT32 iLen);

    void* loanTransmitBuffer(const CHANNEL_INFO *channelinfo, UINT32 iLen);
    void publish(const CHANNEL_INFO* channelinfo);

    void setDataCallback(receive_callback func);

    void printMempoolInfo();



    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static ::msg::AgentPositionInfo::msg_transentityhexidxpostdata SerializePositionData(const transentityhexidxpostdata& _data);
    static void DeserializePositionData(const ::msg::AgentPositionInfo::msg_transentityhexidxpostdata& _data, transentityhexidxpostdata& positiondata);


    static ::msg::AgentPositionInfo::msg_transentityhexidxdata SerializeHexidxData(const transentityhexidxdata& _data, const HEXIDX_HGT_ARRAY &hexidxslist, const POLYGON_LIST &polygonlist);
    static void DeserializeHexidxData(const ::msg::AgentPositionInfo::msg_transentityhexidxdata& _data,transentityhexidxdata &hexidxdata,HEXIDX_HGT_ARRAY &hexidxslist,POLYGON_LIST &polygonlist);


    static ::msg::AgentPositionInfo::msg_transdata_entityposinfo SerializeEntityData(const transdata_entityposinfo& _data);
    static void DeserializeEntityData(const ::msg::AgentPositionInfo::msg_transdata_entityposinfo& _data, transdata_entityposinfo& entityinfo);


    static ::msg::AgentPositionInfo::msg_transdata_sensorposinfo SerializeSensorData(const transdata_sensorposinfo& _data);
    static void DeserializeSensorData(const ::msg::AgentPositionInfo::msg_transdata_sensorposinfo& _data, transdata_sensorposinfo& sensorinfo);


    static ::msg::AgentPositionInfo::msg_transeventdata SerializeEventData(const transeventdata& _data);
    static void DeserializeEventData(const ::msg::AgentPositionInfo::msg_transeventdata& _data, EVENT_INFO& sensorinfo);


private:
    GaeactorTransmitProcessor * m_pGaeactorTransmitProcessor;
    E_DEPLOYMODE_TYPE m_deployType;

};
}
#endif // GAEACTOR_TRANSMIT_INTERFACE_H
