#ifndef GAEACTOR_MANAGER_H
#define GAEACTOR_MANAGER_H
#include <QObject>
#include <unordered_map>
#include "head_define.h"

#include "gaeactor_transmit_define.h"

#include "gaeactor_transmit_interface.h"
#ifdef USING_GAEACTOR_TRANSMIT
namespace gaeactoragentcores {
class GaeactorAgentCores;
}
namespace gaeactoragentsensors {
class GaeactorAgentSensors;
}

namespace gaeactortransmit {
	class GaeactorTransmit;
}
#endif

typedef std::function<void (const E_EVENT_MODE& ,const std::vector<EVENT_INFO>&)> eventlist_update_callback;
typedef std::function<void(const TYPE_ULID &, const TYPE_ULID &,const tagPathInfo&)> path_update_callback;
class KlusterWebSocketClient;

class GaeactorManagerHelper : public QObject
{
	Q_OBJECT
public:
	explicit GaeactorManagerHelper(QObject *parent = nullptr);
	virtual ~GaeactorManagerHelper();

	void registDisplayCallback(display_hexidx_update_callback func);
	void registDisplayPosCallback(display_pos_update_callback func);
	void registIntersectionDisplayCallback(intersection_display_hexidx_update_callback func);
	void registEchoWaveDisplayCallback(echowave_display_hexidx_update_callback func);

	void registEventlistUpdateCallback(const eventlist_update_callback &newEventlist_update_callback);
	void registPathUpdateCallback(const path_update_callback &newPath_update_callback);

	void registSensorUpdateCallback(const sensor_update_callback &newsensor_update_callback);

public slots:

	void dealtransformHexDataSlot(const QByteArray&by);
	void dealtransformHexAttDataSlot(const QByteArray&by);
	void dealtransformHexAttArrayDataSlot(const QByteArray&by);
	void dealtransformHexIntersectionDataSlot(const QByteArray&by);

	void dealtransformEchowaveDataSlot(const QByteArray&by);
	void dealtransformEchowaveArrayDataSlot(const QByteArray&by);
	void dealtransformEventlistDataSlot(const QByteArray&by);
	void dealtransformEventlistArrayDataSlot(const QByteArray&by);
	void dealtransformSensorPathArrayDataSlot(const QByteArray&by);
	void dealtransformSensorUpdateDataSlot(const QByteArray&by);
	void dealtransformSensorUpdateArrayDataSlot(const QByteArray&by);
private:

	display_hexidx_update_callback m_displaycallback;

	display_pos_update_callback m_displayPosCallback;
	intersection_display_hexidx_update_callback m_intersection_display_hexidx_update_callback;
	echowave_display_hexidx_update_callback m_echowave_display_hexidx_update_callback;
	eventlist_update_callback m_eventlist_update_callback;
	path_update_callback m_path_update_callback;
	sensor_update_callback m_sensor_update_callback;
};
class GaeactorManager : public QObject
{
    Q_OBJECT
public:
	enum E_RUNNING_MODE
	{
		E_RUNNING_MODE_REALTIME,
		E_RUNNING_MODE_REVIEW,
	};
	explicit GaeactorManager(E_RUNNING_MODE eRunningMode, QObject *parent = nullptr);
    virtual ~GaeactorManager();

	void dealSensorPath(const TYPE_ULID& src, const TYPE_ULID& dst);
    void dealentityHexidex(TYPE_ULID ulid, const LAT_LNG& pos);
    void clearentityHexidex(TYPE_ULID ulid);
    void dealHexidex(TYPE_ULID ulid, const LAT_LNG& pos, const HEXIDX_ARRAY& hexidxslist, const std::vector<transdata_param_seq_polygon>& _polygon, const UINT32 &slient_time_gap);
    void clearHexidex(TYPE_ULID ulid);

    void binary_receive_data_call_back(const BYTE*pdata, const UINT32& ilen);
    void receive_callback(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE*pdata, const UINT32& ilen, const BYTE*pOriginaldata, const UINT32& iOriginallen);
private:

    void transformHexData(const BYTE*pdata, const UINT32& ilen);
    void transformHexAttData(const BYTE*pdata, const UINT32& ilen);
	void transformHexAttArrayData(const BYTE*pdata, const UINT32& ilen);

    void transformHexIntersectionData(const BYTE*pdata, const UINT32& ilen);

    void transformEchowaveData(const BYTE*pdata, const UINT32& ilen);
	void transformEchowaveArrayData(const BYTE*pdata, const UINT32& ilen);
	void transformEventlistData(const BYTE*pdata, const UINT32& ilen);
	void transformEventlistArrayData(const BYTE*pdata, const UINT32& ilen);
	void transformSensorPathArrayData(const BYTE*pdata, const UINT32& ilen);

	void transformSensorUpdateData(const BYTE*pdata, const UINT32& ilen);
	void transformSensorUpdateArrayData(const BYTE*pdata, const UINT32& ilen);

    void SendBinaryMessage(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType,const char * message, int isize);
signals:
    void dealtransformHexDataSig(const QByteArray&by);
    void dealtransformHexAttDataSig(const QByteArray&by);
	void dealtransformHexAttArrayDataSig(const QByteArray&by);

    void dealtransformHexIntersectionDataSig(const QByteArray&by);

    void dealtransformEchowaveDataSig(const QByteArray&by);
	void dealtransformEchowaveArrayDataSig(const QByteArray&by);
	void dealtransformEventlistDataSig(const QByteArray&by);
	void dealtransformEventlistArrayDataSig(const QByteArray&by);
	void dealtransformSensorPathArrayDataSig(const QByteArray&by);

	void dealtransformSensorUpdateDataSig(const QByteArray&by);
	void dealtransformSensorUpdateArrayDataSig(const QByteArray&by);
	
private:


#ifdef USING_GAEACTOR_TRANSMIT
    std::tuple<std::string, CHANNEL_INFO*> m_Transmitchannel_AgentCores;

    std::tuple<std::string, CHANNEL_INFO*> m_Transmitchannel_AgentSensors;

    std::unordered_map<TYPE_ULID, std::tuple<std::string, CHANNEL_INFO*>> m_ulidchannel;

	gaeactortransmit::GaeactorTransmit * m_pGaeactorTransmit;
#endif
	KlusterWebSocketClient* m_wsclient_;

	E_RUNNING_MODE m_eRunningMode;
};
#endif // GAEACTOR_MANAGER_H
