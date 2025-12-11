#include "gaeactor_environment_interface.h"
#include <h3Index.h>
#include <QDebug>
#include "src/OriginalDateTime.h"
#include "base_define.h"
#include "h3helper.h"
#include "easy/profiler.h"
#include "loghelper.h"
#include "src/algorithm/Dijkstra.h"

#include <QReadLocker>
#include <QWriteLocker>
//#define GENERATE_SECOND_ECHO_WAVE
//#define GENERATE_INTERSECTION_HEDIDX_LIST
//#define ENABLE_ERASE
//#define CLEAR_SENSOR_TO_SENSOR_INTERSECTION
//#define WRITE_LOCKER_DECLARE(MUTEX) QMutexLocker locker
namespace gaeactorenvironment
{
	GaeactorEnvironment::GaeactorEnvironment(QObject *parent)
		:QObject(parent)
		, m_event_thread_index(0)
		, m_interactions_thread_index(0)
		, m_pDijkstra(nullptr)
	{
		m_pDijkstra = new Dijkstra();
		m_pDijkstra->Create_graph(GRAPH_MAX_VEX);
		for (int i = 1; i <= 15; i++)
		{
			m_all_resolutions.push_back(i);
		}
	}

	GaeactorEnvironment::~GaeactorEnvironment()
	{
		if (m_pDijkstra)
		{
			delete m_pDijkstra;
		}
	}


	bool GaeactorEnvironment::registEntityInfo(const H3INDEX &h3Index, const TYPE_ULID &entityulid, transdata_entityposinfo &eninfo)
	{
		//EASY_FUNCTION(profiler::colors::Skin)
        bool isSensor = (PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR) ? true : false;
		appendEntitysUlid(h3Index, entityulid, isSensor, eninfo);

		//buildEntityInfo(h3Index, entityulid);

		return isSensor;
	}


	void GaeactorEnvironment::buildSensorsInfo(const H3INDEX &h3Index, const TYPE_ULID &sensorulid, const TYPE_ULID& sensingmediaid)
	{
		registResolutions(h3Index, sensorulid, sensingmediaid);
	}

	void GaeactorEnvironment::buildSensorsDriverInterveneInfo(const std::vector<transdata_param_seq_hexidx> &hexidxsinfolist, const TYPE_ULID &_sensorulid, const TYPE_ULID &sensingmediaid)
	{
		std::unordered_map<TYPE_ULID, bool> entity_new_interactions_list;

		auto sensor_sensingmedia_id_pair = qMakePair(_sensorulid, sensingmediaid);
		auto AppendSensorInteractionsInfo = [&](const H3INDEX& reshexidx, const H3INDEX &h3Indexsrc, const TYPE_ULID & entityulid) {
			//操作指定的sensorulid 的干涉列表，可以直接往 干涉列表中添加
			//EASY_FUNCTION(profiler::colors::Olive)
			QReadLocker _sensors_ulid_hexidxs_mutex_locker(&m_sensors_ulid_hexidxs_mutex);
			auto _sensors_ulid_hexidxs_itor = m_sensors_ulid_hexidxs.find(_sensorulid);
			if (_sensors_ulid_hexidxs_itor != m_sensors_ulid_hexidxs.end())
			{
				SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = _sensors_ulid_hexidxs_itor->second;
				if (sensor_interceve_entity_info.m_sensor_property.m_bInterceveInfoValid)
				{
					//波为非静默状态才处理对应的感知
					if (!sensor_interceve_entity_info.m_sensor_property.m_bClose)
					{
						QPair<TYPE_ULID, TYPE_ULID> entity_sensingmedia_id_pair = qMakePair(entityulid, sensor_sensingmedia_id_pair.second);
						SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP & entity_ulid_list = sensor_interceve_entity_info.get_sensor_entity_interceve_info_hashmap_lock_ForWrite();
						auto entity_ulid_listitor = entity_ulid_list.find(entity_sensingmedia_id_pair);
						if (entity_ulid_listitor != entity_ulid_list.end())
						{
							entity_ulid_listitor->second.m_resolutionhexidx = reshexidx;
							entity_ulid_listitor->second.m_srchexidx = h3Indexsrc;
							entity_ulid_listitor->second.setInterceve_status(true);
							m_pDijkstra->updateEdge(_sensorulid, entityulid, 1);
						}
						else
						{
							SENSOR_ENTITY_INTERCEVE_INFO sensorrntityinterceveinfo(reshexidx, h3Indexsrc, true);
							entity_ulid_list.insert(std::make_pair(entity_sensingmedia_id_pair, std::move(sensorrntityinterceveinfo)));
							m_pDijkstra->updateEdge(_sensorulid, entityulid, 1);
						}
						sensor_interceve_entity_info.set_sensor_entity_interceve_info_hashmap_unlock();
						auto entity_new_interactions_list_itor = entity_new_interactions_list.find(entityulid);
						if (entity_new_interactions_list_itor != entity_new_interactions_list.end())
						{
							entity_new_interactions_list_itor->second = true;
						}
						else
						{
							entity_new_interactions_list.insert(std::make_pair(entityulid, true));
						}
#ifdef GENERATE_INTERSECTION_HEDIDX_LIST
						intersectionHexidx.push_back(std::make_tuple(entityulid, sensorulid, reshexidx));
#endif
					}
					refershDataUpdate(E_PROCESS_TYPE_ALL);
				}
			}
		};

		//遍历场的hexidx列表
		for (auto hexidx_element : hexidxsinfolist)
		{
			const H3INDEX &h3Index = hexidx_element.PARAM_seq_hexidx_element;

			//registResolutions(h3Index, _sensorulid);

			{
				//得到当前hexidx 的分辨率
				UINT64 hexidx_resolution = H3_GET_RESOLUTION(h3Index);

				QReadLocker locker(&m_resolution_hexidx_entity_ulid_mutex);
				//获取指定的分辨率-hexidx 哈希表
				auto _resolution_hexidx_entity_ulid_itor = m_resolution_hexidx_entity_ulid.find(hexidx_resolution);
				if (_resolution_hexidx_entity_ulid_itor != m_resolution_hexidx_entity_ulid.end())
				{
					//获取指定的hexidx-实体列表 哈希表
					auto _hexidx_entity_ulid_itor = _resolution_hexidx_entity_ulid_itor->second.find(h3Index);
					if (_hexidx_entity_ulid_itor != _resolution_hexidx_entity_ulid_itor->second.end())
					{
						//遍历hexidx下的 实体列表 哈希表
						std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, bool>> &entitylist = _hexidx_entity_ulid_itor->second;
						auto entitylistitor = entitylist.begin();
						while (entitylistitor != entitylist.end())
						{
							const TYPE_ULID & entityulid = entitylistitor->first;
							//排除当前的场实体
							if (entityulid != _sensorulid)
							{
								//当前实体挂载有效
								if (std::get<1>(entitylistitor->second))
								{
									//通过场的hexidx查找到匹配相交的实体hexidx，设置相交有效,本轮被其他其他位置过无效的话，将其改写为有效
									AppendSensorInteractionsInfo(h3Index, std::get<0>(entitylistitor->second), entityulid);
								}
								else
								{
									//当前实体挂载无效，且本轮未被其他位置hexidx设置过有效
									auto entity_new_interactions_list_itor = entity_new_interactions_list.find(entityulid);
									if (entity_new_interactions_list_itor == entity_new_interactions_list.end())
									{
										entity_new_interactions_list.insert(std::make_pair(entityulid, false));
									}
								}
							}
							entitylistitor++;
						}
					}
				}
			}
		}

		auto clearSensorAllInteractionsInfo = [&]() {
			//操作指定的sensorulid 的干涉列表，可以直接往 干涉列表中添加
			//EASY_FUNCTION(profiler::colors::CreamWhite)
			QReadLocker _sensors_ulid_hexidxs_mutex_locker(&m_sensors_ulid_hexidxs_mutex);
			auto _sensors_ulid_hexidxs_itor = m_sensors_ulid_hexidxs.find(_sensorulid);
			if (_sensors_ulid_hexidxs_itor != m_sensors_ulid_hexidxs.end())
			{
				SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = _sensors_ulid_hexidxs_itor->second;
				if (sensor_interceve_entity_info.m_sensor_property.m_bInterceveInfoValid)
				{
					SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP & entity_ulid_list = sensor_interceve_entity_info.get_sensor_entity_interceve_info_hashmap_lock_ForRead();
					//不存在，则在干涉实体的ULID列表中检查清理
					auto entity_ulid_listitor = entity_ulid_list.begin();
					while (entity_ulid_listitor != entity_ulid_list.end())
					{
						if (entity_ulid_listitor->first.second == sensor_sensingmedia_id_pair.second)
						{
							entity_ulid_listitor->second.setInterceve_status(false);

							auto entity_ulid_list_valid_itor = std::find_if(entity_ulid_list.begin(),
								entity_ulid_list.end(),
								[&](SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP::value_type &vt)
							{
								return vt.first.first == entity_ulid_listitor->first.first && vt.second.m_bInterceveValid;
							});
							if (entity_ulid_list_valid_itor == entity_ulid_list.end())
							{
								m_pDijkstra->updateEdge(_sensorulid, entity_ulid_listitor->first.first, DIJKSTRA_MAX_VALUE);
							}
						}
						entity_ulid_listitor++;
					}
					sensor_interceve_entity_info.set_sensor_entity_interceve_info_hashmap_unlock();

					refershDataUpdate(E_PROCESS_TYPE_ALL);
				}
			}
		};

		auto clearSensorInteractionsInfo = [&](const TYPE_ULID& entityulid) {
			//操作指定的sensorulid 的干涉列表，可以直接往 干涉列表中添加
			//EASY_FUNCTION(profiler::colors::CreamWhite)
			QReadLocker _sensors_ulid_hexidxs_mutex_locker(&m_sensors_ulid_hexidxs_mutex);
			auto _sensors_ulid_hexidxs_itor = m_sensors_ulid_hexidxs.find(_sensorulid);
			if (_sensors_ulid_hexidxs_itor != m_sensors_ulid_hexidxs.end())
			{
				SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = _sensors_ulid_hexidxs_itor->second;
				if (sensor_interceve_entity_info.m_sensor_property.m_bInterceveInfoValid)
				{
					QPair<TYPE_ULID, TYPE_ULID> entity_sensingmedia_id_pair = qMakePair(entityulid, sensor_sensingmedia_id_pair.second);
					SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP & entity_ulid_list = sensor_interceve_entity_info.get_sensor_entity_interceve_info_hashmap_lock_ForRead();
					//不存在，则在干涉实体的ULID列表中检查清理
					auto entity_ulid_listitor = entity_ulid_list.find(entity_sensingmedia_id_pair);
					if (entity_ulid_listitor != entity_ulid_list.end())
					{
						entity_ulid_listitor->second.setInterceve_status(false);

						auto entity_ulid_list_valid_itor = std::find_if(entity_ulid_list.begin(),
							entity_ulid_list.end(),
							[&](SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP::value_type &vt)
						{
							return vt.first.first == entityulid && vt.second.m_bInterceveValid;
						});
						if (entity_ulid_list_valid_itor == entity_ulid_list.end())
						{
							m_pDijkstra->updateEdge(_sensorulid, entityulid, DIJKSTRA_MAX_VALUE);
						}
					}
					sensor_interceve_entity_info.set_sensor_entity_interceve_info_hashmap_unlock();

					refershDataUpdate(E_PROCESS_TYPE_ALL);
				}
			}
		};

		if (entity_new_interactions_list.empty())
		{
			clearSensorAllInteractionsInfo();
		}
		else
		{
			//根据旧的实体被场干涉的列表与当前新的实体被干涉的场的列表做对比，得出哪些场未感知该实体了，对之前感知干涉了的 当前未感知干涉的场 做干涉状态清理
			std::vector<TYPE_ULID> exist_old_interactions_list;

			//遍历与场产生碰撞的实体列表
			auto entity_new_interactions_list_itor = entity_new_interactions_list.begin();
			while (entity_new_interactions_list_itor != entity_new_interactions_list.end())
			{
				const TYPE_ULID & entityulid = entity_new_interactions_list_itor->first;
				bool bValid = entity_new_interactions_list_itor->second;
				//            if(!bValid)
				//            {
				//                exist_old_interactions_list.push_back(entityulid);
				//            }
				QWriteLocker _entity_interactions_list_mutex_locker(&m_entity_interactions_list_mutex);
				auto m_entity_interactions_list_itor = m_entity_interactions_list.find(entityulid);
				if (m_entity_interactions_list_itor == m_entity_interactions_list.end())
				{
					//将场信息加入实体被干涉的场的列表
					if (bValid)
					{
						std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, bool> new_interactions_list;
						new_interactions_list.insert(std::make_pair(sensor_sensingmedia_id_pair, true));
						m_entity_interactions_list.insert(std::make_pair(entityulid, std::move(new_interactions_list)));
					}
				}
				else
				{
					std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, bool> &old_interactions_list = m_entity_interactions_list_itor->second;
					auto old_interactions_list_itor = old_interactions_list.find(sensor_sensingmedia_id_pair);
					if (old_interactions_list_itor == old_interactions_list.end())
					{
						//将场信息加入实体被干涉的场的列表
						if (bValid)
						{
							old_interactions_list.insert(std::make_pair(sensor_sensingmedia_id_pair, true));
						}
					}
					else
					{
						//将场信息从实体被干涉的场的列表移除
						if (!bValid)
						{
							exist_old_interactions_list.push_back(entityulid);
							old_interactions_list_itor = old_interactions_list.erase(old_interactions_list_itor);
						}
					}
				}

				entity_new_interactions_list_itor++;
			}

			//清除场与实体未相交的信息
			auto old_interactions_list_itor = exist_old_interactions_list.begin();
			while (old_interactions_list_itor != exist_old_interactions_list.end())
			{
				const TYPE_ULID& entityulid = *old_interactions_list_itor;
				clearSensorInteractionsInfo(entityulid);
				old_interactions_list_itor++;
			}
		}
	}

	void GaeactorEnvironment::buildSensorsUlidHexidxsInfo(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const std::vector<transdata_param_seq_hexidx> &_hexidxs, const transdata_sensorposinfo& _sensorinfo, const std::vector<transdata_param_seq_polygon> &_polygon, E_WAVE_SENSOR_SOURCE_TYPE eWaveSensorSourceType)
	{
		if (!_hexidxs.empty())
		{
			appendSensorsUlidHexidxsInfo(sensorulid, sensingmediaid, _hexidxs, _sensorinfo, _polygon, eWaveSensorSourceType);
		}
		switch (eWaveSensorSourceType)
		{
		case E_WAVE_SENSOR_SOURCE_TYPE_SRC:
		{
#if 0
			setEntitySensorProperty(sensorulid, true);
#endif
		}break;
		case E_WAVE_SENSOR_SOURCE_TYPE_ECHO:
		{
		}break;
		default:break;
		}
	}

	std::unordered_map<UINT64, H3INDEX> GaeactorEnvironment::buildEntityInfo(const H3INDEX &h3Indexsrc, const TYPE_ULID &entityulid, QList<UINT64> &reslist)
	{
		std::unordered_map<UINT64, H3INDEX> ret;
		//清理旧的数据 ，置为无效
		std::unordered_map<UINT64, H3INDEX> old_resolution_target_hexidxs;

		std::unordered_map<UINT64, H3INDEX> new_resolution_target_hexidxs;
		H3Helper::getTargetResolutionIndexs(new_resolution_target_hexidxs, h3Indexsrc, reslist);
		{
			QWriteLocker locker(&m_entity_resolution_hexidx_ulid_mutex);
			auto _entity_resolution_hexidx_ulid_itor = m_entity_resolution_hexidx_ulid.find(entityulid);
			if (_entity_resolution_hexidx_ulid_itor != m_entity_resolution_hexidx_ulid.end())
			{
				old_resolution_target_hexidxs = std::move(_entity_resolution_hexidx_ulid_itor->second);
				_entity_resolution_hexidx_ulid_itor->second = /*std::move*/(new_resolution_target_hexidxs);
			}
			else
			{
				m_entity_resolution_hexidx_ulid.insert(std::make_pair(entityulid, /*std::move*/(new_resolution_target_hexidxs)));
			}
		}

		for (const auto &item : old_resolution_target_hexidxs)
		{
			UINT64 resolution = item.first;
			H3INDEX reshexidx = item.second;
			bool bModify = true;
			auto new_itor = new_resolution_target_hexidxs.find(resolution);
			if (new_itor != new_resolution_target_hexidxs.end())
			{
				if (reshexidx == new_itor->second)
				{
					//新的和旧的内容未变化
					bModify = false;
				}
			}
			if (bModify)
			{
				QWriteLocker locker(&m_resolution_hexidx_entity_ulid_mutex);
				auto _resolution_hexidx_entity_ulid_itor = m_resolution_hexidx_entity_ulid.find(resolution);
				if (_resolution_hexidx_entity_ulid_itor != m_resolution_hexidx_entity_ulid.end())
				{
					auto _hexidx_entity_ulid_itor = _resolution_hexidx_entity_ulid_itor->second.find(reshexidx);
					if (_hexidx_entity_ulid_itor != _resolution_hexidx_entity_ulid_itor->second.end())
					{
						std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, bool>> &entitylist = _hexidx_entity_ulid_itor->second;
						auto entitylistitor = entitylist.find(entityulid);
						if (entitylistitor != entitylist.end())
						{
							std::get<0>(entitylistitor->second) = h3Indexsrc;
							std::get<1>(entitylistitor->second) = false;
							entitylist.erase(entitylistitor);
						}
						if (entitylist.empty())
						{
							_resolution_hexidx_entity_ulid_itor->second.erase(_hexidx_entity_ulid_itor);
						}
					}
					if (_resolution_hexidx_entity_ulid_itor->second.empty())
					{
						m_resolution_hexidx_entity_ulid.erase(_resolution_hexidx_entity_ulid_itor);
					}
				}
			}
		}


		for (const auto &item : new_resolution_target_hexidxs)
		{
			UINT64 resolution = item.first;
			H3INDEX reshexidx = item.second;
			if (reslist.contains(resolution))
			{
				ret.insert(std::make_pair(resolution, reshexidx));
			}
			bool bModify = true;
			auto old_itor = old_resolution_target_hexidxs.find(resolution);
			if (old_itor != old_resolution_target_hexidxs.end())
			{
				if (reshexidx == old_itor->second)
				{
					//新的和旧的内容未变化
					bModify = false;
				}
			}
			if (bModify)
			{
				QWriteLocker locker(&m_resolution_hexidx_entity_ulid_mutex);
				auto _resolution_hexidx_entity_ulid_itor = m_resolution_hexidx_entity_ulid.find(resolution);
				if (_resolution_hexidx_entity_ulid_itor != m_resolution_hexidx_entity_ulid.end())
				{
					auto _hexidx_entity_ulid_itor = _resolution_hexidx_entity_ulid_itor->second.find(reshexidx);
					if (_hexidx_entity_ulid_itor != _resolution_hexidx_entity_ulid_itor->second.end())
					{
						std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, bool>> &entitylist = _hexidx_entity_ulid_itor->second;
						auto entitylistitor = entitylist.find(entityulid);
						if (entitylistitor != entitylist.end())
						{
							std::get<0>(entitylistitor->second) = h3Indexsrc;
							std::get<1>(entitylistitor->second) = true;
						}
						else
						{
							entitylist.insert(std::make_pair(entityulid, std::make_tuple(h3Indexsrc, true)));
						}
					}
					else
					{
						std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, bool>> entitylist;
						entitylist.insert(std::make_pair(entityulid, std::make_tuple(h3Indexsrc, true)));
						_resolution_hexidx_entity_ulid_itor->second.insert(std::make_pair(reshexidx, std::move(entitylist)));
					}
				}
				else
				{
					std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, bool>> entitylist;
					entitylist.insert(std::make_pair(entityulid, std::make_tuple(h3Indexsrc, true)));
					std::unordered_map<H3INDEX, std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, bool>>> _hexidx_entity_ulid;
					_hexidx_entity_ulid.insert(std::make_pair(reshexidx, std::move(entitylist)));
					m_resolution_hexidx_entity_ulid.insert(std::make_pair(resolution, std::move(_hexidx_entity_ulid)));
				}
			}
		}

		//    //设置新的数据
		//    {
		//        QWriteLocker locker(&m_entity_resolution_hexidx_ulid_mutex);
		//        auto _entity_resolution_hexidx_ulid_itor = m_entity_resolution_hexidx_ulid.find(entityulid);
		//        if(_entity_resolution_hexidx_ulid_itor != m_entity_resolution_hexidx_ulid.end())
		//        {
		//            _entity_resolution_hexidx_ulid_itor->second = new_resolution_target_hexidxs;
		//        }
		//        else
		//        {
		//            m_entity_resolution_hexidx_ulid.insert(std::make_pair(entityulid, std::move(new_resolution_target_hexidxs)));
		//        }
		//    }
		return ret;
	}


	void GaeactorEnvironment::clearEntity(const TYPE_ULID &ulid)
	{
		bool isSensorProperty = false;
		{
			QReadLocker _entitys_ulid_mutex_locker(&m_entitys_ulid_mutex);
			auto itor = m_entitys_ulid.find(ulid);
			if (itor != m_entitys_ulid.end())
			{
				isSensorProperty = itor->second.m_isSensor;
				itor->second.m_bEntityValid = false;

				if (isSensorProperty)
				{
					if (m_pDijkstra->removeNode(ulid))
					{
						//m_sensor_update_callback(ulid, E_EVENT_MODE_REMOVE);
					}
				}

				//清理旧的数据 ，置为无效
				///////////////////////////////////////////////////////////////////////////////////

				std::unordered_map<UINT64, H3INDEX> old_resolution_target_hexidxs;
				{
					QWriteLocker locker(&m_entity_resolution_hexidx_ulid_mutex);
					auto _entity_resolution_hexidx_ulid_itor = m_entity_resolution_hexidx_ulid.find(ulid);
					if (_entity_resolution_hexidx_ulid_itor != m_entity_resolution_hexidx_ulid.end())
					{
						old_resolution_target_hexidxs = _entity_resolution_hexidx_ulid_itor->second;
						_entity_resolution_hexidx_ulid_itor->second.clear();
					}
				}
				for (const auto &item : old_resolution_target_hexidxs)
				{
					UINT64 resolution = item.first;
					H3INDEX reshexidx = item.second;
					QWriteLocker locker(&m_resolution_hexidx_entity_ulid_mutex);
					auto _resolution_hexidx_entity_ulid_itor = m_resolution_hexidx_entity_ulid.find(resolution);
					if (_resolution_hexidx_entity_ulid_itor != m_resolution_hexidx_entity_ulid.end())
					{
						auto _hexidx_entity_ulid_itor = _resolution_hexidx_entity_ulid_itor->second.find(reshexidx);
						if (_hexidx_entity_ulid_itor != _resolution_hexidx_entity_ulid_itor->second.end())
						{
							std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, bool>> &entitylist = _hexidx_entity_ulid_itor->second;
							auto entitylistitor = entitylist.find(ulid);
							if (entitylistitor != entitylist.end())
							{
								std::get<1>(entitylistitor->second) = false;
								entitylist.erase(entitylistitor);
							}
							if (entitylist.empty())
							{
								_resolution_hexidx_entity_ulid_itor->second.erase(_hexidx_entity_ulid_itor);
							}
						}
						if (_resolution_hexidx_entity_ulid_itor->second.empty())
						{
							m_resolution_hexidx_entity_ulid.erase(_resolution_hexidx_entity_ulid_itor);
						}
					}
				}
			}
		}
		clearEntityInterceveInfo(ulid, isSensorProperty);

	}

	void GaeactorEnvironment::clearInvalidSensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, bool bClerSensor, bool bAssignEchowave)
	{
		//清理无效的场的感知域
		if (bClerSensor)
		{
			QList<H3INDEX> clearlist;
			{
				QReadLocker _sensors_ulid_hexidxs_mutex_locker(&m_sensors_ulid_hexidxs_mutex);
				auto itor = m_sensors_ulid_hexidxs.find(sensorulid);
				if (itor != m_sensors_ulid_hexidxs.end())
				{
					SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = itor->second;
					if (bAssignEchowave)
					{
						if (sensor_interceve_entity_info.m_sensor_property.m_eWaveSensorSourceType == E_WAVE_SENSOR_SOURCE_TYPE_SRC)
						{
							return;
						}
					}

					SENSOR_HEXIDX_INFO_HASHMAP & hexidxmap = sensor_interceve_entity_info.get_sensor_hexidx_info_map_lock_ForWrite();
					bool bClear = false;
					auto itor2 = hexidxmap.find(sensingmediaid);
					if (itor2 != hexidxmap.end())
					{
						HEXIDX_TIME_STAMP_INFO_HASHMAP & hexidx_time_stamp_info_list = itor2->second;
						auto hexidx_time_stamp_info_list_itor = hexidx_time_stamp_info_list.begin();
						while (hexidx_time_stamp_info_list_itor != hexidx_time_stamp_info_list.end())
						{
							const H3INDEX &hexidxclear = hexidx_time_stamp_info_list_itor->first;
							clearlist.push_back(hexidxclear);
							hexidx_time_stamp_info_list_itor->second.m_bHexidxValid = false;
							hexidx_time_stamp_info_list_itor++;
						}
#ifdef ENABLE_ERASE
						hexidxmap.erase(itor2);
#endif
						sensor_interceve_entity_info.m_sensor_hexidx_info_map_bchange = true;
						bClear = true;
					}
					if (hexidxmap.empty())
					{
						sensor_interceve_entity_info.m_sensor_property.m_bInterceveInfoValid = false;
						auto itor3 = sensor_interceve_entity_info.m_sensor_infos.find(sensingmediaid);
						if (itor3 != sensor_interceve_entity_info.m_sensor_infos.end())
						{
							tagSensorInfo &cur_sensorinfo = itor3->second;
							cur_sensorinfo.m_bValid = false;
						}
						{
							QWriteLocker _sensors_ulid_info_mutex_locker(&m_sensors_ulid_info_mutex);
							auto _sensors_ulid_info_itor = m_sensors_ulid_info.find(sensorulid);
							if (_sensors_ulid_info_itor != m_sensors_ulid_info.end())
							{
								_sensors_ulid_info_itor->second.m_bValid = false;
							}
						}
					}
					sensor_interceve_entity_info.set_sensor_hexidx_info_map_unlock();

					if (bClear)
					{
						SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP & entity_ulid_list = sensor_interceve_entity_info.get_sensor_entity_interceve_info_hashmap_lock_ForRead();
						auto entity_ulid_listitor = entity_ulid_list.begin();
						while (entity_ulid_listitor != entity_ulid_list.end())
						{
							if (entity_ulid_listitor->first.second == sensingmediaid)
							{
								entity_ulid_listitor->second.setInterceve_status(false);

								auto entity_ulid_list_valid_itor = std::find_if(entity_ulid_list.begin(),
									entity_ulid_list.end(),
									[&](SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP::value_type &vt)
								{
									return vt.first.first == entity_ulid_listitor->first.first && vt.second.m_bInterceveValid;
								});
								if (entity_ulid_list_valid_itor == entity_ulid_list.end())
								{
									m_pDijkstra->updateEdge(sensorulid, entity_ulid_listitor->first.first, DIJKSTRA_MAX_VALUE);
								}
							}
							entity_ulid_listitor++;
						}
						sensor_interceve_entity_info.set_sensor_entity_interceve_info_hashmap_unlock();
					}
					refershDataUpdate(E_PROCESS_TYPE_ALL);
				}
			}

			QList<QPair<TYPE_ULID, TYPE_ULID>> clearitemlist;
			clearitemlist << qMakePair(sensorulid, sensingmediaid);
			for (auto clearitem : clearlist)
			{
				clearHexidxULIDMap(clearitem, clearitemlist);
				clearResolutionHexidxULIDMap(clearitem, clearitemlist);
			}


			for (int index = 0; index < EVENTS_THREAD_NUM; index++)
			{
				std::unordered_map<TYPE_ULID, bool> & current_event_thread_deal_list = m_event_thread_deal_list[index];
				auto itor = current_event_thread_deal_list.find(sensorulid);
				if (itor != current_event_thread_deal_list.end())
				{
					itor->second = false;
					break;
				}
			}

			for (int index = 0; index < INTERACTIONS_THREAD_NUM; index++)
			{
				std::unordered_map<TYPE_ULID, bool> & current_interactions_thread_deal_list = m_interactions_thread_deal_list[index];
				auto itor = current_interactions_thread_deal_list.find(sensorulid);
				if (itor != current_interactions_thread_deal_list.end())
				{
					itor->second = false;
					break;
				}
			}
		}
	}

	void GaeactorEnvironment::updateEntityHexidxInfo(std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, std::unordered_map<UINT64, H3INDEX>, bool> > &_entity_hexidx_infos)
	{
		QWriteLocker _entity_hexidx_infos_mutexlocker(&m_entity_hexidx_infos_mutex);
		m_entity_hexidx_infos = std::move(_entity_hexidx_infos);
	}

	void GaeactorEnvironment::updateHexidxEntityInfo(std::unordered_map<H3INDEX, std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, bool> > > &_hexidx_entity_infos)
	{
		QWriteLocker _hexidx_entity_infos_mutexlocker(&m_hexidx_entity_infos_mutex);
		m_hexidx_entity_infos = std::move(_hexidx_entity_infos);
	}

	GaeactorEnvironment &GaeactorEnvironment::getInstance()
	{
		static GaeactorEnvironment gaeactorenv;
		return gaeactorenv;
	}

	void GaeactorEnvironment::registResolutions(const H3INDEX &h3Index, const TYPE_ULID & ulid_ele, const TYPE_ULID &sensingmediaid)
	{
		UINT64 hexidx_resolution = H3_GET_RESOLUTION(h3Index);

		appendResolutions(hexidx_resolution, ulid_ele, sensingmediaid);

		buildHexidxULIDMap(h3Index, ulid_ele, sensingmediaid);

		buildResolutionHexidxULIDMap(hexidx_resolution, h3Index, ulid_ele, sensingmediaid);
	}

	void GaeactorEnvironment::appendResolutions(const UINT64& hexidx_resolution, const TYPE_ULID & ulid_ele, const TYPE_ULID &sensingmediaid)
	{
		QWriteLocker _resolution_sensorsulid_mutex_locker(&m_resolution_sensorsulid_mutex);
		auto itor = m_resolution_sensorsulid.find(hexidx_resolution);
		if (itor != m_resolution_sensorsulid.end())
		{
			std::unordered_map<TYPE_ULID, QList<TYPE_ULID>> & ULIDlist = itor->second;
			auto itor2 = ULIDlist.find(ulid_ele);
			if (itor2 != ULIDlist.end())
			{
				QList<TYPE_ULID> & sensingmediaidlist = itor2->second;
				if (!sensingmediaidlist.contains(sensingmediaid))
				{
					sensingmediaidlist.push_back(sensingmediaid);
				}
			}
			else
			{
				ULIDlist.insert(std::make_pair(ulid_ele, QList<TYPE_ULID>() << sensingmediaid));
			}
		}
		else
		{
			std::unordered_map<TYPE_ULID, QList<TYPE_ULID>> ULIDlist;
			ULIDlist.insert(std::make_pair(ulid_ele, QList<TYPE_ULID>() << sensingmediaid));
			m_resolution_sensorsulid.insert(std::make_pair(hexidx_resolution, ULIDlist));
		}
	}

	void GaeactorEnvironment::appendEntitysUlid(const H3INDEX &h3Index, const TYPE_ULID &entityulid, bool isSensor, transdata_entityposinfo &eninfo)
	{
		//EASY_FUNCTION(profiler::colors::Brick)
		QWriteLocker _entitys_ulid_mutex_locker(&m_entitys_ulid_mutex);
		auto itor2 = m_entitys_ulid.find(entityulid);
		if (itor2 != m_entitys_ulid.end())
		{
			ENTITY_HEXIDX_INFO entityhexidxinfo(h3Index, eninfo, isSensor, true);
			itor2->second = std::move(entityhexidxinfo);
		}
		else
		{
			ENTITY_HEXIDX_INFO entityhexidxinfo(h3Index, eninfo, isSensor, true);
			m_entitys_ulid.insert(std::make_pair(entityulid, std::move(entityhexidxinfo)));
			LOG_PRINT_STR_EX("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++total entity size:" + QString::number(m_entitys_ulid.size()));
		}

		if (isSensor)
		{
            if (m_pDijkstra->appendNode(entityulid, LAT_LNG{ (double)(eninfo.PARAM_latitude / LON_LAT_ACCURACY),(double)(eninfo.PARAM_longitude / LON_LAT_ACCURACY) }))
			{
				//m_sensor_update_callback(entityulid, E_EVENT_MODE_ADD);
			}
		}
	}

	void GaeactorEnvironment::setEntitySensorProperty(const TYPE_ULID &sensorulid, bool bSensor)
	{
		QReadLocker _entitys_ulid_mutex_locker(&m_entitys_ulid_mutex);
		auto itor2 = m_entitys_ulid.find(sensorulid);
		if (itor2 != m_entitys_ulid.end())
		{
			itor2->second.m_isSensor = bSensor;

			if (bSensor)
			{
                if (m_pDijkstra->appendNode(sensorulid, LAT_LNG{ (double)(itor2->second.m_entityinfo.PARAM_latitude / LON_LAT_ACCURACY), (double)(itor2->second.m_entityinfo.PARAM_longitude / LON_LAT_ACCURACY) }))
				{
					//m_sensor_update_callback(sensorulid, E_EVENT_MODE_ADD);
				}
			}
		}
	}

	void GaeactorEnvironment::clearInvalidEntity()
	{
		//EASY_FUNCTION(profiler::colors::DarkTeal)
		QWriteLocker _entitys_ulid_mutex_locker(&m_entitys_ulid_mutex);
		auto entityuliditor = m_entitys_ulid.begin();
		while (entityuliditor != m_entitys_ulid.end())
		{
			if (!entityuliditor->second.m_bEntityValid)
			{
#ifdef ENABLE_ERASE
				entityuliditor = m_entitys_ulid.erase(entityuliditor);
				continue;
#endif
			}
			entityuliditor++;
		}
	}

	ENTITY_HEXIDX_INFO GaeactorEnvironment::getEntityHexidxInfo(const TYPE_ULID &sensorulid)
	{
		QReadLocker _entitys_ulid_mutex_locker(&m_entitys_ulid_mutex);
		return m_entitys_ulid[sensorulid];
	}

	QList<TYPE_ULID> GaeactorEnvironment::getEntitySensorPropertyList()
	{
		QList<TYPE_ULID> entitysensorlist;
		QReadLocker _entitys_ulid_mutex_locker(&m_entitys_ulid_mutex);
		auto entityuliditor = m_entitys_ulid.cbegin();
		while (entityuliditor != m_entitys_ulid.cend())
		{
			if (entityuliditor->second.m_isSensor)
			{
				entitysensorlist.push_back(entityuliditor->first);
			}
			entityuliditor++;
		}
		return entitysensorlist;
	}

	void GaeactorEnvironment::clearEntityInterceveInfo(const TYPE_ULID &entityulid, bool isSensorProperty)
	{
		if (!isSensorProperty)
		{
			//遍历该分别率下的所有ulid
			QReadLocker _sensors_ulid_hexidxs_mutex_locker(&m_sensors_ulid_hexidxs_mutex);
			auto sensorsulidhexidx_itor = m_sensors_ulid_hexidxs.begin();
			while (sensorsulidhexidx_itor != m_sensors_ulid_hexidxs.end())
			{
				SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = sensorsulidhexidx_itor->second;

				auto &entity_ulid_list = sensor_interceve_entity_info.get_sensor_entity_interceve_info_hashmap_lock_ForRead();

				auto entity_ulid_listitor = entity_ulid_list.begin();
				while (entity_ulid_listitor != entity_ulid_list.end())
				{
					if (entity_ulid_listitor->first.first == entityulid)
					{
						entity_ulid_listitor->second.setInterceve_status(false);
					}
					entity_ulid_listitor++;
				}
				sensor_interceve_entity_info.set_sensor_entity_interceve_info_hashmap_unlock();
				sensorsulidhexidx_itor++;
			}
			refershDataUpdate(E_PROCESS_TYPE_ALL);
		}
		else
		{
			QList<H3INDEX> clearlist;
			clearInvalidSensor(entityulid, entityulid, true, true);
		}
	}

	void GaeactorEnvironment::appendSensorsUlidHexidxsInfo(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const std::vector<transdata_param_seq_hexidx>  &_hexidxs, const transdata_sensorposinfo& _sensorinfo, const std::vector<transdata_param_seq_polygon> &_polygon, E_WAVE_SENSOR_SOURCE_TYPE eWaveSensorSourceType)
	{
		UINT32 silent_time = _sensorinfo.PARAM_wave_silent_time_gap;
		UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
		bool bNewAppend = false;
		bool bExistUpdate = false;

		{
			QWriteLocker _sensors_ulid_hexidxs_mutex_locker(&m_sensors_ulid_hexidxs_mutex);
			auto itor = m_sensors_ulid_hexidxs.find(sensorulid);
			if (itor != m_sensors_ulid_hexidxs.end())
			{
				if (!_hexidxs.empty())
				{
					{
						QWriteLocker _sensors_ulid_info_mutex_locker(&m_sensors_ulid_info_mutex);
						auto _sensors_ulid_info_itor = m_sensors_ulid_info.find(sensorulid);
						if (_sensors_ulid_info_itor != m_sensors_ulid_info.end())
						{
							_sensors_ulid_info_itor->second.m_bValid = true;
							_sensors_ulid_info_itor->second.m_sensorinfo = _sensorinfo;
							_sensors_ulid_info_itor->second.m_polygons = _polygon;
						}
					}

					SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = itor->second;
					SENSOR_HEXIDX_INFO_HASHMAP & hexidxmap = sensor_interceve_entity_info.get_sensor_hexidx_info_map_lock_ForWrite();

					auto itor2 = hexidxmap.find(sensingmediaid);
					if (itor2 != hexidxmap.end())
					{
						HEXIDX_TIME_STAMP_INFO_HASHMAP & hexidx_time_stamp_list = itor2->second;
						for (const auto &_hexidxs_item : _hexidxs)
						{
							auto hexidx_time_stamp_list_itor = hexidx_time_stamp_list.find(_hexidxs_item.PARAM_seq_hexidx_element);
							if (hexidx_time_stamp_list_itor != hexidx_time_stamp_list.end())
							{
								//std::cout <<sensingmediaid<< " update sensingmedia "<<fabs(currentTimeStamp - hexidx_time_stamp_list_itor->second.m_timestamp )<<std::endl;
								hexidx_time_stamp_list_itor->second.m_timestamp = currentTimeStamp;
								hexidx_time_stamp_list_itor->second.m_bHexidxValid = true;
								hexidx_time_stamp_list_itor->second.m_HexidxInfo = _hexidxs_item;
							}
							else
							{
								HEXIDX_TIME_STAMP_INFO hexidxstampinfo(currentTimeStamp, true, _hexidxs_item);
								hexidx_time_stamp_list.insert(std::make_pair(_hexidxs_item.PARAM_seq_hexidx_element, std::move(hexidxstampinfo)));
							}
						}
					}
					else
					{
						HEXIDX_TIME_STAMP_INFO_HASHMAP hexidx_time_stamp_list;
						for (const auto &_hexidxs_item : _hexidxs)
						{
							HEXIDX_TIME_STAMP_INFO hexidxstampinfo(currentTimeStamp, true, _hexidxs_item);
							hexidx_time_stamp_list.insert(std::make_pair(_hexidxs_item.PARAM_seq_hexidx_element, std::move(hexidxstampinfo)));
						}
						hexidxmap.insert(std::make_pair(sensingmediaid, std::move(hexidx_time_stamp_list)));
					}
					sensor_interceve_entity_info.m_sensor_hexidx_info_map_bchange = true;
					sensor_interceve_entity_info.set_sensor_hexidx_info_map_unlock();
					sensor_interceve_entity_info.m_sensor_property.m_bInterceveInfoValid = true;
					SENSOR_PROPERTY & sensor_property = sensor_interceve_entity_info.m_sensor_property;
					sensor_property.m_silent_time = silent_time;
					sensor_property.m_bClose = false;
					sensor_property.m_lastUpdateTimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
					sensor_property.m_eWaveSensorSourceType = eWaveSensorSourceType;
					auto itor3 = sensor_interceve_entity_info.m_sensor_infos.find(sensingmediaid);
					if (itor3 != sensor_interceve_entity_info.m_sensor_infos.end())
					{
						tagSensorInfo &cur_sensorinfo = itor3->second;
						cur_sensorinfo.m_bValid = true;
						cur_sensorinfo.m_sensorinfo = _sensorinfo;
						cur_sensorinfo.m_polygons = _polygon;
					}
					else
					{
						tagSensorInfo cur_sensorinfo;
						cur_sensorinfo.m_bValid = true;
						cur_sensorinfo.m_sensorinfo = std::move(_sensorinfo);
						cur_sensorinfo.m_polygons = std::move(_polygon);
						sensor_interceve_entity_info.m_sensor_infos.insert(std::make_pair(sensingmediaid, std::move(cur_sensorinfo)));
						LOG_PRINT_STR_EX("**************************************************************total sensor size:" + QString::number(m_sensors_ulid_hexidxs.size()) + " append sensingmedia " + QString::number(sensingmediaid));

					}

					bExistUpdate = true;
				}
			}
			else
			{
				if (!_hexidxs.empty())
				{
					{
						QWriteLocker _sensors_ulid_info_mutex_locker(&m_sensors_ulid_info_mutex);
						m_sensors_ulid_info.insert(std::make_pair(sensorulid, tagSensorInfo(true, _sensorinfo, _polygon)));
					}

					SENSOR_HEXIDX_INFO_HASHMAP hexidxmap;
					HEXIDX_TIME_STAMP_INFO_HASHMAP hexidx_time_stamp_list;
					for (const auto &_hexidxs_item : _hexidxs)
					{
						HEXIDX_TIME_STAMP_INFO hexidxstampinfo(currentTimeStamp, true, _hexidxs_item);
						hexidx_time_stamp_list.insert(std::make_pair(_hexidxs_item.PARAM_seq_hexidx_element, std::move(hexidxstampinfo)));
					}
					hexidxmap.insert(std::make_pair(sensingmediaid, std::move(hexidx_time_stamp_list)));
					tagSensorProperty sensorproperty(silent_time, false, stdutils::OriDateTime::getCurrentUTCTimeStampMSecs(), eWaveSensorSourceType, true);
					SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP _sensor_entity_interceve_info_hashmap;

					tagSensorInfo cur_sensorinfo;
					cur_sensorinfo.m_bValid = true;
					cur_sensorinfo.m_sensorinfo = std::move(_sensorinfo);
					cur_sensorinfo.m_polygons = std::move(_polygon);
					tagSensorInterceveEntityInfo sensor_interceve_entity_info(hexidxmap, \
						_sensor_entity_interceve_info_hashmap, \
						sensorproperty,
						sensingmediaid,
						cur_sensorinfo);
					m_sensors_ulid_hexidxs.insert(std::make_pair(sensorulid, std::move(sensor_interceve_entity_info)));


					LOG_PRINT_STR_EX("**************************************************************total sensor size:" + QString::number(m_sensors_ulid_hexidxs.size()) + " append sensingmedia " + QString::number(sensingmediaid));
					bNewAppend = true;
				}
			}
			refershDataUpdate(E_PROCESS_TYPE_ALL);
		}

		if (bNewAppend)
		{
			//分派改sensor 的event和interactions 相关的处理线程，该两个的线程一个线程只处理各自对应的部分sensor
			std::unordered_map<TYPE_ULID, bool> & current_event_thread_deal_list = m_event_thread_deal_list[m_event_thread_index];
			current_event_thread_deal_list.insert(std::make_pair(sensorulid, true));
			m_event_thread_index++;
			m_event_thread_index %= EVENTS_THREAD_NUM;

			std::unordered_map<TYPE_ULID, bool> & current_interactions_thread_deal_list = m_interactions_thread_deal_list[m_interactions_thread_index];
			current_interactions_thread_deal_list.insert(std::make_pair(sensorulid, true));
			m_interactions_thread_index++;
			m_interactions_thread_index %= INTERACTIONS_THREAD_NUM;
		}

		if (bExistUpdate)
		{
			for (int index = 0; index < EVENTS_THREAD_NUM; index++)
			{
				std::unordered_map<TYPE_ULID, bool> & current_event_thread_deal_list = m_event_thread_deal_list[index];
				auto itor = current_event_thread_deal_list.find(sensorulid);
				if (itor != current_event_thread_deal_list.end())
				{
					itor->second = true;
					break;
				}
			}

			for (int index = 0; index < INTERACTIONS_THREAD_NUM; index++)
			{
				std::unordered_map<TYPE_ULID, bool> & current_interactions_thread_deal_list = m_interactions_thread_deal_list[index];
				auto itor = current_interactions_thread_deal_list.find(sensorulid);
				if (itor != current_interactions_thread_deal_list.end())
				{
					itor->second = true;
					break;
				}
			}
		}

		if (!_hexidxs.empty())
		{
			if (eWaveSensorSourceType == E_WAVE_SENSOR_SOURCE_TYPE_SRC)
			{
				QWriteLocker _sensorslist_mutex_locker(&m_sensorslist_mutex);
				m_sensorslist.insert(std::make_pair(sensorulid, currentTimeStamp));
			}
		}
	}

	void GaeactorEnvironment::buildHexidxULIDMap(const H3INDEX &h3Index, const TYPE_ULID &ulid_ele, const TYPE_ULID &sensingmediaid)
	{
		QWriteLocker _hexidx_ulid_mutexlocker(&m_hexidx_ulid_mutex);
		auto itor = m_hexidx_ulid.find(h3Index);
		if (itor != m_hexidx_ulid.end())
		{
			std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>& ULIDlist = itor->second;
			auto ULIDlistitor = ULIDlist.find(ulid_ele);
			if (ULIDlistitor != ULIDlist.end())
			{
				std::get<0>(ULIDlistitor->second) = true;
				std::unordered_map<TYPE_ULID, bool> &sensingmediaidmap = std::get<1>(ULIDlistitor->second);
				auto sensingmediaidmapitor = sensingmediaidmap.find(sensingmediaid);
				if (sensingmediaidmapitor != sensingmediaidmap.end())
				{
					sensingmediaidmapitor->second = true;
				}
				else
				{
					sensingmediaidmap.insert(std::make_pair(sensingmediaid, true));
				}
			}
			else
			{
				std::unordered_map<TYPE_ULID, bool> sensingmediaidmap;
				sensingmediaidmap.insert(std::make_pair(sensingmediaid, true));
				ULIDlist.insert(std::make_pair(std::move(ulid_ele), std::make_tuple(true, std::move(sensingmediaidmap))));
			}
		}
		else
		{
			std::unordered_map<TYPE_ULID, bool> sensingmediaidmap;
			sensingmediaidmap.insert(std::make_pair(sensingmediaid, true));
			std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>> ULIDlist;
			ULIDlist.insert(std::make_pair(std::move(ulid_ele), std::make_tuple(true, std::move(sensingmediaidmap))));
			m_hexidx_ulid.insert(std::make_pair(h3Index, std::move(ULIDlist)));
		}
	}

	void GaeactorEnvironment::buildResolutionHexidxULIDMap(const UINT64 &hexidx_resolution, const H3INDEX &h3Index, const TYPE_ULID &ulid_ele, const TYPE_ULID &sensingmediaid)
	{
		QWriteLocker _hexidx_ulid_mutexlocker(&m_resolution_hexidx_ulid_mutex);
		auto itor = m_resolution_hexidx_ulid.find(hexidx_resolution);
		if (itor != m_resolution_hexidx_ulid.end())
		{
			std::unordered_map<H3INDEX, std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>> & hexidx_ULIDlist = itor->second;
			auto hexidx_ULIDlistitor = hexidx_ULIDlist.find(h3Index);
			if (hexidx_ULIDlistitor != hexidx_ULIDlist.end())
			{
				std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>& ULIDlist = hexidx_ULIDlistitor->second;
				auto ULIDlistitor = ULIDlist.find(ulid_ele);
				if (ULIDlistitor != ULIDlist.end())
				{
					std::get<0>(ULIDlistitor->second) = true;
					std::unordered_map<TYPE_ULID, bool> &sensingmediaidmap = std::get<1>(ULIDlistitor->second);
					auto sensingmediaidmapitor = sensingmediaidmap.find(sensingmediaid);
					if (sensingmediaidmapitor != sensingmediaidmap.end())
					{
						sensingmediaidmapitor->second = true;
					}
					else
					{
						sensingmediaidmap.insert(std::make_pair(sensingmediaid, true));
					}
				}
				else
				{
					std::unordered_map<TYPE_ULID, bool> sensingmediaidmap;
					sensingmediaidmap.insert(std::make_pair(sensingmediaid, true));
					ULIDlist.insert(std::make_pair(std::move(ulid_ele), std::make_tuple(true, std::move(sensingmediaidmap))));
				}
			}
			else
			{
				std::unordered_map<TYPE_ULID, bool> sensingmediaidmap;
				sensingmediaidmap.insert(std::make_pair(sensingmediaid, true));
				std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>> ULIDlist;
				ULIDlist.insert(std::make_pair(std::move(ulid_ele), std::make_tuple(true, std::move(sensingmediaidmap))));
				hexidx_ULIDlist.insert(std::make_pair(h3Index, std::move(ULIDlist)));
			}
		}
		else
		{
			//添加 分辨率 ---> hexidx ---> sensorid
			std::unordered_map<TYPE_ULID, bool> sensingmediainfo;
			sensingmediainfo.insert(std::make_pair(sensingmediaid, true));

			std::tuple<bool, std::unordered_map<TYPE_ULID, bool>> sensingmedialist;
			sensingmedialist = std::make_tuple(true, std::move(sensingmediainfo));

			std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>> ULIDlist;
			ULIDlist.insert(std::make_pair(std::move(ulid_ele), std::move(sensingmedialist)));

			std::unordered_map<H3INDEX, std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>> _hexidx_ulid;
			_hexidx_ulid.insert(std::make_pair(h3Index, std::move(ULIDlist)));

			m_resolution_hexidx_ulid.insert(std::make_pair(hexidx_resolution, std::move(_hexidx_ulid)));
		}
	}

	void GaeactorEnvironment::clearHexidxULIDMap(const H3INDEX &h3Index, const QList<QPair<TYPE_ULID, TYPE_ULID> > &clearitemlist)
	{
		QReadLocker _hexidx_ulid_mutexlocker(&m_hexidx_ulid_mutex);
		auto itor3 = m_hexidx_ulid.find(h3Index);
		if (itor3 != m_hexidx_ulid.end())
		{
			std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>& ULIDlist = itor3->second;
			for (auto clearitem : clearitemlist)
			{
				auto ULIDlistitor = ULIDlist.find(clearitem.first);
				if (ULIDlistitor != ULIDlist.end())
				{
					bool &bValid = std::get<0>(ULIDlistitor->second);
					std::unordered_map<TYPE_ULID, bool> &sensingmediaidmap = std::get<1>(ULIDlistitor->second);
					auto sensingmediaidmapitor = sensingmediaidmap.find(clearitem.second);
					if (sensingmediaidmapitor != sensingmediaidmap.end())
					{
						sensingmediaidmapitor->second = false;

						auto sensingmediaidmapitor2 = std::find_if(sensingmediaidmap.begin(),
							sensingmediaidmap.end(),
							[&](const std::unordered_map<TYPE_ULID, bool>::value_type& vt)
						{
							return vt.second;
						});
						bValid = (sensingmediaidmapitor2 != sensingmediaidmap.end()) ? true : false;
					}
				}
			}
		}
	}

	void GaeactorEnvironment::clearResolutionHexidxULIDMap(const H3INDEX &h3Index, const QList<QPair<TYPE_ULID, TYPE_ULID>> &sensorclearitemlist)
	{
		UINT64 hexidx_resolution = H3_GET_RESOLUTION(h3Index);

		QList<QPair<TYPE_ULID, TYPE_ULID>> removesensorclearitemlist;
		QReadLocker _hexidx_ulid_mutexlocker(&m_resolution_hexidx_ulid_mutex);
		auto itor = m_resolution_hexidx_ulid.find(hexidx_resolution);
		if (itor != m_resolution_hexidx_ulid.end())
		{
			std::unordered_map<H3INDEX, std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>> & hexidx_ULIDlist = itor->second;
			auto hexidx_ULIDlistitor = hexidx_ULIDlist.find(h3Index);
			if (hexidx_ULIDlistitor != hexidx_ULIDlist.end())
			{
				std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>& ULIDlist = hexidx_ULIDlistitor->second;
				for (auto clearsensoritem : sensorclearitemlist)
				{
					auto ULIDlistitor = ULIDlist.find(clearsensoritem.first);
					if (ULIDlistitor != ULIDlist.end())
					{
						//置 sensorid 为无效
						bool &bValid = std::get<0>(ULIDlistitor->second);
						std::unordered_map<TYPE_ULID, bool> &sensingmediaidmap = std::get<1>(ULIDlistitor->second);
						auto sensingmediaidmapitor = sensingmediaidmap.find(clearsensoritem.second);
						if (sensingmediaidmapitor != sensingmediaidmap.end())
						{
							sensingmediaidmapitor->second = false;
							removesensorclearitemlist.push_back(clearsensoritem);
							auto sensingmediaidmapitor2 = std::find_if(sensingmediaidmap.begin(),
								sensingmediaidmap.end(),
								[&](const std::unordered_map<TYPE_ULID, bool>::value_type& vt)
							{
								return vt.second;
							});
							bValid = (sensingmediaidmapitor2 != sensingmediaidmap.end()) ? true : false;
						}
					}
				}
			}
		}

		//清理指定 操作指定的sensorulid 的干涉列表 中对应使用的hexidx的干涉项，置为无效
		for (auto sensor_sensingmedia_id_pair : removesensorclearitemlist)
		{
			//操作指定的sensorulid 的干涉列表，可以直接往 干涉列表中添加

			//EASY_FUNCTION(profiler::colors::Olive)
			QReadLocker _sensors_ulid_hexidxs_mutex_locker(&m_sensors_ulid_hexidxs_mutex);
			auto _sensors_ulid_hexidxs_itor = m_sensors_ulid_hexidxs.find(sensor_sensingmedia_id_pair.first);
			if (_sensors_ulid_hexidxs_itor != m_sensors_ulid_hexidxs.end())
			{
				SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = _sensors_ulid_hexidxs_itor->second;
				if (sensor_interceve_entity_info.m_sensor_property.m_bInterceveInfoValid)
				{
					//波为非静默状态才处理对应的感知
					if (!sensor_interceve_entity_info.m_sensor_property.m_bClose)
					{

						SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP & entity_ulid_list = sensor_interceve_entity_info.get_sensor_entity_interceve_info_hashmap_lock_ForRead();
						//不存在，则在干涉实体的ULID列表中检查清理
						auto entity_ulid_listitor = std::find_if(entity_ulid_list.begin(),
							entity_ulid_list.end(),
							[&](const SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP::value_type &vt) {
							return vt.first.second == sensor_sensingmedia_id_pair.second && vt.second.m_bInterceveValid && vt.second.m_resolutionhexidx == h3Index;
						});
						if (entity_ulid_listitor != entity_ulid_list.end())
						{
							entity_ulid_listitor->second.setInterceve_status(false);

							auto entity_ulid_list_valid_itor = std::find_if(entity_ulid_list.begin(),
								entity_ulid_list.end(),
								[&](const SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP::value_type &vt)
							{
								return vt.first.first == entity_ulid_listitor->first.first && vt.second.m_bInterceveValid;
							});
							if (entity_ulid_list_valid_itor == entity_ulid_list.end())
							{
								m_pDijkstra->updateEdge(sensor_sensingmedia_id_pair.first, entity_ulid_listitor->first.first, DIJKSTRA_MAX_VALUE);
							}

						}
						sensor_interceve_entity_info.set_sensor_entity_interceve_info_hashmap_unlock();
					}
					refershDataUpdate(E_PROCESS_TYPE_ALL);
				}
			}
		}
	}

	void GaeactorEnvironment::releaseHexidxULIDMap()
	{
		QWriteLocker _hexidx_ulid_mutexlocker(&m_hexidx_ulid_mutex);
		auto hexidx_ulid_itor = m_hexidx_ulid.begin();
		while (hexidx_ulid_itor != m_hexidx_ulid.end())
		{
			std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>& ULIDlist = hexidx_ulid_itor->second;
			auto ULIDlist_itor = ULIDlist.begin();
			while (ULIDlist_itor != ULIDlist.end())
			{
				bool &bValid = std::get<0>(ULIDlist_itor->second);
				std::unordered_map<TYPE_ULID, bool> &sensingmediaidmap = std::get<1>(ULIDlist_itor->second);
				auto sensingmediaidmapitor = std::find_if(sensingmediaidmap.begin(),
					sensingmediaidmap.end(),
					[&](const std::unordered_map<TYPE_ULID, bool>::value_type & vt) {
					return vt.second;
				});
				bValid = (sensingmediaidmapitor == sensingmediaidmap.end()) ? false : true;
				//            auto sensingmediaidmapitor = sensingmediaidmap.begin();
				//            while(sensingmediaidmapitor != sensingmediaidmap.end())
				//            {
				//                if(!sensingmediaidmapitor->second)
				//                {
				//#ifdef ENABLE_ERASE
				//                    sensingmediaidmapitor = sensingmediaidmap.erase(sensingmediaidmapitor);
				//                    continue;
				//#endif
				//                }
				//                else
				//                {
				//                    bValid = true;
				//                }
				//                sensingmediaidmapitor++;
				//            }
				if (!bValid)
				{
#ifdef ENABLE_ERASE
					ULIDlist_itor = ULIDlist.erase(ULIDlist_itor);
					continue;
#endif
				}
				ULIDlist_itor++;
			}
			if (ULIDlist.empty())
			{
#ifdef ENABLE_ERASE
				hexidx_ulid_itor = m_hexidx_ulid.erase(hexidx_ulid_itor);
				continue;
#endif
			}
			hexidx_ulid_itor++;
		}
	}

	void GaeactorEnvironment::releaseResolutionHexidxULIDMap()
	{
		QWriteLocker _hexidx_ulid_mutexlocker(&m_resolution_hexidx_ulid_mutex);
		auto m_resolution_hexidx_ulid_itor = m_resolution_hexidx_ulid.begin();
		while (m_resolution_hexidx_ulid_itor != m_resolution_hexidx_ulid.end())
		{
			std::unordered_map<H3INDEX, std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>> & hexidx_ULIDlist = m_resolution_hexidx_ulid_itor->second;
			auto hexidx_ulid_itor = hexidx_ULIDlist.begin();
			while (hexidx_ulid_itor != hexidx_ULIDlist.end())
			{
				std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>& ULIDlist = hexidx_ulid_itor->second;
				auto ULIDlist_itor = ULIDlist.begin();
				while (ULIDlist_itor != ULIDlist.end())
				{
					bool &bValid = std::get<0>(ULIDlist_itor->second);
					std::unordered_map<TYPE_ULID, bool> &sensingmediaidmap = std::get<1>(ULIDlist_itor->second);
					auto sensingmediaidmapitor = std::find_if(sensingmediaidmap.begin(),
						sensingmediaidmap.end(),
						[&](const std::unordered_map<TYPE_ULID, bool>::value_type & vt) {
						return vt.second;
					});
					bValid = (sensingmediaidmapitor == sensingmediaidmap.end()) ? false : true;
					//            auto sensingmediaidmapitor = sensingmediaidmap.begin();
					//            while(sensingmediaidmapitor != sensingmediaidmap.end())
					//            {
					//                if(!sensingmediaidmapitor->second)
					//                {
					//#ifdef ENABLE_ERASE
					//                    sensingmediaidmapitor = sensingmediaidmap.erase(sensingmediaidmapitor);
					//                    continue;
					//#endif
					//                }
					//                else
					//                {
					//                    bValid = true;
					//                }
					//                sensingmediaidmapitor++;
					//            }
					if (!bValid)
					{
#ifdef ENABLE_ERASE
						ULIDlist_itor = ULIDlist.erase(ULIDlist_itor);
						continue;
#endif
					}
					ULIDlist_itor++;
				}
				if (ULIDlist.empty())
				{
#ifdef ENABLE_ERASE
					hexidx_ulid_itor = hexidx_ULIDlist.erase(hexidx_ulid_itor);
					continue;
#endif
				}
				hexidx_ulid_itor++;
			}
			if (hexidx_ULIDlist.empty())
			{
#ifdef ENABLE_ERASE
				m_resolution_hexidx_ulid_itor = m_resolution_hexidx_ulid.erase(m_resolution_hexidx_ulid_itor);
				continue;
#endif
			}
			m_resolution_hexidx_ulid_itor++;
		}
	}


	CLEAR_RELATION_INFO_LIST GaeactorEnvironment::refershSensorSilenceStatusclearSensorEntityRelation()
	{
		//EASY_FUNCTION(profiler::colors::DarkTeal)
		CLEAR_RELATION_INFO_LIST clear_relation_info_list;
		INTERSECTION_HEXIDX_LIST &intersectionHexidx = std::get<0>(clear_relation_info_list);
		QList<QPair<TYPE_ULID, TYPE_ULID>> &sensorulidlist = std::get<1>(clear_relation_info_list);

		QList<TYPE_ULID> clearsensorlist;
		clearInvalidEntity();
		std::unordered_map<H3INDEX, QList<QPair<TYPE_ULID, TYPE_ULID>>> clearlist;

#ifdef ENABLE_ERASE
		std::list<QPair<TYPE_ULID, TYPE_ULID>> clear_interation_list;
#endif
		{

#ifdef ENABLE_ERASE
			QWriteLocker _sensors_ulid_hexidxs_mutex_locker(&m_sensors_ulid_hexidxs_mutex);
#else
			QReadLocker _sensors_ulid_hexidxs_mutex_locker(&m_sensors_ulid_hexidxs_mutex);
#endif

			UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
			bool bNeedRefershData = false;
			auto sensors_uliditor = m_sensors_ulid_hexidxs.begin();
			while (sensors_uliditor != m_sensors_ulid_hexidxs.end())
			{
				const TYPE_ULID &sensorulid = sensors_uliditor->first;
				SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = sensors_uliditor->second;

				if (sensor_interceve_entity_info.m_sensor_property.m_bInterceveInfoValid)
				{
					///////////////////////////////////////////////////////////////////////////////////////////////
					//设置静默超时 状态置为无效
					//刷新波感知域的静默状态，根据静默间隙来回切换静默状态，当状态由非静默变为静默时，清理在非静默状态时，波感知检测并放入的干涉实体ULID哈希列表的实体
					std::unordered_map<H3INDEX, uint> silenceclearlist;

					SENSOR_HEXIDX_INFO_HASHMAP & hexidxmap = sensor_interceve_entity_info.get_sensor_hexidx_info_map_lock_ForWrite();
					SENSOR_HEXIDX_INFO_HASHMAP::iterator hexidxmap_itor = hexidxmap.begin();
					while (hexidxmap_itor != hexidxmap.end())
					{
						const TYPE_ULID &sensingmediaid = hexidxmap_itor->first;
						HEXIDX_TIME_STAMP_INFO_HASHMAP & hexidx_time_stamp_info_list = hexidxmap_itor->second;
						auto hexidx_time_stamp_info_list_itor = hexidx_time_stamp_info_list.begin();
						while (hexidx_time_stamp_info_list_itor != hexidx_time_stamp_info_list.end())
						{
							const H3INDEX & hexidx = hexidx_time_stamp_info_list_itor->first;
							if (!hexidx_time_stamp_info_list_itor->second.m_bHexidxValid)
							{
								//                            if(!sensorulidlist.contains(sensorulid))
								//                            {
								//                                sensorulidlist.push_back(sensorulid);
								//                            }
#ifdef ENABLE_ERASE
								bNeedRefershData = true;
								hexidx_time_stamp_info_list_itor = hexidx_time_stamp_info_list.erase(hexidx_time_stamp_info_list_itor);
#else
								hexidx_time_stamp_info_list_itor++;
#endif
								//                            sensor_interceve_entity_info.m_sensor_hexidx_info_map_bchange = true;
								continue;
							}

							if (sensor_interceve_entity_info.m_sensor_property.m_silent_time == 0)
							{
								hexidx_time_stamp_info_list_itor->second.m_timestamp = currentTimeStamp;
							}
							else
							{
								if (fabs(currentTimeStamp - hexidx_time_stamp_info_list_itor->second.m_timestamp) > sensor_interceve_entity_info.m_sensor_property.m_silent_time)
								{
									hexidx_time_stamp_info_list_itor->second.m_bHexidxValid = !hexidx_time_stamp_info_list_itor->second.m_bHexidxValid;
									//std::cout<<hexidxmap_itor->first<<" ++++++++set hexidx invalid "<<fabs(currentTimeStamp - hexidx_time_stamp_info_list_itor->second.m_timestamp)<<std::endl;
									bNeedRefershData = true;
								}
								if (!hexidx_time_stamp_info_list_itor->second.m_bHexidxValid)
								{
									if (!sensorulidlist.contains(qMakePair(sensorulid, sensingmediaid)))
									{
										sensorulidlist.push_back(std::move(qMakePair(sensorulid, sensingmediaid)));
									}

									auto clearitor = clearlist.find(hexidx);
									if (clearitor != clearlist.end())
									{
										clearitor->second.push_back(qMakePair(sensorulid, sensingmediaid));
									}
									else
									{
										clearlist.insert(std::make_pair(hexidx, std::move(QList<QPair<TYPE_ULID, TYPE_ULID>>() << qMakePair(sensorulid, sensingmediaid))));
									}
									auto silenceclearlistitor = silenceclearlist.find(hexidx);
									if (silenceclearlistitor != silenceclearlist.end())
									{
										silenceclearlistitor->second = hexidx_time_stamp_info_list_itor->second.m_timestamp;
									}
									else
									{
										silenceclearlist.insert(std::make_pair(hexidx, hexidx_time_stamp_info_list_itor->second.m_timestamp));
									}
#ifdef ENABLE_ERASE
									bNeedRefershData = true;
									hexidx_time_stamp_info_list_itor = hexidx_time_stamp_info_list.erase(hexidx_time_stamp_info_list_itor);
									continue;
#endif
									sensor_interceve_entity_info.m_sensor_hexidx_info_map_bchange = true;
								}
							}
							hexidx_time_stamp_info_list_itor++;
						}
						hexidxmap_itor++;
					}
					sensor_interceve_entity_info.set_sensor_hexidx_info_map_unlock();

					/////////////////////////////////////////////////////////////////////////////////////////////////
					/// 清理场与场实体形成的感知交互
					SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP & entity_ulid_list = sensor_interceve_entity_info.get_sensor_entity_interceve_info_hashmap_lock_ForWrite();

					auto entity_ulid_listitor = entity_ulid_list.begin();
					while (entity_ulid_listitor != entity_ulid_list.end())
					{
						if (!entity_ulid_listitor->second.m_bInterceveValid)
						{
							entity_ulid_listitor++;
							continue;
						}
						const QPair<TYPE_ULID, TYPE_ULID>& entity_sensingmedia_id_pair = entity_ulid_listitor->first;
#ifdef CLEAR_SENSOR_TO_SENSOR_INTERSECTION
						if ((sensor_interceve_entity_info.m_sensor_property.m_eWaveSensorSourceType == E_WAVE_SENSOR_SOURCE_TYPE_SRC) && isEntitySensorProperty(entityulid))
						{
							//                        LOG_PRINT_STR_EX("need clear sensor interceve")
							entity_ulid_listitor->second.setInterceve_status(false);
							bNeedRefershData = true;
						}
#endif
						if (silenceclearlist.find(entity_ulid_listitor->second.m_resolutionhexidx) != silenceclearlist.end() && entity_ulid_listitor->second.m_bInterceveValid)
						{
							entity_ulid_listitor->second.setInterceve_status(false);

							auto entity_ulid_list_valid_itor = std::find_if(entity_ulid_list.begin(),
								entity_ulid_list.end(),
								[&](SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP::value_type &vt)
							{
								return vt.first.first == entity_sensingmedia_id_pair.first && vt.first.second == entity_sensingmedia_id_pair.second && vt.second.m_bInterceveValid;
							});
							if (entity_ulid_list_valid_itor == entity_ulid_list.end())
							{
								m_pDijkstra->updateEdge(sensorulid, entity_sensingmedia_id_pair.first, DIJKSTRA_MAX_VALUE);
							}
							bNeedRefershData = true;
						}
						//clear invalid hexidx by over timestap
						if (!entity_ulid_listitor->second.m_bInterceveValid)
						{
							intersectionHexidx.emplace_back(std::make_tuple(entity_sensingmedia_id_pair, sensorulid, entity_ulid_listitor->second.m_resolutionhexidx));
#ifdef ENABLE_ERASE
							bNeedRefershData = true;
							QPair<TYPE_ULID, TYPE_ULID> eventtile = qMakePair(sensorulid, entity_sensingmedia_id_pair.first);
							clear_interation_list.emplace_back(std::move(eventtile));
							entity_ulid_listitor = entity_ulid_list.erase(entity_ulid_listitor);
							continue;
#endif
						}
						entity_ulid_listitor++;
					}

					sensor_interceve_entity_info.set_sensor_entity_interceve_info_hashmap_unlock();

					/////////////////////////////////////////////////////////////////////////////////////////////////
				}
				else
				{
					SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP & entity_ulid_list = sensor_interceve_entity_info.get_sensor_entity_interceve_info_hashmap_lock_ForRead();

					auto entity_ulid_listitor = entity_ulid_list.begin();
					while (entity_ulid_listitor != entity_ulid_list.end())
					{
						const QPair<TYPE_ULID, TYPE_ULID>& entity_sensingmedia_id_pair = entity_ulid_listitor->first;
						intersectionHexidx.emplace_back(std::make_tuple(entity_sensingmedia_id_pair, sensorulid, entity_ulid_listitor->second.m_resolutionhexidx));
						entity_ulid_listitor++;
					}
					sensor_interceve_entity_info.set_sensor_entity_interceve_info_hashmap_unlock();
					QList<QPair<TYPE_ULID, TYPE_ULID>> &sensorulidlist = std::get<1>(clear_relation_info_list);
					SENSOR_HEXIDX_INFO_HASHMAP & hexidxmap = sensor_interceve_entity_info.get_sensor_hexidx_info_map_lock_ForRead();
					SENSOR_HEXIDX_INFO_HASHMAP::iterator hexidxmap_itor = hexidxmap.begin();
					while (hexidxmap_itor != hexidxmap.end())
					{
						const TYPE_ULID &sensingmediaid = hexidxmap_itor->first;
						if (!sensorulidlist.contains(qMakePair(sensorulid, sensingmediaid)))
						{
							sensorulidlist.push_back(std::move(qMakePair(sensorulid, sensingmediaid)));
						}
						hexidxmap_itor++;
					}
					sensor_interceve_entity_info.set_sensor_hexidx_info_map_unlock();


					if (sensor_interceve_entity_info.m_sensor_property.m_eWaveSensorSourceType == E_WAVE_SENSOR_SOURCE_TYPE_SRC)
					{
						if (!clearsensorlist.contains(sensorulid))
						{
							clearsensorlist.push_back(std::move(sensorulid));
						}
					}
#ifdef ENABLE_ERASE
					sensors_uliditor = m_sensors_ulid_hexidxs.erase(sensors_uliditor);
					continue;
#endif
				}
				sensors_uliditor++;
			}
			if (bNeedRefershData)
			{
				refershDataUpdate((E_PROCESS_TYPE)(E_PROCESS_TYPE_EVENTS | E_PROCESS_TYPE_INTERACTIONS));
			}
		}
		//#ifdef ENABLE_ERASE
		//        auto clear_interation_list_itor = clear_interation_list.begin();
		//        while (clear_interation_list_itor != clear_interation_list.end())
		//        {
		//            const QPair<TYPE_ULID,TYPE_ULID> &eventtile = *clear_interation_list_itor;

		//            {
		//                QWriteLocker _clear_events_mutex_locker(&m_clear_events_mutex);

		//                auto _clear_events_itor = m_clear_events.find(eventtile);
		//                if(_clear_events_itor != m_clear_events.end())
		//                {
		//                    _clear_events_itor->second = true;
		//                }
		//                else
		//                {
		//                    m_clear_events.insert(std::make_pair(eventtile, true));
		//                }
		//            }
		//            {

		//                QWriteLocker _clear_interactions_mutex_locker(&m_clear_interactions_mutex);

		//                auto _clear_interactions_itor = m_clear_interactions.find(eventtile);
		//                if(_clear_interactions_itor != m_clear_interactions.end())
		//                {
		//                    _clear_interactions_itor->second = true;
		//                }
		//                else
		//                {
		//                    m_clear_interactions.insert(std::make_pair(eventtile, true));
		//                }
		//            }

		//            clear_interation_list_itor++;
		//        }
		//#endif

		for (auto clearsensorlistitem : clearsensorlist)
		{
			QWriteLocker _sensorslist_mutex_locker(&m_sensorslist_mutex);
			auto _sensorslistitor = m_sensorslist.find((clearsensorlistitem));
			if (_sensorslistitor != m_sensorslist.end())
			{
				m_sensorslist.erase(_sensorslistitor);
			}
		}

		for (auto clearlist_itor = clearlist.begin(); clearlist_itor != clearlist.end(); clearlist_itor++)
		{
			const H3INDEX & clearhexidx = clearlist_itor->first;
			QList<QPair<TYPE_ULID, TYPE_ULID>> clearitemlist = clearlist_itor->second;

			clearHexidxULIDMap(clearhexidx, clearitemlist);
			clearResolutionHexidxULIDMap(clearhexidx, clearitemlist);
		}

#ifdef ENABLE_ERASE
		releaseHexidxULIDMap();
		releaseResolutionHexidxULIDMap();
#endif

		return clear_relation_info_list;
	}


	void GaeactorEnvironment::updateInterveneInfo(const TYPE_ULID &ulid, \
		const std::unordered_map<UINT64, H3INDEX>& resolution_target_hexidxs, \
		E_WAVE_SENSOR_SOURCE_TYPE eWaveSensorSourceType,
		const TYPE_ULID &entityulid,
		const H3INDEX &h3Indexsrc,
		INTERSECTION_HEXIDX_LIST& intersectionHexidx)
	{
		//EASY_FUNCTION(profiler::colors::DarkGreen)

		std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, bool> new_interactions_list;


		auto AppendSensorInteractionsInfo = [&](const QPair<TYPE_ULID, TYPE_ULID>& sensor_sensingmedia_id_pair, const H3INDEX& reshexidx) {
			//操作指定的sensorulid 的干涉列表，可以直接往 干涉列表中添加
			//EASY_FUNCTION(profiler::colors::Olive)
			QReadLocker _sensors_ulid_hexidxs_mutex_locker(&m_sensors_ulid_hexidxs_mutex);
			auto _sensors_ulid_hexidxs_itor = m_sensors_ulid_hexidxs.find(sensor_sensingmedia_id_pair.first);
			if (_sensors_ulid_hexidxs_itor != m_sensors_ulid_hexidxs.end())
			{
				SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = _sensors_ulid_hexidxs_itor->second;
				if (sensor_interceve_entity_info.m_sensor_property.m_bInterceveInfoValid)
				{
					if (eWaveSensorSourceType != sensor_interceve_entity_info.m_sensor_property.m_eWaveSensorSourceType)
					{
						return;
					}

					//波为非静默状态才处理对应的感知
					if (!sensor_interceve_entity_info.m_sensor_property.m_bClose)
					{
						QPair<TYPE_ULID, TYPE_ULID> entity_sensingmedia_id_pair = qMakePair(entityulid, sensor_sensingmedia_id_pair.second);
						SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP & entity_ulid_list = sensor_interceve_entity_info.get_sensor_entity_interceve_info_hashmap_lock_ForWrite();
						auto entity_ulid_listitor = entity_ulid_list.find(entity_sensingmedia_id_pair);
						if (entity_ulid_listitor != entity_ulid_list.end())
						{
							entity_ulid_listitor->second.m_resolutionhexidx = reshexidx;
							entity_ulid_listitor->second.m_srchexidx = h3Indexsrc;
							entity_ulid_listitor->second.setInterceve_status(true);
							m_pDijkstra->updateEdge(sensor_sensingmedia_id_pair.first, entityulid, 1);
						}
						else
						{
							SENSOR_ENTITY_INTERCEVE_INFO sensorrntityinterceveinfo(reshexidx, h3Indexsrc, true);
							entity_ulid_list.insert(std::make_pair(entity_sensingmedia_id_pair, std::move(sensorrntityinterceveinfo)));
							m_pDijkstra->updateEdge(sensor_sensingmedia_id_pair.first, entityulid, 1);
						}
						sensor_interceve_entity_info.set_sensor_entity_interceve_info_hashmap_unlock();
						if (new_interactions_list.find(sensor_sensingmedia_id_pair) == new_interactions_list.end())
						{
							new_interactions_list.insert(std::make_pair(sensor_sensingmedia_id_pair, true));
						}
#ifdef GENERATE_INTERSECTION_HEDIDX_LIST
						intersectionHexidx.push_back(std::make_tuple(entityulid, sensorulid, reshexidx));
#endif
					}
					refershDataUpdate(E_PROCESS_TYPE_ALL);
				}
			}
		};

		for (const auto &item : resolution_target_hexidxs)
		{
			UINT64 resolution = item.first;
			H3INDEX reshexidx = item.second;

			{
				std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>> sensorsulidlist = getSensorsResolutionHexidxs(resolution, reshexidx);
				//根据分辨率 ，查找对应分辨率下的ulid，进而得到ulid的所有的hexidx
				//        QList<TYPE_ULID> sensorsulidlist = getResolutionsHexidxs(resolution);
				//遍历该分别率下的所有ulid
				auto sensorsulidlist_itor = sensorsulidlist.begin();
				while (sensorsulidlist_itor != sensorsulidlist.end())
				{
					const TYPE_ULID& sensorulid = sensorsulidlist_itor->first;
					const bool &bSensorValid = std::get<0>(sensorsulidlist_itor->second);
					const std::unordered_map<TYPE_ULID, bool>& sensingmedialist = std::get<1>(sensorsulidlist_itor->second);
					if (!bSensorValid)
					{
						sensorsulidlist_itor++;
						continue;
					}
					if (ulid == sensorulid)
					{
						sensorsulidlist_itor++;
						continue;
					}

					auto sensingmedialist_itor = sensingmedialist.begin();
					while (sensingmedialist_itor != sensingmedialist.cend())
					{
						auto &sensingmediaid = sensingmedialist_itor->first;
						auto sensor_sensingmedia_id_pair = qMakePair(sensorulid, sensingmediaid);

						auto &sensingmediaValid = sensingmedialist_itor->second;
						if (sensingmediaValid)
						{
							AppendSensorInteractionsInfo(sensor_sensingmedia_id_pair, reshexidx);
						}
						sensingmedialist_itor++;
					}

					sensorsulidlist_itor++;
				}
			}
		}


		auto clearSensorInteractionsInfo = [&](const QPair<TYPE_ULID, TYPE_ULID>& sensor_sensingmedia_id_pair, const TYPE_ULID& entityulid) {
			//操作指定的sensorulid 的干涉列表，可以直接往 干涉列表中添加
			//EASY_FUNCTION(profiler::colors::CreamWhite)
			QReadLocker _sensors_ulid_hexidxs_mutex_locker(&m_sensors_ulid_hexidxs_mutex);
			auto _sensors_ulid_hexidxs_itor = m_sensors_ulid_hexidxs.find(sensor_sensingmedia_id_pair.first);
			if (_sensors_ulid_hexidxs_itor != m_sensors_ulid_hexidxs.end())
			{
				SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = _sensors_ulid_hexidxs_itor->second;
				if (sensor_interceve_entity_info.m_sensor_property.m_bInterceveInfoValid)
				{
					if (eWaveSensorSourceType != sensor_interceve_entity_info.m_sensor_property.m_eWaveSensorSourceType)
					{
						return;
					}

					SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP & entity_ulid_list = sensor_interceve_entity_info.get_sensor_entity_interceve_info_hashmap_lock_ForRead();
					//不存在，则在干涉实体的ULID列表中检查清理

					QPair<TYPE_ULID, TYPE_ULID> entity_sensingmedia_id_pair = qMakePair(entityulid, sensor_sensingmedia_id_pair.second);
					auto entity_ulid_listitor = entity_ulid_list.find(entity_sensingmedia_id_pair);
					if (entity_ulid_listitor != entity_ulid_list.end())
					{
						entity_ulid_listitor->second.setInterceve_status(false);

						auto entity_ulid_list_valid_itor = std::find_if(entity_ulid_list.begin(),
							entity_ulid_list.end(),
							[&](SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP::value_type &vt)
						{
							return vt.first.first == entityulid && vt.second.m_bInterceveValid;
						});
						if (entity_ulid_list_valid_itor == entity_ulid_list.end())
						{
							m_pDijkstra->updateEdge(sensor_sensingmedia_id_pair.first, entityulid, DIJKSTRA_MAX_VALUE);
						}
					}

					sensor_interceve_entity_info.set_sensor_entity_interceve_info_hashmap_unlock();

					refershDataUpdate(E_PROCESS_TYPE_ALL);
				}
			}
		};

		{
			//根据旧的实体被场干涉的列表与当前新的实体被干涉的场的列表做对比，得出哪些场未感知该实体了，对之前感知干涉了的 当前未感知干涉的场 做干涉状态清理
			std::vector<QPair<TYPE_ULID, TYPE_ULID>>  exist_old_interactions_list;
			{
				QWriteLocker _entity_interactions_list_mutex_locker(&m_entity_interactions_list_mutex);
				auto m_entity_interactions_list_itor = m_entity_interactions_list.find(entityulid);
				if (m_entity_interactions_list_itor == m_entity_interactions_list.end())
				{
					m_entity_interactions_list.insert(std::make_pair(entityulid, std::move(new_interactions_list)));
				}
				else
				{
					std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, bool> &old_interactions_list = m_entity_interactions_list_itor->second;

					//遍历旧的实体与场碰撞列表,旧的存在 新的不存在，则记录并清理【由场驱动的检测 则只清理与场本身相关的】
					auto old_interactions_list_itor = old_interactions_list.begin();
					while (old_interactions_list_itor != old_interactions_list.end())
					{
						const QPair<TYPE_ULID, TYPE_ULID>& sensor_sensing_id_pair = old_interactions_list_itor->first;
						if (new_interactions_list.find(sensor_sensing_id_pair) != new_interactions_list.end())
						{
							old_interactions_list_itor++;
							continue;
						}
						exist_old_interactions_list.push_back(sensor_sensing_id_pair);
						old_interactions_list_itor = old_interactions_list.erase(old_interactions_list_itor);
					}

					//遍历新的实体与场碰撞列表,新的存在，旧的不存在则加入
					auto new_interactions_list_itor = new_interactions_list.begin();
					while (new_interactions_list_itor != new_interactions_list.end())
					{
						const QPair<TYPE_ULID, TYPE_ULID>& sensor_sensing_id_pair = new_interactions_list_itor->first;
						if (old_interactions_list.find(sensor_sensing_id_pair) == old_interactions_list.end())
						{
							old_interactions_list.insert(std::make_pair(sensor_sensing_id_pair, new_interactions_list_itor->second));
						}
						new_interactions_list_itor++;
					}
				}
			}

			auto old_interactions_list_itor = exist_old_interactions_list.begin();
			while (old_interactions_list_itor != exist_old_interactions_list.end())
			{
				const QPair<TYPE_ULID, TYPE_ULID>& sensor_sensing_id_pair = *old_interactions_list_itor;
				clearSensorInteractionsInfo(sensor_sensing_id_pair, entityulid);
				old_interactions_list_itor++;
			}
		}
	}

	tagPathInfo GaeactorEnvironment::getPath(const TYPE_ULID &uildsrc, const TYPE_ULID &dst)
	{
		return m_pDijkstra->getPath(uildsrc, dst);
	}



	bool GaeactorEnvironment::isEntitySensorProperty(const TYPE_ULID &entityulid)
	{
		//EASY_FUNCTION(profiler::colors::DeepOrange)
		bool isSensor = false;
		QReadLocker _sensorslist_mutex_locker(&m_sensorslist_mutex);
		auto _sensorslistitor = m_sensorslist.find(entityulid);
		if (_sensorslistitor != m_sensorslist.end())
		{
			isSensor = true;
		}
		return isSensor;
	}

	bool GaeactorEnvironment::isEntityHaveSensorProperty(const TYPE_ULID &entityulid)
	{
		//EASY_FUNCTION(profiler::colors::DeepOrange)
		bool isSensor = false;
		QReadLocker _entitys_ulid_mutex_locker(&m_entitys_ulid_mutex);
		auto entityuliditor = m_entitys_ulid.find(entityulid);
		if (entityuliditor != m_entitys_ulid.end())
		{
			isSensor = entityuliditor->second.m_isSensor;
		}
		return isSensor;
	}

	QList<std::tuple<TYPE_ULID, H3INDEX> > GaeactorEnvironment::getValidEntityList()
	{
		QList<std::tuple<TYPE_ULID, H3INDEX> >  entityvalidlist;
		QReadLocker _entitys_ulid_mutex_locker(&m_entitys_ulid_mutex);
		auto entityuliditor = m_entitys_ulid.cbegin();
		while (entityuliditor != m_entitys_ulid.cend())
		{
			const TYPE_ULID &entityulid = entityuliditor->first;
			if (!entityuliditor->second.m_isSensor && entityuliditor->second.m_bEntityValid)
			{
				entityvalidlist.push_back(std::make_tuple(entityulid, entityuliditor->second.m_h3Index));
			}
			entityuliditor++;
		}
		return entityvalidlist;
	}

	QList<std::tuple<TYPE_ULID, H3INDEX> > GaeactorEnvironment::getValidSensorEntityList()
	{
		QList<std::tuple<TYPE_ULID, H3INDEX> >  entityvalidlist;
		QReadLocker _entitys_ulid_mutex_locker(&m_entitys_ulid_mutex);
		auto entityuliditor = m_entitys_ulid.cbegin();
		while (entityuliditor != m_entitys_ulid.cend())
		{
			const TYPE_ULID &entityulid = entityuliditor->first;
			if (entityuliditor->second.m_isSensor && entityuliditor->second.m_bEntityValid)
			{
				entityvalidlist.push_back(std::make_tuple(entityulid, entityuliditor->second.m_h3Index));
			}
			entityuliditor++;
		}
		return entityvalidlist;
	}

	QList<std::tuple<TYPE_ULID, H3INDEX> > GaeactorEnvironment::getValidAllEntityList()
	{
		QList<std::tuple<TYPE_ULID, H3INDEX> >  entityvalidlist;
		QReadLocker _entitys_ulid_mutex_locker(&m_entitys_ulid_mutex);
		auto entityuliditor = m_entitys_ulid.cbegin();
		while (entityuliditor != m_entitys_ulid.cend())
		{
			const TYPE_ULID &entityulid = entityuliditor->first;
			if (entityuliditor->second.m_bEntityValid)
			{
				entityvalidlist.push_back(std::make_tuple(entityulid, entityuliditor->second.m_h3Index));
			}
			entityuliditor++;
		}
		return entityvalidlist;
	}

	QList<std::tuple<TYPE_ULID, H3INDEX> > GaeactorEnvironment::getValidAllExcludeEntityList(const TYPE_ULID &ulid)
	{
		QList<std::tuple<TYPE_ULID, H3INDEX> >  entityvalidlist;
		QReadLocker _entitys_ulid_mutex_locker(&m_entitys_ulid_mutex);
		auto entityuliditor = m_entitys_ulid.cbegin();
		while (entityuliditor != m_entitys_ulid.cend())
		{
			const TYPE_ULID &entityulid = entityuliditor->first;
			if (entityuliditor->second.m_bEntityValid && ulid != entityulid)
			{
				entityvalidlist.push_back(std::make_tuple(entityulid, entityuliditor->second.m_h3Index));
			}
			entityuliditor++;
		}
		return entityvalidlist;
	}

	H3INDEX GaeactorEnvironment::getEntityHexidx(const TYPE_ULID &ulid)
	{
		QReadLocker _entitys_ulid_mutex_locker(&m_entitys_ulid_mutex);
		auto itor2 = m_entitys_ulid.find(ulid);
		if (itor2 != m_entitys_ulid.cend())
		{
			return itor2->second.m_h3Index;
		}
		return 0;
	}

	transdata_entityposinfo GaeactorEnvironment::gettransdata_entityposinfo(const TYPE_ULID &ulid)
	{
		QReadLocker _entitys_ulid_mutex_locker(&m_entitys_ulid_mutex);
		auto itor2 = m_entitys_ulid.find(ulid);
		if (itor2 != m_entitys_ulid.cend())
		{
			return itor2->second.m_entityinfo;
		}
		return transdata_entityposinfo();
	}

	tagSensorInfo GaeactorEnvironment::gettransdata_sensorposinfo(const TYPE_ULID &ulid)
	{
		QReadLocker _sensors_ulid_info_mutex_locker(&m_sensors_ulid_info_mutex);
		auto itor2 = m_sensors_ulid_info.find(ulid);
		if (itor2 != m_sensors_ulid_info.cend())
		{
			return itor2->second;
		}
		return tagSensorInfo();
	}


	CLEAR_RELATION_INFO_LIST GaeactorEnvironment::refershRelation()
	{
		//EASY_FUNCTION(profiler::colors::Teal)
		return refershSensorSilenceStatusclearSensorEntityRelation();
	}

	IDENTIFI_EVENT_INFO GaeactorEnvironment::refershEvent(int id)
	{
		//EASY_FUNCTION(profiler::colors::DeepPurple)
		IDENTIFI_EVENT_INFO identifi_event_info;
		EVENTS_HASHMAP &addEventlist = std::get<0>(identifi_event_info);
		EVENTS_HASHMAP &clearEventlist = std::get<1>(identifi_event_info);
		EVENTS_HASHMAP &updateEventlist = std::get<2>(identifi_event_info);

#if 0
		//获取该线程对应处理的sensor列表
		QReadLocker _sensors_ulid_hexidxs_mutex_locker(&m_sensors_ulid_hexidxs_mutex);
		auto sensorsulidhexidx_itor = m_sensors_ulid_hexidxs.begin();
		while (sensorsulidhexidx_itor != m_sensors_ulid_hexidxs.end())
		{
			auto &sensorsulid = sensorsulidhexidx_itor->first;
			SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = sensorsulidhexidx_itor->second;

			auto sensorentituhexidx = gaeactorenvironment::GaeactorEnvironment::getInstance().getEntityHexidx(sensorsulid);

			if (sensorentituhexidx != 0)
			{
				SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP & entity_ulid_list = sensor_interceve_entity_info.get_sensor_entity_interceve_info_hashmap_lock_ForRead();
				for (auto entity_ulid_list_itor = entity_ulid_list.begin(); entity_ulid_list_itor != entity_ulid_list.end(); entity_ulid_list_itor++)
				{
					auto &entityulid = entity_ulid_list_itor->first;
					QPair<TYPE_ULID, TYPE_ULID> eventtile = qMakePair(sensorsulid, entityulid);

					auto _sensorposinfo = gaeactorenvironment::GaeactorEnvironment::getInstance().gettransdata_entityposinfo(sensorsulid);
					auto _entityposinfo = gaeactorenvironment::GaeactorEnvironment::getInstance().gettransdata_entityposinfo(entityulid);

					transdata_sensorposinfo _entityproprety;
					bool _entityisSensorProprety = isEntitySensorProperty(entityulid);
					if (_entityisSensorProprety)
					{
						_entityproprety = gaeactorenvironment::GaeactorEnvironment::getInstance().gettransdata_sensorposinfo(entityulid);
					}
					EVENT_INFO eventinfo{ sensorsulid,
										   entityulid,
										   sensor_interceve_entity_info.m_sensor_property.m_eWaveSensorSourceType,
										   sensorentituhexidx,
										   entity_ulid_list_itor->second.m_srchexidx,
										   std::move(_sensorposinfo),
										   std::move(_entityposinfo),
										   sensor_interceve_entity_info.m_sensorinfo,
										   _entityisSensorProprety,
										   _entityproprety };

					if (entity_ulid_list_itor->second.isAddStatus())
					{
						eventinfo.m_timestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
						addEventlist.insert(std::make_pair(std::move(eventtile), eventinfo));
					}
					else if (entity_ulid_list_itor->second.isUpdateStatus())
					{
						eventinfo.m_timestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
						updateEventlist.insert(std::make_pair(std::move(eventtile), eventinfo));
					}
					else if (entity_ulid_list_itor->second.isRemoving())
					{
						eventinfo.m_timestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
						clearEventlist.insert(std::make_pair(std::move(eventtile), eventinfo));
					}
				}
				sensor_interceve_entity_info.set_sensor_entity_interceve_info_hashmap_unlock();
			}
			sensorsulidhexidx_itor++;
		}
#else
		//    EVENTS_HASHMAP new_exist_events_map;
		EVENTS_HASHMAP &new_exist_events_map = m_exist_events_map[id];
		//获取该线程对应处理的sensor列表
		std::unordered_map<TYPE_ULID, bool> & current_event_thread_deal_list = m_event_thread_deal_list[id];

		auto current_event_thread_deal_listitor = current_event_thread_deal_list.begin();
		while (current_event_thread_deal_listitor != current_event_thread_deal_list.end())
		{
			if (!current_event_thread_deal_listitor->second)
			{
				current_event_thread_deal_listitor++;
				continue;
			}
			const TYPE_ULID & event_deal_sensorid = current_event_thread_deal_listitor->first;
			{
				//QReadLocker _sensors_ulid_hexidxs_mutex_locker(&m_sensors_ulid_hexidxs_mutex);
				auto sensorsulidhexidx_itor = m_sensors_ulid_hexidxs.find(event_deal_sensorid);
				if (sensorsulidhexidx_itor != m_sensors_ulid_hexidxs.end())
				{
					auto &sensorsulid = sensorsulidhexidx_itor->first;
					SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = sensorsulidhexidx_itor->second;

					auto sensorentituhexidx = gaeactorenvironment::GaeactorEnvironment::getInstance().getEntityHexidx(sensorsulid);

					if (sensorentituhexidx != 0)
					{
						SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP & entity_ulid_list = sensor_interceve_entity_info.get_sensor_entity_interceve_info_hashmap_lock_ForRead();
						for (auto entity_ulid_list_itor = entity_ulid_list.begin(); entity_ulid_list_itor != entity_ulid_list.end(); entity_ulid_list_itor++)
						{
							auto &entity_sensingmedia_id_pair = entity_ulid_list_itor->first;
							SENSOR_ENTITY_INTERCEVE_INFO & sensor_entity_interceve_info = entity_ulid_list_itor->second;
							EVENT_KEY_TYPE eventtile = EVENT_KEY_TYPE{ sensorsulid, entity_sensingmedia_id_pair.first,entity_sensingmedia_id_pair.second };
							auto _exist_events_map_itor = new_exist_events_map.find(eventtile);
							if (_exist_events_map_itor != new_exist_events_map.end())
							{
								if (entity_ulid_list_itor->second.m_bInterceveValid)
								{
									EVENT_INFO & entity_info = _exist_events_map_itor->second;
									//update exist event
									entity_info.m_sensorposinfo = gaeactorenvironment::GaeactorEnvironment::getInstance().gettransdata_entityposinfo(sensorsulid);
									entity_info.m_entityposinfo = gaeactorenvironment::GaeactorEnvironment::getInstance().gettransdata_entityposinfo(entity_sensingmedia_id_pair.first);
									if (sensor_interceve_entity_info.m_sensor_infos.find(entity_sensingmedia_id_pair.second) != sensor_interceve_entity_info.m_sensor_infos.end())
									{
										entity_info.m_sensorproprety = sensor_interceve_entity_info.m_sensor_infos.at(entity_sensingmedia_id_pair.second).m_sensorinfo;
									}
									entity_info.m_distance = H3Helper::greatDistanceM(entity_info.m_sensorposinfo.PARAM_pos_hexidx, entity_info.m_entityposinfo.PARAM_pos_hexidx);
									entity_info.m_entityisSensorProprety = isEntityHaveSensorProperty(entity_sensingmedia_id_pair.first);
									if (entity_info.m_entityisSensorProprety)
									{
										tagSensorInfo sensorinfo = gaeactorenvironment::GaeactorEnvironment::getInstance().gettransdata_sensorposinfo(entity_sensingmedia_id_pair.first);
										entity_info.m_entityproprety = sensorinfo.m_sensorinfo;
									}
									entity_info.m_timestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
									updateEventlist.insert(std::make_pair(std::move(eventtile), _exist_events_map_itor->second));
									//m_pDijkstra->updateEdge(sensorsulid, entityulid, 1);
								}
								else
								{
									//clear exist event
									EVENT_INFO & event_info = _exist_events_map_itor->second;
									event_info.m_timestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();

									clearEventlist.insert(std::make_pair(std::move(eventtile), _exist_events_map_itor->second));
									new_exist_events_map.erase(_exist_events_map_itor);
								}
							}
							else
							{
								if (entity_ulid_list_itor->second.m_bInterceveValid)
								{
									//add new event
									auto _sensorposinfo = gaeactorenvironment::GaeactorEnvironment::getInstance().gettransdata_entityposinfo(sensorsulid);
									auto _entityposinfo = gaeactorenvironment::GaeactorEnvironment::getInstance().gettransdata_entityposinfo(entity_sensingmedia_id_pair.first);

									auto distance = H3Helper::greatDistanceM(_sensorposinfo.PARAM_pos_hexidx, _entityposinfo.PARAM_pos_hexidx);

									transdata_sensorposinfo _entityproprety;
									bool _entityisSensorProprety = isEntityHaveSensorProperty(entity_sensingmedia_id_pair.first);
									if (_entityisSensorProprety)
									{
										tagSensorInfo sensorinfo = gaeactorenvironment::GaeactorEnvironment::getInstance().gettransdata_sensorposinfo(entity_sensingmedia_id_pair.first);
										_entityproprety = sensorinfo.m_sensorinfo;
									}

									transdata_sensorposinfo _sensorinfo;
									if (sensor_interceve_entity_info.m_sensor_infos.find(entity_sensingmedia_id_pair.second) != sensor_interceve_entity_info.m_sensor_infos.end())
									{
										_sensorinfo = sensor_interceve_entity_info.m_sensor_infos.at(entity_sensingmedia_id_pair.second).m_sensorinfo;
									}

									EVENT_INFO eventinfo{ sensorsulid,
														 entity_sensingmedia_id_pair.first,
														 entity_sensingmedia_id_pair.second,
														 sensor_interceve_entity_info.m_sensor_property.m_eWaveSensorSourceType,
														 std::move(_sensorposinfo),
														 std::move(_entityposinfo),
														 _sensorinfo,
														 _entityisSensorProprety,
														 std::move(_entityproprety),
														 distance,
														 stdutils::OriDateTime::getCurrentUTCTimeStampMicros() };

									new_exist_events_map.insert(std::make_pair(eventtile, std::move(eventinfo)));
									addEventlist.insert(std::make_pair(std::move(eventtile), new_exist_events_map.at(eventtile)));
									//m_pDijkstra->updateEdge(sensorsulid, entityulid, 1);
								}
							}
						}
						sensor_interceve_entity_info.set_sensor_entity_interceve_info_hashmap_unlock();
					}
				}
			}
			current_event_thread_deal_listitor++;
		}

#if 0
		{
			///////////////////////////////////////////////////////////////////////////////////////////////////////////
			EVENTS_HASHMAP &curretn_exist_events_map = m_exist_events_map[id];
			//事件存在于新表中，存在则更新 不存在则 添加
			auto new_exist_events_map_itor = new_exist_events_map.begin();
			while (new_exist_events_map_itor != new_exist_events_map.end())
			{
				const QPair<TYPE_ULID, TYPE_ULID> &eventtile = new_exist_events_map_itor->first;
				auto old_eventsmap_itor = curretn_exist_events_map.find(eventtile);
				if (old_eventsmap_itor != curretn_exist_events_map.end())
				{
					new_exist_events_map_itor->second.m_timestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
					updateEventlist.insert(std::make_pair(eventtile, new_exist_events_map_itor->second));
					curretn_exist_events_map[eventtile] = std::move(new_exist_events_map_itor->second);
				}
				else
				{
					new_exist_events_map_itor->second.m_timestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
					addEventlist.insert(std::make_pair(eventtile, new_exist_events_map_itor->second));
					curretn_exist_events_map.insert(std::make_pair(eventtile, std::move(new_exist_events_map_itor->second)));
				}
				new_exist_events_map_itor++;
			}

			//事件存在于旧表中，新表中不存在则 移除
			auto old_exist_events_map_itor = curretn_exist_events_map.begin();
			while (old_exist_events_map_itor != curretn_exist_events_map.end())
			{
				const QPair<TYPE_ULID, TYPE_ULID> &eventtile = old_exist_events_map_itor->first;
				auto new_eventsmap_itor = new_exist_events_map.find(eventtile);
				if (new_eventsmap_itor == new_exist_events_map.end())
				{
					old_exist_events_map_itor->second.m_timestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
					clearEventlist.insert(std::make_pair(old_exist_events_map_itor->first, old_exist_events_map_itor->second));
					old_exist_events_map_itor = curretn_exist_events_map.erase(old_exist_events_map_itor);
					continue;
				}
				old_exist_events_map_itor++;
			}
#ifdef ENABLE_ERASE
			QWriteLocker _clear_events_mutex_locker(&m_clear_events_mutex);

			auto _clear_events_itor = m_clear_events.begin();
			while (_clear_events_itor != m_clear_events.end())
			{
				const QPair<TYPE_ULID, TYPE_ULID> & eventtile = _clear_events_itor->first;
				bool &bEventClear = _clear_events_itor->second;

				auto _old_exist_events_map_itor = curretn_exist_events_map.find(eventtile);
				if (_old_exist_events_map_itor != curretn_exist_events_map.end())
				{
					bEventClear = false;
					_old_exist_events_map_itor->second.m_timestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
					clearEventlist.insert(std::make_pair(std::move(eventtile), _old_exist_events_map_itor->second));
					curretn_exist_events_map.erase(_old_exist_events_map_itor);
				}
				if (!bEventClear)
				{
					_clear_events_itor = m_clear_events.erase(_clear_events_itor);
					continue;
				}
				_clear_events_itor++;
			}
#endif
		}
#endif
#endif

		return identifi_event_info;
	}


#define ARC_SENSOR_TO_EITITY
	//#define SHOW_SENSOR_TO_ENTITY_WAVE


	void GaeactorEnvironment::refershInteractions(int id)
	{
		//EASY_FUNCTION(profiler::colors::Teal)

		std::list<std::tuple<TYPE_ULID, EVENT_TUPLE, HEXIDX_ARRAY, QVector<LAT_LNG>, bool>> result;
		auto generateEmptyIntercation = [&](const EVENT_KEY_TYPE& event_key_tuple,
			bool bGenerateEchoWave)
		{
			auto &sensorid = event_key_tuple.sensorid;
			auto &entityid = event_key_tuple.entityid;
			auto &sensingmediaid = event_key_tuple.sensingmediaid;

			auto  sensortoentityechowaveinfo = std::make_tuple(entityid, sensorid, sensingmediaid, 0, 0);

			QVector<LAT_LNG> geolatlnglistsensortoentity;
			HEXIDX_ARRAY hexidxslistsensortoentity;

			auto   retval = std::make_tuple(sensorid,
				std::move(sensortoentityechowaveinfo), \
				std::move(hexidxslistsensortoentity), \
				std::move(geolatlnglistsensortoentity),
				bGenerateEchoWave);
			result.emplace_back(std::move(retval));
		};

		auto generateEValidIntercation = [&](const EVENT_KEY_TYPE& event_key_tuple,
			const H3INDEX entitysrchexidx, \
			const H3INDEX &sensorentituhexidx, \
			bool bGenerateEchoWave)
		{
			auto &sensorid = event_key_tuple.sensorid;
			auto &entityid = event_key_tuple.entityid;
			auto &sensingmediaid = event_key_tuple.sensingmediaid;

			int res = H3Helper::getresolution(entitysrchexidx) - 5;
			res = res <= 15 ? res : 15;
			res = res >= 0 ? res : 1;

			H3INDEX entityhexidxsrcdstres;
			H3Helper::conversionHexidxResolution(entityhexidxsrcdstres, entitysrchexidx, res);
			H3INDEX sensorentituhexidxdstres;
			H3Helper::conversionHexidxResolution(sensorentituhexidxdstres, sensorentituhexidx, res);



			QVector<LAT_LNG> geolatlnglistsensortoentity;
			HEXIDX_ARRAY hexidxslistsensortoentity;
#ifdef ARC_SENSOR_TO_EITITY
			//hexidxslistsensortoentity = H3Helper::getArcCells(sensorentituhexidxdstres,entityhexidxsrcdstres,USING_SPAN_ANGLE,res,geolatlnglistsensortoentity);

			hexidxslistsensortoentity.push_back(sensorentituhexidxdstres);
			hexidxslistsensortoentity.push_back(entityhexidxsrcdstres);

			auto _sensorposinfo = gaeactorenvironment::GaeactorEnvironment::getInstance().gettransdata_entityposinfo(sensorid);
            auto _entityposinfo = gaeactorenvironment::GaeactorEnvironment::getInstance().gettransdata_entityposinfo(entityid);
            geolatlnglistsensortoentity.push_back(LAT_LNG{(double)(_sensorposinfo.PARAM_latitude) / LON_LAT_ACCURACY, (double)(_sensorposinfo.PARAM_longitude) / LON_LAT_ACCURACY});
            geolatlnglistsensortoentity.push_back(LAT_LNG{(double)(_entityposinfo.PARAM_latitude) / LON_LAT_ACCURACY, (double)(_entityposinfo.PARAM_longitude) / LON_LAT_ACCURACY});
#else
			hexidxslistsensortoentity = H3Helper::getArcCells(entityhexidxsrcdstres, sensorentituhexidxdstres, USING_SPAN_ANGLE, res, geolatlnglistsensortoentity);
#endif
			//                        LOG_PRINT_STR_EX("generate hit wave")

			auto  sensortoentityechowaveinfo = std::make_tuple(entityid, sensorid, sensingmediaid, entityhexidxsrcdstres, sensorentituhexidxdstres);
			auto   retval = std::make_tuple(sensorid,
				std::move(sensortoentityechowaveinfo), \
				std::move(hexidxslistsensortoentity), \
				std::move(geolatlnglistsensortoentity),
				bGenerateEchoWave);
			result.emplace_back(std::move(retval));


		};




		//    std::unordered_map<QPair<TYPE_ULID,TYPE_ULID>, std::tuple<H3INDEX,H3INDEX, bool,bool >> &new_exist_interactions_map = m_exist_interactions_map[id];
		std::unordered_map<EVENT_KEY_TYPE, std::tuple<H3INDEX, H3INDEX, bool, bool >> new_exist_interactions_map;
		//获取该线程对应处理的sensor列表
		std::unordered_map<TYPE_ULID, bool> & current_interactions_thread_deal_list = m_interactions_thread_deal_list[id];

		auto current_interactions_thread_deal_listitor = current_interactions_thread_deal_list.begin();
		while (current_interactions_thread_deal_listitor != current_interactions_thread_deal_list.end())
		{
			if (!current_interactions_thread_deal_listitor->second)
			{
				current_interactions_thread_deal_listitor++;
				continue;
			}
			const TYPE_ULID & interactions_deal_sensorid = current_interactions_thread_deal_listitor->first;

			{
				QReadLocker _sensors_ulid_hexidxs_mutex_locker(&m_sensors_ulid_hexidxs_mutex);
				auto sensorsulidhexidx_itor = m_sensors_ulid_hexidxs.find(interactions_deal_sensorid);
				if (sensorsulidhexidx_itor != m_sensors_ulid_hexidxs.end())
				{
					auto &sensorsulid = sensorsulidhexidx_itor->first;
					SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = sensorsulidhexidx_itor->second;

					auto sensorentituhexidx = gaeactorenvironment::GaeactorEnvironment::getInstance().getEntityHexidx(sensorsulid);
					//if(sensorentituhexidx != 0)
					{
						SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP & entity_ulid_list = sensor_interceve_entity_info.get_sensor_entity_interceve_info_hashmap_lock_ForRead();
						for (auto entity_ulid_list_itor = entity_ulid_list.begin(); entity_ulid_list_itor != entity_ulid_list.end(); entity_ulid_list_itor++)
						{
							auto &entity_sensingmedia_id_pair = entity_ulid_list_itor->first;

							bool bGenerateEchoWave = true;
							//                        switch(sensor_interceve_entity_info.m_sensor_property.m_eWaveSensorSourceType)
							//                        {
							//                        case E_WAVE_SENSOR_SOURCE_TYPE_SRC:bGenerateEchoWave = true;break;
							//                        case E_WAVE_SENSOR_SOURCE_TYPE_ECHO:bGenerateEchoWave = false;break;
							//                        default:break;
							//                        }
							EVENT_KEY_TYPE eventtile = EVENT_KEY_TYPE{ sensorsulid, entity_sensingmedia_id_pair.first,entity_sensingmedia_id_pair.second };

#if 1
							auto _new_exist_interactions_itor = new_exist_interactions_map.find(eventtile);
							if (_new_exist_interactions_itor != new_exist_interactions_map.end())
							{
								if (entity_ulid_list_itor->second.m_bInterceveValid)
								{
									std::get<0>(_new_exist_interactions_itor->second) = entity_ulid_list_itor->second.m_srchexidx;
									std::get<1>(_new_exist_interactions_itor->second) = sensorentituhexidx;
									std::get<2>(_new_exist_interactions_itor->second) = bGenerateEchoWave;
									std::get<3>(_new_exist_interactions_itor->second) = entity_ulid_list_itor->second.m_bInterceveValid;

									//                                generateEValidIntercation(eventtile.first,
									//                                                          eventtile.second,
									//                                                          std::get<0>(_new_exist_interactions_itor->second),
									//                                                          std::get<1>(_new_exist_interactions_itor->second),
									//                                                          std::get<2>(_new_exist_interactions_itor->second));
								}
								else
								{
									//                                generateEmptyIntercation(eventtile.first,
									//                                                         eventtile.second,
									//                                                         std::get<2>(_new_exist_interactions_itor->second));

									new_exist_interactions_map.erase(_new_exist_interactions_itor);
								}
							}
							else
							{
								if (entity_ulid_list_itor->second.m_bInterceveValid)
								{
									//                                generateEValidIntercation(eventtile.first,
									//                                                          eventtile.second,
									//                                                          std::get<0>(_new_exist_interactions_itor->second),
									//                                                          std::get<1>(_new_exist_interactions_itor->second),
									//                                                          std::get<2>(_new_exist_interactions_itor->second));

									std::tuple<H3INDEX, H3INDEX, bool, bool> item = std::make_tuple(entity_ulid_list_itor->second.m_srchexidx, sensorentituhexidx, bGenerateEchoWave, entity_ulid_list_itor->second.m_bInterceveValid);
									new_exist_interactions_map.insert(std::make_pair(eventtile, std::move(item)));
								}
							}
#else
							auto _new_exist_interactions_itor = new_exist_interactions_map.find(eventtile);
							if (_new_exist_interactions_itor != new_exist_interactions_map.end())
							{
								std::get<0>(_new_exist_interactions_itor->second) = entity_ulid_list_itor->second.m_srchexidx;
								std::get<1>(_new_exist_interactions_itor->second) = sensorentituhexidx;
								std::get<2>(_new_exist_interactions_itor->second) = bGenerateEchoWave;
								std::get<3>(_new_exist_interactions_itor->second) = entity_ulid_list_itor->second.m_bInterceveValid;

								//                            generateEValidIntercation(eventtile.first,
								//                                                      eventtile.second,
								//                                                      std::get<0>(_new_exist_interactions_itor->second),
								//                                                      std::get<1>(_new_exist_interactions_itor->second),
								//                                                      std::get<2>(_new_exist_interactions_itor->second));
							}
							else
							{
								//                            generateEValidIntercation(eventtile.first,
								//                                                      eventtile.second,
								//                                                      std::get<0>(_new_exist_interactions_itor->second),
								//                                                      std::get<1>(_new_exist_interactions_itor->second),
								//                                                      std::get<2>(_new_exist_interactions_itor->second));

								std::tuple<H3INDEX, H3INDEX, bool, bool> item = std::make_tuple(entity_ulid_list_itor->second.m_srchexidx, sensorentituhexidx, bGenerateEchoWave, entity_ulid_list_itor->second.m_bInterceveValid);
								new_exist_interactions_map.insert(std::make_pair(eventtile, std::move(item)));
							}
#endif
						}
						sensor_interceve_entity_info.set_sensor_entity_interceve_info_hashmap_unlock();
					}
				}
			}

			current_interactions_thread_deal_listitor++;
		}

#if 1
		{
			///////////////////////////////////////////////////////////////////////////////////////////////////////////
			std::unordered_map<EVENT_KEY_TYPE, std::tuple<H3INDEX, H3INDEX, bool, bool>> &current_exist_interactions_map = m_exist_interactions_map[id];
			//事件存在于新表中，存在则更新 不存在则 添加
			auto new_exist_interactions_map_itor = new_exist_interactions_map.begin();
			while (new_exist_interactions_map_itor != new_exist_interactions_map.end())
			{
				const EVENT_KEY_TYPE &eventtile = new_exist_interactions_map_itor->first;
				auto old_exist_interactions_map_itor = current_exist_interactions_map.find(eventtile);
				if (old_exist_interactions_map_itor != current_exist_interactions_map.end())
				{
					if (std::get<3>(new_exist_interactions_map_itor->second))
					{
						generateEValidIntercation(eventtile,
							std::get<0>(new_exist_interactions_map_itor->second),
							std::get<1>(new_exist_interactions_map_itor->second),
							std::get<2>(new_exist_interactions_map_itor->second));
						current_exist_interactions_map[eventtile] = std::move(new_exist_interactions_map_itor->second);
					}
					else
					{
						generateEmptyIntercation(eventtile,
							std::get<2>(new_exist_interactions_map_itor->second));
					}
				}
				else
				{
					generateEValidIntercation(eventtile,
						std::get<0>(new_exist_interactions_map_itor->second),
						std::get<1>(new_exist_interactions_map_itor->second),
						std::get<2>(new_exist_interactions_map_itor->second));
					current_exist_interactions_map.insert(std::make_pair(eventtile, std::move(new_exist_interactions_map_itor->second)));
				}
				new_exist_interactions_map_itor++;
			}

			//事件存在于旧表中，新表中不存在则 移除
			auto old_exist_interactions_map_itor = current_exist_interactions_map.begin();
			while (old_exist_interactions_map_itor != current_exist_interactions_map.end())
			{
				const EVENT_KEY_TYPE &eventtile = old_exist_interactions_map_itor->first;
				auto _new_exist_interactions_map_itor = new_exist_interactions_map.find(eventtile);
				if (_new_exist_interactions_map_itor == new_exist_interactions_map.end())
				{
					generateEmptyIntercation(eventtile,
						std::get<2>(old_exist_interactions_map_itor->second));
					old_exist_interactions_map_itor = current_exist_interactions_map.erase(old_exist_interactions_map_itor);
					continue;
				}
				old_exist_interactions_map_itor++;
			}

			//#ifdef ENABLE_ERASE
			//        QWriteLocker _clear_interactions_mutex_locker(&m_clear_interactions_mutex);

			//        auto _clear_interactions_itor = m_clear_interactions.begin();
			//        while (_clear_interactions_itor != m_clear_interactions.end())
			//        {
			//            const QPair<TYPE_ULID,TYPE_ULID> & eventtile = _clear_interactions_itor->first;
			//            bool &bInteractionsClear = _clear_interactions_itor->second;

			//            auto _old_exist_interactions_itor = current_exist_interactions_map.find(eventtile);
			//            if(_old_exist_interactions_itor != current_exist_interactions_map.end())
			//            {
			//                bInteractionsClear = false;
			//                generateEmptyIntercation(eventtile.first, eventtile.second, std::get<2>(_old_exist_interactions_itor->second));
			//                current_exist_interactions_map.erase(_old_exist_interactions_itor);
			//            }
			//            if(!bInteractionsClear)
			//            {
			//                _clear_interactions_itor = m_clear_interactions.erase(_clear_interactions_itor);
			//                continue;
			//            }
			//            _clear_interactions_itor++;
			//        }
			//#endif
		}

#endif

		//    for(auto itor = result.begin(); itor != result.end(); itor++)
		//    {
		//        m_echowave_display_hexidx_update_callback(std::get<0>(*itor),\
		//                                                  std::get<1>(*itor),\
		//                                                  std::get<2>(*itor),\
		//                                                  std::get<3>(*itor),\
		//                                                  std::get<4>(*itor));
		//    }

		//    m_echowave_list_display_hexidx_update_callback(result);


	}

	QList<UINT64> GaeactorEnvironment::getResolutions()
	{
		//EASY_FUNCTION(profiler::colors::Brown)
		QList<UINT64> retlist;
		QReadLocker _resolution_sensorsulid_mutex_locker(&m_resolution_sensorsulid_mutex);
		auto itor = m_resolution_sensorsulid.begin();
		while (itor != m_resolution_sensorsulid.end())
		{
			retlist.push_back(itor->first);
			itor++;
		}
		return retlist;
	}

	void GaeactorEnvironment::registDisplayCallback(echowave_display_hexidx_update_callback func)
	{
		m_echowave_display_hexidx_update_callback = std::move(func);
	}

	void GaeactorEnvironment::registDisplayListCallback(echowave_list_display_hexidx_update_callback func)
	{
		m_echowave_list_display_hexidx_update_callback = std::move(func);
	}

	void GaeactorEnvironment::registHexidxDisplayCallback(display_hexidx_update_callback func)
	{
		m_display_hexidx_update_callback = std::move(func);
	}

	void GaeactorEnvironment::registSensorUpdateCallback(sensor_update_callback func)
	{
		m_sensor_update_callback = std::move(func);
	}

	void GaeactorEnvironment::regist_data_update_callback(E_PROCESS_TYPE eprocessType, data_update_callback func)
	{
		m_data_update_callback_lsit.insert(std::make_pair(eprocessType, std::move(func)));
	}

	std::unordered_map<TYPE_ULID, QList<TYPE_ULID>> GaeactorEnvironment::getResolutionsHexidxs(UINT64 resolution)
	{
		QReadLocker _resolution_sensorsulid_mutex_locker(&m_resolution_sensorsulid_mutex);
		auto itor = m_resolution_sensorsulid.find(resolution);
		if (itor != m_resolution_sensorsulid.cend())
		{
			return itor->second;
		}
		return std::unordered_map<TYPE_ULID, QList<TYPE_ULID>>();
	}


	std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>> GaeactorEnvironment::getSensorsHexidxs(const H3INDEX &h3Index)
	{
		QReadLocker _hexidx_ulid_mutexlocker(&m_hexidx_ulid_mutex);
		auto itor = m_hexidx_ulid.find(h3Index);
		if (itor != m_hexidx_ulid.end())
		{
			return itor->second;
		}
		return std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>();
	}

	std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>> GaeactorEnvironment::getSensorsResolutionHexidxs(const H3INDEX &h3Index)
	{
		UINT64 hexidx_resolution = H3_GET_RESOLUTION(h3Index);
		return getSensorsResolutionHexidxs(hexidx_resolution, h3Index);
	}

	std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>> GaeactorEnvironment::getSensorsResolutionHexidxs(const UINT64 &hexidx_resolution, const H3INDEX &h3Index)
	{
		QReadLocker _hexidx_ulid_mutexlocker(&m_resolution_hexidx_ulid_mutex);
		auto itor = m_resolution_hexidx_ulid.find(hexidx_resolution);
		if (itor != m_resolution_hexidx_ulid.end())
		{
			std::unordered_map<H3INDEX, std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>> & hexidx_ULIDlist = itor->second;
			auto hexidx_ULIDlistitor = hexidx_ULIDlist.find(h3Index);
			if (hexidx_ULIDlistitor != hexidx_ULIDlist.end())
			{
				std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>& ULIDlist = hexidx_ULIDlistitor->second;
				return ULIDlist;
			}
		}
		return std::unordered_map<TYPE_ULID, std::tuple<bool, std::unordered_map<TYPE_ULID, bool>>>();
	}

	void GaeactorEnvironment::refershDataUpdate(E_PROCESS_TYPE eprocessType)
	{
		if (eprocessType & E_PROCESS_TYPE_AUDITIONS)
		{
			m_data_update_callback_lsit.at(E_PROCESS_TYPE_AUDITIONS)(true);
		}

		if (eprocessType & E_PROCESS_TYPE_EVENTS)
		{
			m_data_update_callback_lsit.at(E_PROCESS_TYPE_EVENTS)(true);
		}


		if (eprocessType & E_PROCESS_TYPE_INTERACTIONS)
		{
			m_data_update_callback_lsit.at(E_PROCESS_TYPE_INTERACTIONS)(true);
		}
	}

	bool GaeactorEnvironment::getUlidSensorInfo(const TYPE_ULID &ulid, const TYPE_ULID& sensingmediaid, transdata_sensorposinfo &_sensorinfo)
	{
		QReadLocker _sensors_ulid_hexidxs_mutexlocker(&m_sensors_ulid_hexidxs_mutex);
		auto itor = m_sensors_ulid_hexidxs.find(ulid);
		if (itor != m_sensors_ulid_hexidxs.end())
		{
			SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = itor->second;
			auto itor3 = sensor_interceve_entity_info.m_sensor_infos.find(sensingmediaid);
			if (itor3 != sensor_interceve_entity_info.m_sensor_infos.end())
			{
				tagSensorInfo &cur_sensorinfo = itor3->second;

				_sensorinfo = cur_sensorinfo.m_sensorinfo;
			}
		}
		return true;
	}
	bool GaeactorEnvironment::getUlidSensorHexidxs(const TYPE_ULID &ulid, const TYPE_ULID& sensingmediaid, SENSOR_INFO_TYPE& sensor_info, bool bget)
	{
		QReadLocker _sensors_ulid_hexidxs_mutexlocker(&m_sensors_ulid_hexidxs_mutex);
		auto itor = m_sensors_ulid_hexidxs.find(ulid);
		if (itor != m_sensors_ulid_hexidxs.end())
		{
			SENSOR_INTERCEVE_ENTITY_INFO & sensor_interceve_entity_info = itor->second;
			if (!bget && !sensor_interceve_entity_info.m_sensor_hexidx_info_map_bchange)
			{
				return false;
			}
			SENSOR_HEXIDX_INFO_HASHMAP &ulid_hexidxmap = sensor_interceve_entity_info.get_sensor_hexidx_info_map_lock_ForRead();
			//        SENSOR_HEXIDX_INFO_HASHMAP::iterator ulid_hexidxitor = ulid_hexidxmap.begin();
			//        while(ulid_hexidxitor != ulid_hexidxmap.end())
			auto ulid_hexidxitor = ulid_hexidxmap.find(sensingmediaid);
			if (ulid_hexidxitor != ulid_hexidxmap.end())
			{
				HEXIDX_TIME_STAMP_INFO_HASHMAP & hexidx_time_stamp_info_list = ulid_hexidxitor->second;
				HEXIDX_TIME_STAMP_INFO_HASHMAP::iterator hexidx_time_stamp_info_list_itor = hexidx_time_stamp_info_list.begin();
				while (hexidx_time_stamp_info_list_itor != hexidx_time_stamp_info_list.end())
				{
					const H3INDEX & hexidx = hexidx_time_stamp_info_list_itor->first;
					if (hexidx_time_stamp_info_list_itor->second.m_bHexidxValid)
					{
						std::get<0>(sensor_info).push_back(hexidx);
					}
					hexidx_time_stamp_info_list_itor++;
				}
				//ulid_hexidxitor++;
			}
			sensor_interceve_entity_info.m_sensor_hexidx_info_map_bchange = false;
			sensor_interceve_entity_info.set_sensor_hexidx_info_map_unlock();
			auto itor3 = sensor_interceve_entity_info.m_sensor_infos.find(sensingmediaid);
			if (itor3 != sensor_interceve_entity_info.m_sensor_infos.end())
			{
				tagSensorInfo &cur_sensorinfo = itor3->second;

				std::get<1>(sensor_info) = cur_sensorinfo.m_polygons;
				std::get<2>(sensor_info) = cur_sensorinfo.m_sensorinfo;
			}
		}
		return true;
	}
}
