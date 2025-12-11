#ifndef GAEACTOR_ENVIRONMENT_INTERFACE_H
#define GAEACTOR_ENVIRONMENT_INTERFACE_H

#include "gaeactor_environment_global.h"
#include <QObject>
#include "gaeactor_environment_define.h"
#include <QHash>
#include <QList>
#include <QVector>
#include <QMutex>

#include <QReadWriteLock>

#include "gaeactor_environment_datahelper.h"
class Dijkstra;
namespace gaeactorenvironment {

	typedef std::function<void(bool)> data_update_callback;

	class GAEACTOR_ENVIRONMENT_EXPORT GaeactorEnvironment : public QObject
	{
		Q_OBJECT
	public:
		enum E_PROCESS_TYPE : UINT32
		{
			E_PROCESS_TYPE_UNKNOWN = 0x00,
			E_PROCESS_TYPE_AUDITIONS = 0x01,
			E_PROCESS_TYPE_EVENTS = 0x02,
			E_PROCESS_TYPE_INTERACTIONS = 0x04,
			E_PROCESS_TYPE_ALL = E_PROCESS_TYPE_AUDITIONS | E_PROCESS_TYPE_EVENTS | E_PROCESS_TYPE_INTERACTIONS,
		};

		static GaeactorEnvironment & getInstance();
		explicit GaeactorEnvironment(QObject *parent = nullptr);
		virtual ~GaeactorEnvironment();

		bool registEntityInfo(const H3INDEX& h3Index, const TYPE_ULID &entityulid, transdata_entityposinfo &eninfo);
		void buildSensorsInfo(const H3INDEX& h3Index, const TYPE_ULID &sensorulid, const TYPE_ULID& sensingmediaid);
		void buildSensorsDriverInterveneInfo(const std::vector<transdata_param_seq_hexidx> &hexidxsinfolist, const TYPE_ULID &_sensorulid, const TYPE_ULID &sensingmediaid);
		void buildSensorsUlidHexidxsInfo(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const std::vector<transdata_param_seq_hexidx> &_hexidxs, const transdata_sensorposinfo& _sensorinfo, const std::vector<transdata_param_seq_polygon>& _polygon, E_WAVE_SENSOR_SOURCE_TYPE eWaveSensorSourceType);
		std::unordered_map<UINT64, H3INDEX> buildEntityInfo(const H3INDEX &h3Indexsrc, const TYPE_ULID &entityulid, QList<UINT64> &reslist);

		void clearEntity(const TYPE_ULID &ulid);
		void clearInvalidSensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, bool bClerSensor, bool bAssignEchowave = false);

		void updateEntityHexidxInfo(std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, std::unordered_map<UINT64, H3INDEX>, bool>>& _entity_hexidx_infos);
		void updateHexidxEntityInfo(std::unordered_map<H3INDEX, std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, bool>>> &_hexidx_entity_infos);



		void updateInterveneInfo(const TYPE_ULID &ulid, const std::unordered_map<UINT64, H3INDEX> &resolution_target_hexidxs, E_WAVE_SENSOR_SOURCE_TYPE eWaveSensorSourceType, const TYPE_ULID &entityulid, const H3INDEX &h3Indexsrc, INTERSECTION_HEXIDX_LIST &intersectionHexidx);


		tagPathInfo getPath(const TYPE_ULID &uildsrc, const TYPE_ULID &dst);
		bool getUlidSensorInfo(const TYPE_ULID &ulid, const TYPE_ULID& sensingmediaid, transdata_sensorposinfo &_sensorinfo);
		bool getUlidSensorHexidxs(const TYPE_ULID &ulid, const TYPE_ULID &sensingmediaid, SENSOR_INFO_TYPE& sensor_info, bool bget);
		H3INDEX getEntityHexidx(const TYPE_ULID& ulid);
		transdata_entityposinfo gettransdata_entityposinfo(const TYPE_ULID& ulid);
		tagSensorInfo gettransdata_sensorposinfo(const TYPE_ULID& ulid);

		CLEAR_RELATION_INFO_LIST refershRelation();
		IDENTIFI_EVENT_INFO refershEvent(int id);
		void refershInteractions(int id);
		QList<UINT64> getResolutions();

		void registDisplayCallback(echowave_display_hexidx_update_callback func);
		void registDisplayListCallback(echowave_list_display_hexidx_update_callback func);

		void registHexidxDisplayCallback(display_hexidx_update_callback func);
		void registSensorUpdateCallback(sensor_update_callback func);

		void regist_data_update_callback(E_PROCESS_TYPE eprocessType, data_update_callback func);

	private:
		std::unordered_map<TYPE_ULID, QList<TYPE_ULID> > getResolutionsHexidxs(UINT64 resolution);

		std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool> > > getSensorsHexidxs(const H3INDEX& h3Index);
		std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool> > > getSensorsResolutionHexidxs(const H3INDEX& h3Index);
		std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool> > > getSensorsResolutionHexidxs(const UINT64 &hexidx_resolution, const H3INDEX& h3Index);


		void refershDataUpdate(E_PROCESS_TYPE eprocessType);
		//private:
		//    explicit GaeactorEnvironment(QObject *parent = nullptr);
	private:
		void registResolutions(const H3INDEX& h3Index, const TYPE_ULID &ulid_ele, const TYPE_ULID& sensingmediaid);
		void appendResolutions(const UINT64& hexidx_resolution, const TYPE_ULID & ulid_ele, const TYPE_ULID &sensingmediaid);
		void appendEntitysUlid(const H3INDEX &h3Index, const TYPE_ULID &entityulid, bool bSensorProperty, transdata_entityposinfo &eninfo);
		void setEntitySensorProperty(const TYPE_ULID &sensorulid, bool bSensor);
		void clearInvalidEntity();
		ENTITY_HEXIDX_INFO getEntityHexidxInfo(const TYPE_ULID &sensorulid);
		QList<TYPE_ULID> getEntitySensorPropertyList();
		void clearEntityInterceveInfo(const TYPE_ULID &ulid, bool isSensorProperty);
		void appendSensorsUlidHexidxsInfo(const TYPE_ULID &sensorulid, const TYPE_ULID &entityulid, const std::vector<transdata_param_seq_hexidx> &_hexidxs, const transdata_sensorposinfo& _sensorinfo, const std::vector<transdata_param_seq_polygon> &_polygon, E_WAVE_SENSOR_SOURCE_TYPE eWaveSensorSourceType);

		CLEAR_RELATION_INFO_LIST refershSensorSilenceStatusclearSensorEntityRelation();
		bool isEntitySensorProperty(const TYPE_ULID &entityulid);
		bool isEntityHaveSensorProperty(const TYPE_ULID &entityulid);

		QList<std::tuple<TYPE_ULID, H3INDEX>> getValidEntityList();
		QList<std::tuple<TYPE_ULID, H3INDEX>> getValidSensorEntityList();
		QList<std::tuple<TYPE_ULID, H3INDEX>> getValidAllEntityList();
		QList<std::tuple<TYPE_ULID, H3INDEX>> getValidAllExcludeEntityList(const TYPE_ULID &ulid);

		void buildHexidxULIDMap(const H3INDEX& h3Index, const TYPE_ULID &ulid_ele, const TYPE_ULID &sensingmediaid);
		void buildResolutionHexidxULIDMap(const UINT64 &hexidx_resolution, const H3INDEX& h3Index, const TYPE_ULID &ulid_ele, const TYPE_ULID &sensingmediaid);

		void clearHexidxULIDMap(const H3INDEX& h3Index, const QList<QPair<TYPE_ULID, TYPE_ULID>> &ulid_ele);
		void clearResolutionHexidxULIDMap(const H3INDEX& h3Index, const QList<QPair<TYPE_ULID, TYPE_ULID> > &ulid_ele);

		void releaseHexidxULIDMap();
		void releaseResolutionHexidxULIDMap();
	private:
		//单个hexidx 多个ulid对应关系
		QReadWriteLock m_hexidx_ulid_mutex;
		std::unordered_map<H3INDEX, std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>> m_hexidx_ulid;


		//单个resolution 单个hexidx 多个ulid对应关系
		QReadWriteLock m_resolution_hexidx_ulid_mutex;
		std::unordered_map<UINT64, std::unordered_map<H3INDEX, std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>>> m_resolution_hexidx_ulid;

		//单个resolution 多个ulid对应关系
		QReadWriteLock m_resolution_sensorsulid_mutex;
		std::unordered_map<UINT64, std::unordered_map<TYPE_ULID, QList<TYPE_ULID>>> m_resolution_sensorsulid;

		//波单个ulid 多个hexidxs对应关系
		QReadWriteLock m_sensors_ulid_hexidxs_mutex;
		SENSORS_INTERCEVE_HASHMAP m_sensors_ulid_hexidxs;

		QReadWriteLock m_sensors_ulid_info_mutex;
		std::unordered_map<TYPE_ULID, tagSensorInfo> m_sensors_ulid_info;

		QReadWriteLock m_entity_interactions_list_mutex;
		std::unordered_map<TYPE_ULID, std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, bool>> m_entity_interactions_list;

		QReadWriteLock m_sensorslist_mutex;
		std::unordered_map<TYPE_ULID, UINT64> m_sensorslist;

		QReadWriteLock m_entitys_ulid_mutex;
		ENTITY_HEXIDX_HASHMAP m_entitys_ulid;

		QReadWriteLock m_resolution_hexidx_entity_ulid_mutex;
		std::unordered_map<UINT64, std::unordered_map<H3INDEX, std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, bool>>>> m_resolution_hexidx_entity_ulid;

		QReadWriteLock m_entity_resolution_hexidx_ulid_mutex;
		std::unordered_map<TYPE_ULID, std::unordered_map<UINT64, H3INDEX>> m_entity_resolution_hexidx_ulid;


		std::unordered_map<TYPE_ULID, bool> m_event_thread_deal_list[EVENTS_THREAD_NUM];
		uint64_t m_event_thread_index;
		EVENTS_HASHMAP m_exist_events_map[EVENTS_THREAD_NUM];

		std::unordered_map<TYPE_ULID, bool> m_interactions_thread_deal_list[INTERACTIONS_THREAD_NUM];
		uint64_t m_interactions_thread_index;
		std::unordered_map<EVENT_KEY_TYPE, std::tuple<H3INDEX, H3INDEX, bool, bool>> m_exist_interactions_map[INTERACTIONS_THREAD_NUM];


		QReadWriteLock m_entitySensormap_mutex;
		std::unordered_map<TYPE_ULID, std::unordered_map<TYPE_ULID, bool>> m_entitySensormap;

		QReadWriteLock m_hexidx_entity_infos_mutex;
		std::unordered_map<H3INDEX, std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, bool>>> m_hexidx_entity_infos;


		QReadWriteLock m_entity_hexidx_infos_mutex;
		std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, std::unordered_map<UINT64, H3INDEX>, bool> > m_entity_hexidx_infos;

		echowave_display_hexidx_update_callback m_echowave_display_hexidx_update_callback;
		echowave_list_display_hexidx_update_callback m_echowave_list_display_hexidx_update_callback;
		display_hexidx_update_callback m_display_hexidx_update_callback;

		sensor_update_callback m_sensor_update_callback;

		std::unordered_map<E_PROCESS_TYPE, data_update_callback> m_data_update_callback_lsit;

		Dijkstra *m_pDijkstra;

		QList<UINT64> m_all_resolutions;
	};
}
#endif // GAEACTOR_ENVIRONMENT_INTERFACE_H



