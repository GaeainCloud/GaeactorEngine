#include "gaeactor_processor_normal.h"
#include "src/OriginalDateTime.h"
#include <iostream>
#include "LocationHelper.h"
#include <h3Index.h>
#include <QTimer>

#include "easy/profiler.h"
#include "loghelper.h"
#include <sstream>
#include "runningmodeconfig.h"
#include "settingsconfig.h"
#define SUB_SIZE (7)

#define USING_LIST_TMP

#define H3_RES_BC_MASK ((uint64_t)(2047) << H3_BC_OFFSET)

namespace gaeactorenvironment_ex_normal
{
HexIdexInfo::HexIdexInfo()
	:m_h3Index(H3_INIT),
	m_eHexidexStatus(E_HEXINDEX_STATUS_FREE)
{
	m_bValid.store(false);
}

void HexIdexInfo::init(const H3INDEX& h3, H3CellInfo &&h3cellinfo,H3IndexBufferManager *_pH3IndexBufferManager)
{
	m_h3Index = h3;
	m_h3CellInfo = std::move(h3cellinfo);
    m_pH3IndexBufferManager = _pH3IndexBufferManager;
}

void HexIdexInfo::updateData(const TYPE_ULID& uildsrc, const H3INDEX& h3, const UINT32& basecell, const UINT32& resolution, const UINT64& digit_origin, E_DISPLAY_MODE eDdisplayMode, bool bRemove)
{
    EASY_FUNCTION(profiler::colors::Green)
	UINT64 digit_valid = digit_origin >> ((INDEX_MAPPING_RESOLUTION_MAX - m_h3CellInfo.resolution) * 3);
	if ((H3_INIT == m_h3Index) ||
		m_h3CellInfo.resolution > resolution ||
		m_h3CellInfo.basecell != basecell ||
		m_h3CellInfo.digit_valid != digit_valid)
	{
		return;
	}

	switch (eDdisplayMode)
	{
	case E_DISPLAY_MODE_ENTITY:
	{
		{
			QWriteLocker locker(&m_entitylist_mutex);

			if (bRemove)
			{
				const auto finditor = m_entitylist.find(uildsrc);
				if (finditor != m_entitylist.end())
				{
					m_entitylist.erase(finditor);
				}
			}
			else
			{
				const auto finditor = m_entitylist.find(uildsrc);
				if (finditor != m_entitylist.end())
				{
					finditor->second = true;
				}
				else
				{
					m_entitylist.insert(std::make_pair(uildsrc, true));
				}
			}
		}

		if (!m_entitylist.empty())
		{
			m_eHexidexStatus = static_cast<E_HEXINDEX_STATUS>(m_eHexidexStatus | E_HEXINDEX_STATUS_ENTITY);
		}
		else
		{
            m_eHexidexStatus = static_cast<E_HEXINDEX_STATUS>(m_eHexidexStatus & ~E_HEXINDEX_STATUS_ENTITY);
        }
        updateIntersect();
    }
	break;
	default:
		break;
	}
	if (!m_sensorlist.empty() || !m_entitylist.empty())
	{
		m_bValid.store(true);
	}
	else
	{
		m_bValid.store(false);
		m_eHexidexStatus = E_HEXINDEX_STATUS_FREE;
	}
}

void HexIdexInfo::updateData(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const H3INDEX& h3, E_DISPLAY_MODE eDdisplayMode, bool bRemove)
{
	if (H3_INIT == m_h3Index || m_h3Index != h3)
	{
		return;
	}

	switch (eDdisplayMode)
	{
	case E_DISPLAY_MODE_WAVE:
	{
		{
			QWriteLocker locker(&m_sensorlist_mutex);
			QPair<TYPE_ULID, TYPE_ULID> uildsrc = qMakePair(sensorulid, sensingmediaid);
			if (bRemove)
			{
				const auto finditor = m_sensorlist.find(uildsrc);
				if (finditor != m_sensorlist.end())
				{
					m_sensorlist.erase(finditor);
				}
			}
			else
			{
				const auto finditor = m_sensorlist.find(uildsrc);
				if (finditor != m_sensorlist.end())
				{
					finditor->second = true;
				}
				else
				{
					m_sensorlist.insert(std::make_pair(uildsrc, true));
				}
			}
		}
		if (!m_sensorlist.empty())
		{
			m_eHexidexStatus = static_cast<E_HEXINDEX_STATUS>(m_eHexidexStatus | E_HEXINDEX_STATUS_SENSOR);
		}
		else
		{
			m_eHexidexStatus = static_cast<E_HEXINDEX_STATUS>(m_eHexidexStatus & ~E_HEXINDEX_STATUS_SENSOR);
        }
        updateIntersect();
    }
	break;
	default:
		break;
	}
	if (!m_sensorlist.empty() || !m_entitylist.empty())
	{
		m_bValid.store(true);
	}
	else
	{
		m_bValid.store(false);
		m_eHexidexStatus = E_HEXINDEX_STATUS_FREE;
	}
}

E_HEXINDEX_STATUS HexIdexInfo::getHexidexStatus()
{
	return m_eHexidexStatus;
}

bool HexIdexInfo::isValid()
{
    return m_bValid.load();
}

void HexIdexInfo::updateIntersect()
{
    EASY_FUNCTION(profiler::colors::Red)
    if(m_eHexidexStatus == gaeactorenvironment_ex_normal::E_HEXINDEX_STATUS_ALL)
    {
        tagIntersectInfo interscet;
        std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, bool> sensorlist = this->sensorlist();

        std::unordered_map<TYPE_ULID, bool> entitylist = this->entitylist();
        interscet.sensorlist = std::move(sensorlist);
        interscet.entitylist = std::move(entitylist);

        if(m_pH3IndexBufferManager)
        {
            m_pH3IndexBufferManager->appendIntersect(m_h3Index, std::move(interscet));
        }
    }
    else
    {
        if(m_pH3IndexBufferManager)
        {
            m_pH3IndexBufferManager->removeIntersect(m_h3Index);
        }
    }
}

std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, bool> HexIdexInfo::sensorlist()
{
	QReadLocker locker(&m_sensorlist_mutex);
	return m_sensorlist;
}

std::unordered_map<TYPE_ULID, bool> HexIdexInfo::entitylist()
{
	QReadLocker locker(&m_entitylist_mutex);
	return m_entitylist;
}



INTERSECTINFOMAP& H3IndexBufferManager::IntersectInfoMap()
{
    return m_IntersectInfoMap;
}

QMutex *H3IndexBufferManager::IntersectInfoMap_update_mutex()
{
    return &m_IntersectInfoMap_update_mutex;
}

QReadWriteLock* H3IndexBufferManager::IntersectInfoMap_mutex()
{
    return &m_IntersectInfoMap_mutex;
}
H3IndexBufferManager::H3IndexBufferManager()
    :m_pGaeactorProcessor(nullptr)
{

}

H3IndexBufferManager::~H3IndexBufferManager()
{

}

void H3IndexBufferManager::setGaeactorProcessor(GaeactorProcessor *pGaeactorProcessor)
{
    m_pGaeactorProcessor = pGaeactorProcessor;
}

H3CellInfo H3IndexBufferManager::getCellInfo(const H3INDEX& h3)
{
	H3CellInfo cellinfo;
	cellinfo.basecell = H3_GET_BASE_CELL(h3);
	cellinfo.resolution = H3_GET_RESOLUTION(h3);
//	cellinfo.resolution_basecell = ((int)((((h3)&H3_RES_BC_MASK) >> H3_BC_OFFSET)));
    UINT64 resolution_basecell_digit_origin = (UINT64)(h3 << (H3_NUM_BITS - H3_RES_OFFSET));
    cellinfo.resolution_basecell_digit_valid = resolution_basecell_digit_origin >> ((INDEX_MAPPING_RESOLUTION_MAX - cellinfo.resolution) * 3);
    UINT64 digit_origin = (UINT64)(h3)&H3_INIT;
    cellinfo.digit_valid = digit_origin >> ((INDEX_MAPPING_RESOLUTION_MAX - cellinfo.resolution) * 3);
	return cellinfo;
}

std::vector<HexIdexInfo*>  H3IndexBufferManager::getCellBuffersByEntityHex(const H3INDEX& h3)
{
	H3CellInfo h3cellinfo = getCellInfo(h3);
	std::vector<HexIdexInfo*>  hexindexinfos;
	hexindexinfos.resize(h3cellinfo.resolution + 1);
	for (int res = 0; res <= h3cellinfo.resolution; res++)
	{
		H3INDEX parentindex;
		if (E_SUCCESS == cellToParent(h3, res, &parentindex))
		{
			HexIdexInfo* pHexIdexInfo = getCellBufferByHex(parentindex);
			if (pHexIdexInfo)
			{
				hexindexinfos[res] = pHexIdexInfo;
			}
		}
	}
	return hexindexinfos;
}

HexIdexInfo* H3IndexBufferManager::getCellBufferByHex(const H3INDEX& h3)
{
    EASY_FUNCTION(profiler::colors::Green)
	HexIdexInfo* hexindexinfo = nullptr;
	H3CellInfo h3cellinfo = getCellInfo(h3);
    {
        QMutexLocker locker(&m_h3CellBuffer_mutex);
        auto _h3indexBuffer_itor = m_Cellbuffer.find(h3cellinfo.resolution_basecell_digit_valid);
        if (_h3indexBuffer_itor != m_Cellbuffer.end())
        {
            hexindexinfo = _h3indexBuffer_itor->second;
        }
    }
    if(hexindexinfo == nullptr)
    {
        hexindexinfo = new HexIdexInfo();
        hexindexinfo->init(h3, std::move(h3cellinfo),this);
        {
            QMutexLocker locker(&m_h3CellBuffer_mutex);
            m_Cellbuffer.insert(std::make_pair(hexindexinfo->m_h3CellInfo.resolution_basecell_digit_valid, hexindexinfo));
        }
    }
    return hexindexinfo;
}

void H3IndexBufferManager::lockIntersect()
{
    m_IntersectInfoMap_update_mutex.lock();
}

void H3IndexBufferManager::unlockIntersect()
{
    m_IntersectInfoMap_update_mutex.unlock();
}

void H3IndexBufferManager::appendIntersect(const H3INDEX &h3, tagIntersectInfo &&info)
{
    EASY_FUNCTION(profiler::colors::Green)
    QWriteLocker locker(&m_IntersectInfoMap_mutex);
    auto itor = m_IntersectInfoMap.find(h3);
    if(itor != m_IntersectInfoMap.end())
    {
        itor->second = std::move(info);
    }
    else
    {
        m_IntersectInfoMap.insert(std::make_pair(h3, std::move(info)));
    }
}


void H3IndexBufferManager::removeIntersect(const H3INDEX &h3)
{
    EASY_FUNCTION(profiler::colors::Green)
    QWriteLocker locker(&m_IntersectInfoMap_mutex);
    auto itor = m_IntersectInfoMap.find(h3);
    if(itor != m_IntersectInfoMap.end())
    {
        m_IntersectInfoMap.erase(itor);
    }
}

CELLBUFFERMAP H3IndexBufferManager::getCellbuffers()
{
    QMutexLocker locker(&m_h3CellBuffer_mutex);
    return m_Cellbuffer;
}

std::unordered_map<H3INDEX, tagIntersectInfo> H3IndexBufferManager::getIntersectInfos()
{
    EASY_FUNCTION(profiler::colors::Green)
    QMutexLocker _IntersectInfoMap_update_locker(&m_IntersectInfoMap_update_mutex);

    QReadLocker locker(&m_IntersectInfoMap_mutex);
    return m_IntersectInfoMap;
}

std::unordered_map<H3INDEX, tagIntersectInfo> H3IndexBufferManager::getIntersectInfos_by_cores(const TYPE_ULID &entityid)
{
    EASY_FUNCTION(profiler::colors::Green)
    std::unordered_map<H3INDEX, tagIntersectInfo> ret;
    {
        QMutexLocker _IntersectInfoMap_update_locker(&m_IntersectInfoMap_update_mutex);
        QReadLocker locker(&m_IntersectInfoMap_mutex);
        //返回的结果可能必须只会存在 或者 没有
        auto intersectInfos_itor = m_IntersectInfoMap.begin();
        while (intersectInfos_itor != m_IntersectInfoMap.end())
        {
            auto exist_itor = intersectInfos_itor->second.entitylist.find(entityid);
            if(exist_itor != intersectInfos_itor->second.entitylist.end())
            {
                ret.insert(std::make_pair(intersectInfos_itor->first, intersectInfos_itor->second));
            }
            intersectInfos_itor++;
        }
    }
    return ret;
}

std::unordered_map<H3INDEX, tagIntersectInfo> H3IndexBufferManager::getIntersectInfos_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmediaid)
{
    EASY_FUNCTION(profiler::colors::Green)
    QPair<TYPE_ULID, TYPE_ULID> pair_val = qMakePair(sensorid, sensingmediaid);
    std::unordered_map<H3INDEX, tagIntersectInfo> ret;
    {
        QMutexLocker _IntersectInfoMap_update_locker(&m_IntersectInfoMap_update_mutex);
        QReadLocker locker(&m_IntersectInfoMap_mutex);
        //返回的结果可能必须只会存在 或者 没有
        auto intersectInfos_itor = m_IntersectInfoMap.begin();
        while (intersectInfos_itor != m_IntersectInfoMap.end())
        {
            auto exist_itor = intersectInfos_itor->second.sensorlist.find(pair_val);
            if(exist_itor != intersectInfos_itor->second.sensorlist.end())
            {
                ret.insert(std::make_pair(intersectInfos_itor->first, intersectInfos_itor->second));
            }
            intersectInfos_itor++;
        }
    }
    return ret;
}


void H3IndexBufferManager::deal_hexindex_entity(const TYPE_ULID& uildsrc, const H3INDEX& h3, E_DISPLAY_MODE eDdisplayMode, bool bRemove)
{
    EASY_FUNCTION(profiler::colors::Green)
	UINT32 basecell = H3_GET_BASE_CELL(h3);
	UINT32 resolution = H3_GET_RESOLUTION(h3);
	UINT64 digit_origin = (UINT64)(h3)&H3_INIT;

	std::vector<HexIdexInfo*> pHexIdexInfos = getCellBuffersByEntityHex(h3);
	if (!pHexIdexInfos.empty())
	{
		for (int index = 0; index < pHexIdexInfos.size(); index++)
		{
			HexIdexInfo* pHexIdexInfo = pHexIdexInfos[index];
			if (pHexIdexInfo)
			{
				pHexIdexInfo->updateData(uildsrc, h3, basecell, resolution, digit_origin, eDdisplayMode, bRemove);
			}
		}
	}
}

void H3IndexBufferManager::deal_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const H3INDEX& h3, E_DISPLAY_MODE eDdisplayMode, bool bRemove)
{    
    EASY_FUNCTION(profiler::colors::Green)
	HexIdexInfo* pHexIdexInfo = getCellBufferByHex(h3);
	if (pHexIdexInfo)
	{
		pHexIdexInfo->updateData(sensorulid, sensingmediaid, h3, eDdisplayMode, bRemove);
	}
}

GaeactorProcessor::GaeactorProcessor(QObject* parent /*= nullptr*/)
{
}

GaeactorProcessor::~GaeactorProcessor()
{

}

void GaeactorProcessor::cleardata()
{
	{
		QReadLocker locker(&m_sensorInfosMap_mutex);
		auto _sensorInfosMap_itor = this->m_sensorInfosMap.begin();
		while (_sensorInfosMap_itor != this->m_sensorInfosMap.end())
		{
			tagHexindexSensorInfo* ptagHexindexSensorInfo = _sensorInfosMap_itor->second;
			if(ptagHexindexSensorInfo)
			{
				delete ptagHexindexSensorInfo;
			}

			_sensorInfosMap_itor++;
		}
	}
	{
		QWriteLocker locker(&m_sensorInfosMap_mutex);
		this->m_sensorInfosMap.clear();
	}
}

H3IndexBufferManager& GaeactorProcessor::getBuffer()
{
    return m_H3IndexBufferManager;
}

void GaeactorProcessor::deal_hexindex(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const H3INDEX& h3, E_DISPLAY_MODE eDdisplayMode, bool bRemove)
{    
    EASY_FUNCTION(profiler::colors::Green)
	switch (eDdisplayMode)
	{
	case E_DISPLAY_MODE_ENTITY:
	{
		if (bRemove)
		{
			clear_hexindex_entity(sensorulid, h3);
		}
		else
		{
			deal_hexindex_entity(sensorulid, h3);
		}
	}
	break;
	case E_DISPLAY_MODE_WAVE:
	{
		HEXIDX_ARRAY hexidxslist;
		hexidxslist.push_back(h3);

		if (bRemove)
		{
			clear_hexindex_sensor(sensorulid, sensingmediaid, hexidxslist);
		}
		else
		{
			deal_hexindex_sensor(sensorulid, sensingmediaid, hexidxslist);
		}
	}
	break;
	case E_DISPLAY_MODE_INTERSECTION:
		break;
	case E_DISPLAY_MODE_ECHO:
		break;
	default:
		break;
	}
}

void GaeactorProcessor::clear_hexindex_entity(const TYPE_ULID& uildsrc)
{
	QReadLocker locker(&m_entityInfosMap_mutex);
	auto entityinfo_itor = m_entityInfosMap.find(uildsrc);
	if (entityinfo_itor != m_entityInfosMap.end())
	{
        m_H3IndexBufferManager.deal_hexindex_entity(uildsrc, entityinfo_itor->second.m_h3Index, E_DISPLAY_MODE_ENTITY, true);
		entityinfo_itor->second.m_bValid = false;
	}
}

void GaeactorProcessor::clear_hexindex_entity(const TYPE_ULID& uildsrc, const H3INDEX& h3)
{
	QReadLocker locker(&m_entityInfosMap_mutex);
	auto entityinfo_itor = m_entityInfosMap.find(uildsrc);
	if (entityinfo_itor != m_entityInfosMap.end())
	{
        if (/*entityinfo_itor->second.m_bValid &&*/ h3 == entityinfo_itor->second.m_h3Index)
		{
            m_H3IndexBufferManager.deal_hexindex_entity(uildsrc, entityinfo_itor->second.m_h3Index, E_DISPLAY_MODE_ENTITY, true);
			entityinfo_itor->second.m_bValid = false;
		}
	}
}

void GaeactorProcessor::clear_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid)
{
	{
		QReadLocker locker(&m_sensorInfosMap_mutex);
		auto sensorInfo_itor = m_sensorInfosMap.find(sensorulid);
		if (sensorInfo_itor != m_sensorInfosMap.end() && sensorInfo_itor->second)
		{
			tagHexindexSensorInfo *hexsensorinfo = sensorInfo_itor->second;
			hexsensorinfo->clear_hexindex_sensor(sensorulid, sensingmediaid);
		}
	}
}

void GaeactorProcessor::clear_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_ARRAY& hexidxslist)
{
	{
		QReadLocker locker(&m_sensorInfosMap_mutex);
		auto sensorInfo_itor = m_sensorInfosMap.find(sensorulid);
		if (sensorInfo_itor != m_sensorInfosMap.end() && sensorInfo_itor->second)
		{
			tagHexindexSensorInfo *hexsensorinfo = sensorInfo_itor->second;
			hexsensorinfo->clear_hexindex_sensor_by_hexidexlist(sensorulid, sensingmediaid, hexidxslist);
		}
	}
}

void GaeactorProcessor::deal_hexindex_entity(const TYPE_ULID& uildsrc, const H3INDEX& h3)
{
	bool bNewAppend = true;
	{
		QWriteLocker locker(&m_entityInfosMap_mutex);
		auto entityinfo_itor = m_entityInfosMap.find(uildsrc);
		if (entityinfo_itor != m_entityInfosMap.end())
        {
            bool bUpdateRemoveAppend = false;
			if (entityinfo_itor->second.m_h3Index != h3)
			{
//				if (entityinfo_itor->second.m_bValid)
				{
                    bUpdateRemoveAppend = true;
                    //此处添加交互关系map ，是因为存在更新，避免出现移除再添加时候的中间时间差，导致产生多余的移除再调教事件，当同个飞机的h3索引更新，移除之前先上锁，添加之后再解锁
                    m_H3IndexBufferManager.lockIntersect();
                    m_H3IndexBufferManager.deal_hexindex_entity(uildsrc, entityinfo_itor->second.m_h3Index, E_DISPLAY_MODE_ENTITY, true);
				}
				entityinfo_itor->second.m_bValid = false;
			}
			else
			{
				entityinfo_itor->second.m_bValid = true;
			}
			if (!entityinfo_itor->second.m_bValid)
			{
				entityinfo_itor->second.m_bValid = true;
				entityinfo_itor->second.m_h3Index = h3;
                m_H3IndexBufferManager.deal_hexindex_entity(uildsrc, h3, E_DISPLAY_MODE_ENTITY, false);
                if(bUpdateRemoveAppend)
                {
                    m_H3IndexBufferManager.unlockIntersect();
                }
			}
			bNewAppend = false;
		}
	}
	if(bNewAppend)
	{
		tagHexindexEntityInfo hexindexentityinfo;
		hexindexentityinfo.m_h3Index = h3;
		hexindexentityinfo.m_bValid = true;
		{
			QWriteLocker locker(&m_entityInfosMap_mutex);
			m_entityInfosMap.insert(std::make_pair(uildsrc, std::move(hexindexentityinfo)));

            std::stringstream ss;
            ss<<"++++++++++++++++++++++++++total entity size:"<<m_entityInfosMap.size();
            TRACE_LOG_PRINT_EX2(ss);
		}
        m_H3IndexBufferManager.deal_hexindex_entity(uildsrc, h3, E_DISPLAY_MODE_ENTITY, false);
	}
}

void GaeactorProcessor::update_hexindex_entity(const TYPE_ULID& uildsrc, const H3INDEX& h3,
                                               const FLOAT64 &hgt, const transdata_entityposinfo& _entityinfo)
{
	EASY_FUNCTION(profiler::colors::Green)
	bool bNewAppend = true;
	{
		QWriteLocker locker(&m_entityInfosMap_mutex);
		auto entityinfo_itor = m_entityInfosMap.find(uildsrc);
		if (entityinfo_itor != m_entityInfosMap.end())
		{
            bool bUpdateRemoveAppend = false;
			if (entityinfo_itor->second.m_h3Index != h3)
			{
                //if (entityinfo_itor->second.m_bValid)
				{
                    bUpdateRemoveAppend = true;
                    //此处添加交互关系map ，是因为存在更新，避免出现移除再添加时候的中间时间差，导致产生多余的移除再调教事件，当同个飞机的h3索引更新，移除之前先上锁，添加之后再解锁
                    m_H3IndexBufferManager.lockIntersect();

                    m_H3IndexBufferManager.deal_hexindex_entity(uildsrc, entityinfo_itor->second.m_h3Index, E_DISPLAY_MODE_ENTITY, true);
				}
				entityinfo_itor->second.m_bValid = false;
			}
			else
			{
                entityinfo_itor->second.m_bValid = (h3 != 0) ? true : false;
                entityinfo_itor->second.m_h3Index = h3;
                entityinfo_itor->second.m_entityInfo = _entityinfo;
            }
            if (!entityinfo_itor->second.m_bValid)
            {
                entityinfo_itor->second.m_bValid = (h3 != 0) ? true : false;
                entityinfo_itor->second.m_h3Index = h3;
                entityinfo_itor->second.m_entityInfo = _entityinfo;
                entityinfo_itor->second.m_isSensor = (PROPERTY_GET_TYPE(_entityinfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR) ? true : false;
                if(h3 != 0)
                {
                    m_H3IndexBufferManager.deal_hexindex_entity(uildsrc, h3, E_DISPLAY_MODE_ENTITY, false);
                }
                if(bUpdateRemoveAppend)
                {
                    m_H3IndexBufferManager.unlockIntersect();
                }
            }

			bNewAppend = false;
		}
	}
    if(bNewAppend)
	{
		tagHexindexEntityInfo hexindexentityinfo;
        hexindexentityinfo.m_h3Index = h3;
        hexindexentityinfo.m_bValid = (h3 != 0) ? true : false;
		hexindexentityinfo.m_entityInfo = _entityinfo;
        hexindexentityinfo.m_isSensor = (PROPERTY_GET_TYPE(_entityinfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR)? true : false;
		{
			QWriteLocker locker(&m_entityInfosMap_mutex);
			m_entityInfosMap.insert(std::make_pair(uildsrc, std::move(hexindexentityinfo)));

            std::stringstream ss;
            ss<<"++++++++++++++++++++++++++total entity size:"<<m_entityInfosMap.size();
            TRACE_LOG_PRINT_EX2(ss);

		}
        if(h3 != 0)
        {
            m_H3IndexBufferManager.deal_hexindex_entity(uildsrc, h3, E_DISPLAY_MODE_ENTITY, false);
        }
	}
}

void GaeactorProcessor::deal_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_ARRAY& hexidxslist, const UINT32 &_silent_time)
{    
    EASY_FUNCTION(profiler::colors::Green)
	UINT32 silent_time = _silent_time;
	UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();

	bool bNewAppend = true;
	{
		QWriteLocker locker(&m_sensorInfosMap_mutex);
		auto sensorInfo_itor = m_sensorInfosMap.find(sensorulid);
		if (sensorInfo_itor != m_sensorInfosMap.end() && sensorInfo_itor->second)
		{
			tagHexindexSensorInfo *hexindexsensorinfo = sensorInfo_itor->second;
			auto ptagHexindexlistSensorInfo = hexindexsensorinfo->getHexindexlistSensorInfo(sensingmediaid);
			if (nullptr != ptagHexindexlistSensorInfo)
            {
                if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_CLEAR_OLD_SENSOR_SENSINGMEDIA_FORCE_COVER_MODE())
                {
                    //清理旧的存在 新的不存在 的无效的
                    force_cover_update_hexindex_sensor_h3list_ex(sensorulid, sensingmediaid, hexidxslist, ptagHexindexlistSensorInfo);
                }
                ///////////////////////////////////////////////////////////////////////////////
				//新的存在 旧的不存在 / 新的存在 旧的存在但被设置无效的
				HEXIDX_ARRAY newapendhexidxslist;
				ptagHexindexlistSensorInfo->deal_newappend_hexindexs_by_h3list(newapendhexidxslist, hexidxslist, currentTimeStamp);
				for (auto newitem : newapendhexidxslist)
				{
                    m_H3IndexBufferManager.deal_hexindex_sensor(sensorulid, sensingmediaid, newitem, E_DISPLAY_MODE_WAVE, false);
				}


				if (!hexidxslist.empty())
				{
					ptagHexindexlistSensorInfo->m_bValid = true;
					ptagHexindexlistSensorInfo->m_silent_time = silent_time;
					ptagHexindexlistSensorInfo->m_lastUpdateTimestamp = currentTimeStamp;
				}
				///////////////////////////////////////////////////////////////////////////////
			}
			else
			{
				tagHexindexlistSensorInfo *hexindexlistensorinfo = new tagHexindexlistSensorInfo;
				for (auto h3 : hexidxslist)
				{
					hexindexlistensorinfo->append_hexindexinfo(h3, currentTimeStamp);
                    m_H3IndexBufferManager.deal_hexindex_sensor(sensorulid, sensingmediaid, h3, E_DISPLAY_MODE_WAVE, false);
				}
				if (!hexidxslist.empty())
				{
					hexindexlistensorinfo->m_bValid = true;
					hexindexlistensorinfo->m_silent_time = silent_time;
					hexindexlistensorinfo->m_lastUpdateTimestamp = currentTimeStamp;
				}
				hexindexsensorinfo->append_HexindexlistSensorInfo(sensingmediaid, std::move(hexindexlistensorinfo));
			}
			bNewAppend = false;
		}
	}
	if(bNewAppend)
    {
        tagHexindexSensorInfo *hexindexsensorinfo = new tagHexindexSensorInfo(&m_H3IndexBufferManager);

		tagHexindexlistSensorInfo *hexindexlistensorinfo = new tagHexindexlistSensorInfo;
		for (auto h3 : hexidxslist)
		{
			hexindexlistensorinfo->append_hexindexinfo(h3, currentTimeStamp);
            m_H3IndexBufferManager.deal_hexindex_sensor(sensorulid, sensingmediaid, h3, E_DISPLAY_MODE_WAVE, false);
		}
		if (!hexidxslist.empty())
		{
			hexindexlistensorinfo->m_bValid = true;
			hexindexlistensorinfo->m_silent_time = silent_time;
			hexindexlistensorinfo->m_lastUpdateTimestamp = currentTimeStamp;
		}

		hexindexsensorinfo->append_HexindexlistSensorInfo(sensingmediaid, std::move(hexindexlistensorinfo));
		{
			QWriteLocker locker(&m_sensorInfosMap_mutex);
			m_sensorInfosMap.insert(std::make_pair(sensorulid, std::move(hexindexsensorinfo)));

			//LOG_PRINT_STR_EX("++++++++++++++++++++++++++total sensor size:"+QString::number(m_sensorInfosMap.size()));
		}
	}
}

void GaeactorProcessor::update_hexindex_sensor(const TYPE_ULID &sensorulid,
                                               const TYPE_ULID &sensingmediaid,
                                               const HEXIDX_HGT_ARRAY  &_hexidxs,
                                               transdata_sensorposinfo &&_sensorinfo,
                                               POLYGON_LIST &&_polygon)
{
	EASY_FUNCTION(profiler::colors::Green)
	UINT32 silent_time = _sensorinfo.PARAM_wave_silent_time_gap;
	UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
	bool bNewAppend = true;
	{
		QWriteLocker locker(&m_sensorInfosMap_mutex);
		auto sensorInfo_itor = m_sensorInfosMap.find(sensorulid);
		if (sensorInfo_itor != m_sensorInfosMap.end() && sensorInfo_itor->second)
		{
			tagHexindexSensorInfo *hexindexsensorinfo = sensorInfo_itor->second;

            hexindexsensorinfo->m_bValid = true;
            hexindexsensorinfo->m_sensorinfo = _sensorinfo;

			auto ptagHexindexlistSensorInfo = hexindexsensorinfo->getHexindexlistSensorInfo(sensingmediaid);
			if (nullptr != ptagHexindexlistSensorInfo)
            {
                if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_CLEAR_OLD_SENSOR_SENSINGMEDIA_FORCE_COVER_MODE())
                {
                    //清理旧的存在 新的不存在 的无效的
                    force_cover_update_hexindex_sensor_ex(sensorulid, sensingmediaid, _hexidxs, ptagHexindexlistSensorInfo);
                }
                ///////////////////////////////////////////////////////////////////////////////
                //新的存在 旧的不存在 / 新的存在 旧的存在但被设置无效的
				HEXIDX_ARRAY newapendhexidxslist;
				ptagHexindexlistSensorInfo->deal_newappend_hexindexs_by_h3list(newapendhexidxslist, _hexidxs,currentTimeStamp);
				for (auto newitem : newapendhexidxslist)
				{
                    m_H3IndexBufferManager.deal_hexindex_sensor(sensorulid, sensingmediaid, newitem, E_DISPLAY_MODE_WAVE, false);
				}
				if (!_hexidxs.empty())
				{
                    ptagHexindexlistSensorInfo->m_sensorInfo = std::move(_sensorinfo);
                    ptagHexindexlistSensorInfo->m_polygon = std::move(_polygon);
					ptagHexindexlistSensorInfo->m_bValid = true;
					ptagHexindexlistSensorInfo->m_silent_time = silent_time;
					ptagHexindexlistSensorInfo->m_lastUpdateTimestamp = currentTimeStamp;
				}
				///////////////////////////////////////////////////////////////////////////////
			}
			else
			{
				tagHexindexlistSensorInfo *hexindexlistensorinfo = new tagHexindexlistSensorInfo;
				for (auto _hexidxs_item : _hexidxs)
				{
					const H3INDEX& h3 = _hexidxs_item.PARAM_seq_hexidx_element;

					hexindexlistensorinfo->append_hexindexinfo(h3, currentTimeStamp, _hexidxs_item);
                    m_H3IndexBufferManager.deal_hexindex_sensor(sensorulid, sensingmediaid, h3, E_DISPLAY_MODE_WAVE, false);
				}
				if (!_hexidxs.empty())
				{
                    hexindexlistensorinfo->m_sensorInfo = std::move(_sensorinfo);
                    hexindexlistensorinfo->m_polygon = std::move(_polygon);
					hexindexlistensorinfo->m_bValid = true;
					hexindexlistensorinfo->m_silent_time = silent_time;
					hexindexlistensorinfo->m_lastUpdateTimestamp = currentTimeStamp;
				}

				hexindexsensorinfo->append_HexindexlistSensorInfo(sensingmediaid, std::move(hexindexlistensorinfo));
			}
			bNewAppend = false;
		}
	}
	if(bNewAppend)
	{
        tagHexindexSensorInfo *hexindexsensorinfo = new tagHexindexSensorInfo(&m_H3IndexBufferManager);
        hexindexsensorinfo->m_bValid = true;
        hexindexsensorinfo->m_sensorinfo = _sensorinfo;

		tagHexindexlistSensorInfo *hexindexlistensorinfo = new tagHexindexlistSensorInfo;

		for (auto _hexidxs_item : _hexidxs)
		{
			const H3INDEX& h3 = _hexidxs_item.PARAM_seq_hexidx_element;
			hexindexlistensorinfo->append_hexindexinfo(h3, currentTimeStamp, _hexidxs_item);
            m_H3IndexBufferManager.deal_hexindex_sensor(sensorulid, sensingmediaid, h3, E_DISPLAY_MODE_WAVE, false);
		}
		if (!_hexidxs.empty())
		{
            hexindexlistensorinfo->m_sensorInfo = std::move(_sensorinfo);
            hexindexlistensorinfo->m_polygon = std::move(_polygon);
			hexindexlistensorinfo->m_bValid = true;
			hexindexlistensorinfo->m_silent_time = silent_time;
			hexindexlistensorinfo->m_lastUpdateTimestamp = currentTimeStamp;
		}

		hexindexsensorinfo->append_HexindexlistSensorInfo(sensingmediaid, std::move(hexindexlistensorinfo));
		{
			QWriteLocker locker(&m_sensorInfosMap_mutex);
			m_sensorInfosMap.insert(std::make_pair(sensorulid, std::move(hexindexsensorinfo)));

			//LOG_PRINT_STR_EX("++++++++++++++++++++++++++total sensor size:"+QString::number(m_sensorInfosMap.size()));
        }
    }
}

void GaeactorProcessor::force_cover_deal_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_ARRAY &hexidxslist, const UINT32 &_silent_time)
{
//    EASY_FUNCTION(profiler::colors::Green)
    {
        QWriteLocker locker(&m_sensorInfosMap_mutex);
        auto sensorInfo_itor = m_sensorInfosMap.find(sensorulid);
        if (sensorInfo_itor != m_sensorInfosMap.end() && sensorInfo_itor->second)
        {
            tagHexindexSensorInfo *hexindexsensorinfo = sensorInfo_itor->second;
            auto ptagHexindexlistSensorInfo = hexindexsensorinfo->getHexindexlistSensorInfo(sensingmediaid);
            if (nullptr != ptagHexindexlistSensorInfo)
            {
                //清理旧的存在 新的不存在 的无效的
                force_cover_update_hexindex_sensor_h3list_ex(sensorulid, sensingmediaid, hexidxslist, ptagHexindexlistSensorInfo);
            }
        }
    }
}


void GaeactorProcessor::force_cover_update_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &_hexidxs, transdata_sensorposinfo &&_sensorinfo, POLYGON_LIST &&_polygon)
{
    EASY_FUNCTION(profiler::colors::Green)
    {
        QWriteLocker locker(&m_sensorInfosMap_mutex);
        auto sensorInfo_itor = m_sensorInfosMap.find(sensorulid);
        if (sensorInfo_itor != m_sensorInfosMap.end() && sensorInfo_itor->second)
        {
            tagHexindexSensorInfo *hexindexsensorinfo = sensorInfo_itor->second;

            hexindexsensorinfo->m_bValid = true;
            hexindexsensorinfo->m_sensorinfo = _sensorinfo;

            auto ptagHexindexlistSensorInfo = hexindexsensorinfo->getHexindexlistSensorInfo(sensingmediaid);
            if (nullptr != ptagHexindexlistSensorInfo)
            {
                //清理旧的存在 新的不存在 的无效的
                force_cover_update_hexindex_sensor_ex(sensorulid, sensingmediaid, _hexidxs, ptagHexindexlistSensorInfo);
            }
        }
    }
}

void GaeactorProcessor::refresh_silent_timeout()
{
	UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
	QReadLocker locker(&m_sensorInfosMap_mutex);
	auto sensorInfo_itor = m_sensorInfosMap.begin();
	while (sensorInfo_itor != m_sensorInfosMap.end())
	{
		if(sensorInfo_itor->second)
		{
			const TYPE_ULID &sensorulid = sensorInfo_itor->first;
			tagHexindexSensorInfo *hexindexsensorinfo = sensorInfo_itor->second;
			hexindexsensorinfo->refresh_silent_timeout(sensorulid, currentTimeStamp);
		}
		sensorInfo_itor++;
	}
}

transdata_entityposinfo GaeactorProcessor::gettransdata_entityposinfo(const TYPE_ULID &ulid)
{
	QReadLocker locker(&m_entityInfosMap_mutex);
	auto itor2 = m_entityInfosMap.find(ulid);
	if (itor2 != m_entityInfosMap.cend())
	{
		return itor2->second.m_entityInfo;
	}
	return transdata_entityposinfo();
}

bool GaeactorProcessor::isEntityHaveSensorProperty(const TYPE_ULID &entityulid)
{
	//EASY_FUNCTION(profiler::colors::DeepOrange)
	bool isSensor = false;
	QReadLocker locker(&m_entityInfosMap_mutex);
	auto entityuliditor = m_entityInfosMap.find(entityulid);
	if (entityuliditor != m_entityInfosMap.end())
	{
		isSensor = entityuliditor->second.m_isSensor;
    }
    return isSensor;
}

void GaeactorProcessor::force_cover_update_hexindex_sensor_h3list_ex(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_ARRAY &hexidxslist, tagHexindexlistSensorInfo *ptagHexindexlistSensorInfo)
{
    EASY_FUNCTION(profiler::colors::Green)
    //清理旧的存在 新的不存在 的无效的
    HEXIDX_ARRAY invalidhexidxslist;
    ptagHexindexlistSensorInfo->deal_invalid_hexindexs_by_h3list(invalidhexidxslist, hexidxslist);
    for (auto invaliditem : invalidhexidxslist)
    {
        m_H3IndexBufferManager.deal_hexindex_sensor(sensorulid, sensingmediaid, invaliditem, E_DISPLAY_MODE_WAVE, true);
    }

}

void GaeactorProcessor::force_cover_update_hexindex_sensor_ex(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &_hexidxs,GaeactorProcessor::tagHexindexlistSensorInfo * ptagHexindexlistSensorInfo)
{
    EASY_FUNCTION(profiler::colors::Green)
    //清理旧的存在 新的不存在 的无效的
    HEXIDX_ARRAY invalidhexidxslist;
    ptagHexindexlistSensorInfo->deal_invalid_hexindexs_by_h3list(invalidhexidxslist, _hexidxs);
    for (auto invaliditem : invalidhexidxslist)
    {
        m_H3IndexBufferManager.deal_hexindex_sensor(sensorulid, sensingmediaid, invaliditem, E_DISPLAY_MODE_WAVE, true);
    }
}


transdata_sensorposinfo GaeactorProcessor::gettransdata_sensorposinfo(const TYPE_ULID &ulid)
{
	QReadLocker locker(&m_sensorInfosMap_mutex);
	auto sensorInfo_itor = m_sensorInfosMap.find(ulid);
	if (sensorInfo_itor != m_sensorInfosMap.cend() && sensorInfo_itor->second)
	{
		return sensorInfo_itor->second->m_sensorinfo;
	}
	return transdata_sensorposinfo();
}

transdata_sensorposinfo GaeactorProcessor::gettransdata_sensorposinfo_by_sensingmedia(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid)
{
	EASY_FUNCTION(profiler::colors::Green)
	QReadLocker locker(&m_sensorInfosMap_mutex);
	auto sensorInfo_itor = m_sensorInfosMap.find(sensorulid);
	if (sensorInfo_itor != m_sensorInfosMap.cend() && sensorInfo_itor->second)
	{
		return sensorInfo_itor->second->gettransdata_sensorposinfo_by_sensingmedia(sensorulid, sensingmediaid);
	}
	return transdata_sensorposinfo();
}
//#define INTERSECT_EVENT_AFTER

void GaeactorProcessor::refresh_event(IDENTIFI_EVENT_INFO &identifi_event_info)
{
	EVENTS_HASHMAP &addEventlist = std::get<0>(identifi_event_info);
	EVENTS_HASHMAP &clearEventlist = std::get<1>(identifi_event_info);
	EVENTS_HASHMAP &updateEventlist = std::get<2>(identifi_event_info);

	QWriteLocker locker(&m_exist_events_map_mutex);
	std::unordered_map<EVENT_KEY_TYPE, QPair<EVENT_INFO, bool>> &new_exist_events_map = m_exist_events_map;
	auto new_exist_events_map_itor = new_exist_events_map.begin();
	while(new_exist_events_map_itor != new_exist_events_map.end())
	{
		new_exist_events_map_itor->second.second = false;
		new_exist_events_map_itor++;
	}
    //static UINT64 m_ieventcount_sub = 0;
    //static UINT64 m_ieventcount_plus = 0;
#if 0
        UINT64 curTimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
#else
    UINT64 curTimestamp = SettingsConfig::getInstance().m_simparams.m_sim_timestamp * 1000;
#endif

#if 1
    std::unordered_map<H3INDEX, tagIntersectInfo> intersectInfos = m_H3IndexBufferManager.getIntersectInfos();

    auto intersectInfos_itor = intersectInfos.begin();
    while (intersectInfos_itor != intersectInfos.end())
    {
        std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, bool> &sensorlist = intersectInfos_itor->second.sensorlist;
        std::unordered_map<TYPE_ULID, bool> &entitylist = intersectInfos_itor->second.entitylist;

        auto sensorlist_itor = sensorlist.begin();
        while (sensorlist_itor != sensorlist.end())
        {
            const TYPE_ULID &sensorulid = sensorlist_itor->first.first;
            const TYPE_ULID &sensingmediaid = sensorlist_itor->first.second;
            auto entitylist_itor = entitylist.begin();
            while (entitylist_itor != entitylist.end())
            {
                const TYPE_ULID & entityulid = entitylist_itor->first;
                if (sensorulid == entityulid)
                {
                    entitylist_itor++;
                    continue;
                }
                EVENT_KEY_TYPE eventtile = EVENT_KEY_TYPE{ sensorulid, entityulid, sensingmediaid };

                auto _exist_events_map_itor = new_exist_events_map.find(eventtile);
                if (_exist_events_map_itor != new_exist_events_map.end())
                {
                    EVENT_INFO & event_info = _exist_events_map_itor->second.first;

                    _exist_events_map_itor->second.second = true;

                    //update exist event
                    auto new_sensorposinfo = gettransdata_entityposinfo(sensorulid);
                    auto new_entityposinfo = gettransdata_entityposinfo(entityulid);
                    //此处控制update事件的刷新频率，
                    //当场实体发生变化 或 运动实体发生变化 【感知位置发生变化】或 【感知位置未发生变化，但更新周期到了】上次更新时间间隔超过250ms，触发发送
                    //反之场实体未发生变化 且 运动实体未发生变化 且感知时间更新周期太短 ，则不触发该更新事件的发送
                    if(event_info.m_sensorposinfo.PARAM_pos_hexidx != new_sensorposinfo.PARAM_pos_hexidx ||
                        event_info.m_entityposinfo.PARAM_pos_hexidx != new_entityposinfo.PARAM_pos_hexidx /*||
                        curTimestamp - event_info.m_timestamp > 250*/)
                    {
                        event_info.m_sensorposinfo = std::move(new_sensorposinfo);
                        event_info.m_entityposinfo = std::move(new_entityposinfo);

                        event_info.m_sensorproprety = gettransdata_sensorposinfo_by_sensingmedia(sensorulid, sensingmediaid);
                        event_info.m_distance = LocationHelper::greatDistanceM(event_info.m_sensorposinfo.PARAM_pos_hexidx, event_info.m_entityposinfo.PARAM_pos_hexidx);
                        event_info.m_entityisSensorProprety = isEntityHaveSensorProperty(entityulid);
                        if (event_info.m_entityisSensorProprety)
                        {
                            event_info.m_entityproprety = gettransdata_sensorposinfo(entityulid);
                        }
                        event_info.m_timestamp = curTimestamp;
                        updateEventlist.insert(std::make_pair(std::move(eventtile), _exist_events_map_itor->second.first));
                        //std::cout << "update event " << eventtile.sensorid << " " << eventtile.sensingmediaid << " " << eventtile.entityid << std::endl;
                    }
                }
                else
                {
                    //add new event
                    auto _sensorposinfo = gettransdata_entityposinfo(sensorulid);
                    auto _entityposinfo = gettransdata_entityposinfo(entityulid);

                    auto distance = LocationHelper::greatDistanceM(_sensorposinfo.PARAM_pos_hexidx, _entityposinfo.PARAM_pos_hexidx);

                    transdata_sensorposinfo _entityproprety;
                    bool _entityisSensorProprety = isEntityHaveSensorProperty(entityulid);
                    if (_entityisSensorProprety)
                    {
                        _entityproprety = gettransdata_sensorposinfo(entityulid);
                    }

                    transdata_sensorposinfo _sensorproprety = gettransdata_sensorposinfo_by_sensingmedia(sensorulid, sensingmediaid);

                    EVENT_INFO eventinfo{ sensorulid,
                                         entityulid,
                                         sensingmediaid,
                                         std::move(_sensorposinfo),
                                         std::move(_entityposinfo),
                                         std::move(_sensorproprety),
                                         _entityisSensorProprety,
                                         std::move(_entityproprety),
                                         distance ,
                                         curTimestamp};

                    new_exist_events_map.insert(std::make_pair(eventtile, qMakePair(std::move(eventinfo), true)));
                    addEventlist.insert(std::make_pair(eventtile, new_exist_events_map.at(eventtile).first));
                    //m_ieventcount_plus++;
                    //std::cout << "add event " << m_ieventcount_plus << " " << eventtile.sensingmediaid << " " << eventtile.entityid << std::endl;
                }
                entitylist_itor++;
            }
            sensorlist_itor++;
        }

        intersectInfos_itor++;
    }

#else
    {
        QMutexLocker IntersectInfoMap_update_mutex_locker(m_H3IndexBufferManager.IntersectInfoMap_update_mutex());
        QReadLocker IntersectInfoMap_mutex_locker(m_H3IndexBufferManager.IntersectInfoMap_mutex());

        auto intersectInfos_itor = m_H3IndexBufferManager.IntersectInfoMap().begin();
        while (intersectInfos_itor != m_H3IndexBufferManager.IntersectInfoMap().end())
        {
            std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, bool> &sensorlist = intersectInfos_itor->second.sensorlist;
            std::unordered_map<TYPE_ULID, bool> &entitylist = intersectInfos_itor->second.entitylist;

            auto sensorlist_itor = sensorlist.begin();
            while (sensorlist_itor != sensorlist.end())
            {
                const TYPE_ULID &sensorulid = sensorlist_itor->first.first;
                const TYPE_ULID &sensingmediaid = sensorlist_itor->first.second;
                auto entitylist_itor = entitylist.begin();
                while (entitylist_itor != entitylist.end())
                {
                    const TYPE_ULID & entityulid = entitylist_itor->first;
                    if (sensorulid == entityulid)
                    {
                        entitylist_itor++;
                        continue;
                    }
                    EVENT_KEY_TYPE eventtile = EVENT_KEY_TYPE{ sensorulid, entityulid, sensingmediaid };

                    auto _exist_events_map_itor = new_exist_events_map.find(eventtile);
                    if (_exist_events_map_itor != new_exist_events_map.end())
                    {
                        EVENT_INFO & event_info = _exist_events_map_itor->second.first;

                        _exist_events_map_itor->second.second = true;

                        //update exist event
                        auto new_sensorposinfo = gettransdata_entityposinfo(sensorulid);
                        auto new_entityposinfo = gettransdata_entityposinfo(entityulid);
                        //此处控制update事件的刷新频率，
                        //当场实体发生变化 或 运动实体发生变化 【感知位置发生变化】或 【感知位置未发生变化，但更新周期到了】上次更新时间间隔超过250ms，触发发送
                        //反之场实体未发生变化 且 运动实体未发生变化 且感知时间更新周期太短 ，则不触发该更新事件的发送
                        if(event_info.m_sensorposinfo.PARAM_pos_hexidx != new_sensorposinfo.PARAM_pos_hexidx ||
                            event_info.m_entityposinfo.PARAM_pos_hexidx != new_entityposinfo.PARAM_pos_hexidx /*||
                        curTimestamp - event_info.m_timestamp > 250*/)
                        {
                            event_info.m_sensorposinfo = std::move(new_sensorposinfo);
                            event_info.m_entityposinfo = std::move(new_entityposinfo);

                            event_info.m_sensorproprety = gettransdata_sensorposinfo_by_sensingmedia(sensorulid, sensingmediaid);
                            event_info.m_distance = LocationHelper::greatDistanceM(event_info.m_sensorposinfo.PARAM_pos_hexidx, event_info.m_entityposinfo.PARAM_pos_hexidx);
                            event_info.m_entityisSensorProprety = isEntityHaveSensorProperty(entityulid);
                            if (event_info.m_entityisSensorProprety)
                            {
                                event_info.m_entityproprety = gettransdata_sensorposinfo(entityulid);
                            }
                            event_info.m_timestamp = curTimestamp;
                            updateEventlist.insert(std::make_pair(std::move(eventtile), _exist_events_map_itor->second.first));
                            //std::cout << "update event " << eventtile.sensorid << " " << eventtile.sensingmediaid << " " << eventtile.entityid << std::endl;
                        }
                    }
                    else
                    {
                        //add new event
                        auto _sensorposinfo = gettransdata_entityposinfo(sensorulid);
                        auto _entityposinfo = gettransdata_entityposinfo(entityulid);

                        auto distance = LocationHelper::greatDistanceM(_sensorposinfo.PARAM_pos_hexidx, _entityposinfo.PARAM_pos_hexidx);

                        transdata_sensorposinfo _entityproprety;
                        bool _entityisSensorProprety = isEntityHaveSensorProperty(entityulid);
                        if (_entityisSensorProprety)
                        {
                            _entityproprety = gettransdata_sensorposinfo(entityulid);
                        }

                        transdata_sensorposinfo _sensorproprety = gettransdata_sensorposinfo_by_sensingmedia(sensorulid, sensingmediaid);

                        EVENT_INFO eventinfo{ sensorulid,
                                             entityulid,
                                             sensingmediaid,
                                             std::move(_sensorposinfo),
                                             std::move(_entityposinfo),
                                             std::move(_sensorproprety),
                                             _entityisSensorProprety,
                                             std::move(_entityproprety),
                                             distance ,
                                             curTimestamp};

                        new_exist_events_map.insert(std::make_pair(eventtile, qMakePair(std::move(eventinfo), true)));
                        addEventlist.insert(std::make_pair(eventtile, new_exist_events_map.at(eventtile).first));
                        //m_ieventcount_plus++;
                        //std::cout << "add event " << m_ieventcount_plus << " " << eventtile.sensingmediaid << " " << eventtile.entityid << std::endl;
                    }
                    entitylist_itor++;
                }
                sensorlist_itor++;
            }

            intersectInfos_itor++;
        }
    }

#endif


	auto clear_exist_events_map_itor = new_exist_events_map.begin();
	while (clear_exist_events_map_itor != new_exist_events_map.end())
	{
		if (!clear_exist_events_map_itor->second.second)
		{
			EVENT_INFO & event_info = clear_exist_events_map_itor->second.first;
			event_info.m_timestamp = curTimestamp;
			clearEventlist.insert(std::make_pair(clear_exist_events_map_itor->first, clear_exist_events_map_itor->second.first));
            //m_ieventcount_sub++;
//			std::cout << "remove event " << m_ieventcount_sub << " " << clear_exist_events_map_itor->first.sensingmediaid << " " << clear_exist_events_map_itor->first.entityid << std::endl;
			clear_exist_events_map_itor = new_exist_events_map.erase(clear_exist_events_map_itor);
			continue;
		}
		clear_exist_events_map_itor++;
	}
}

void GaeactorProcessor::refresh_events_by_cores(const TYPE_ULID &entityid, const H3INDEX &h3Indexsrc,const FLOAT64 &hgt,const transdata_entityposinfo& eninfo,IDENTIFI_EVENT_INFO& identifi_event_info)
{
    EASY_FUNCTION(profiler::colors::RichRed)
    EVENTS_HASHMAP &addEventlist = std::get<0>(identifi_event_info);
    EVENTS_HASHMAP &clearEventlist = std::get<1>(identifi_event_info);
    EVENTS_HASHMAP &updateEventlist = std::get<2>(identifi_event_info);

    QWriteLocker locker(&m_exist_events_map_mutex);
    std::unordered_map<EVENT_KEY_TYPE, QPair<EVENT_INFO, bool>> &new_exist_events_map = m_exist_events_map;
    auto new_exist_events_map_itor = new_exist_events_map.begin();
    while(new_exist_events_map_itor != new_exist_events_map.end())
    {
        //当前实体的存在的事件 重置 ，其他实体的不管
        if(new_exist_events_map_itor->first.entityid == entityid)
        {
            new_exist_events_map_itor->second.second = false;
        }
        new_exist_events_map_itor++;
    }
#ifdef ENABLE_TRACE_LOG_LEVEL_EVENT_DETECT
    static UINT64 m_ieventcount_sub = 0;
    static UINT64 m_ieventcount_plus = 0;
#endif
#if 0
        UINT64 curTimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
#else
    UINT64 curTimestamp = SettingsConfig::getInstance().m_simparams.m_sim_timestamp * 1000;
#endif


    //获取当前实体存在的碰撞信息
    std::unordered_map<H3INDEX, tagIntersectInfo> intersectInfos = m_H3IndexBufferManager.getIntersectInfos_by_cores(entityid);

    //返回的结果可能必须只会存在 或者 没有
    auto intersectInfos_itor = intersectInfos.begin();
    while (intersectInfos_itor != intersectInfos.end())
    {
        std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, bool> &sensorlist = intersectInfos_itor->second.sensorlist;
        std::unordered_map<TYPE_ULID, bool> &entitylist = intersectInfos_itor->second.entitylist;

        auto sensorlist_itor = sensorlist.begin();
        while (sensorlist_itor != sensorlist.end())
        {
            const TYPE_ULID &sensorulid = sensorlist_itor->first.first;
            const TYPE_ULID &sensingmediaid = sensorlist_itor->first.second;
            auto entitylist_itor = entitylist.begin();
            while (entitylist_itor != entitylist.end())
            {
                const TYPE_ULID & entityulid = entitylist_itor->first;
                if (sensorulid == entityulid)
                {
                    entitylist_itor++;
                    continue;
                }

                //其他实体不管
                if (entityid != entityulid)
                {
                    entitylist_itor++;
                    continue;
                }

                EVENT_KEY_TYPE eventtile = EVENT_KEY_TYPE{ sensorulid, entityulid, sensingmediaid };

                auto _exist_events_map_itor = new_exist_events_map.find(eventtile);
                if (_exist_events_map_itor != new_exist_events_map.end())
                {
                    EVENT_INFO & event_info = _exist_events_map_itor->second.first;

                    _exist_events_map_itor->second.second = true;

                    //update exist event
                    auto new_sensorposinfo = gettransdata_entityposinfo(sensorulid);
                    auto new_entityposinfo = gettransdata_entityposinfo(entityulid);
                    //此处控制update事件的刷新频率，
                    //当场实体发生变化 或 运动实体发生变化 【感知位置发生变化】或 【感知位置未发生变化，但更新周期到了】上次更新时间间隔超过250ms，触发发送
                    //反之场实体未发生变化 且 运动实体未发生变化 且感知时间更新周期太短 ，则不触发该更新事件的发送
                    if(event_info.m_sensorposinfo.PARAM_pos_hexidx != new_sensorposinfo.PARAM_pos_hexidx ||
                        event_info.m_entityposinfo.PARAM_pos_hexidx != new_entityposinfo.PARAM_pos_hexidx /*||
//                        curTimestamp - event_info.m_timestamp > 250*/)
                    {
                        event_info.m_sensorposinfo = std::move(new_sensorposinfo);
                        event_info.m_entityposinfo = std::move(new_entityposinfo);

                        event_info.m_sensorproprety = gettransdata_sensorposinfo_by_sensingmedia(sensorulid, sensingmediaid);
                        event_info.m_distance = LocationHelper::greatDistanceM(event_info.m_sensorposinfo.PARAM_pos_hexidx, event_info.m_entityposinfo.PARAM_pos_hexidx);
                        event_info.m_entityisSensorProprety = isEntityHaveSensorProperty(entityulid);
                        if (event_info.m_entityisSensorProprety)
                        {
                            event_info.m_entityproprety = gettransdata_sensorposinfo(entityulid);
                        }
                        event_info.m_timestamp = curTimestamp;
                        updateEventlist.insert(std::make_pair(std::move(eventtile), _exist_events_map_itor->second.first));
#ifdef ENABLE_TRACE_LOG_LEVEL_EVENT_DETECT
                        std::stringstream ss;
                        ss << "entity_event_detect update_event " << " " << eventtile.sensorid  << " " << eventtile.sensingmediaid << " " << eventtile.entityid;
                        TRACE_LOG_PRINT_EX2(ss);
#endif

                    }
                }
                else
                {
                    //add new event
                    auto _sensorposinfo = gettransdata_entityposinfo(sensorulid);
                    auto _entityposinfo = gettransdata_entityposinfo(entityulid);

                    auto distance = LocationHelper::greatDistanceM(_sensorposinfo.PARAM_pos_hexidx, _entityposinfo.PARAM_pos_hexidx);

                    transdata_sensorposinfo _entityproprety;
                    bool _entityisSensorProprety = isEntityHaveSensorProperty(entityulid);
                    if (_entityisSensorProprety)
                    {
                        _entityproprety = gettransdata_sensorposinfo(entityulid);
                    }

                    transdata_sensorposinfo _sensorproprety = gettransdata_sensorposinfo_by_sensingmedia(sensorulid, sensingmediaid);

                    EVENT_INFO eventinfo{ sensorulid,
                                         entityulid,
                                         sensingmediaid,
                                         std::move(_sensorposinfo),
                                         std::move(_entityposinfo),
                                         std::move(_sensorproprety),
                                         _entityisSensorProprety,
                                         std::move(_entityproprety),
                                         distance ,
                                         curTimestamp};

                    new_exist_events_map.insert(std::make_pair(eventtile, qMakePair(std::move(eventinfo), true)));
                    addEventlist.insert(std::make_pair(eventtile, new_exist_events_map.at(eventtile).first));
#ifdef ENABLE_TRACE_LOG_LEVEL_EVENT_DETECT
                    m_ieventcount_plus++;
                    std::stringstream ss;
                    ss << "entity_event_detect add_event " << m_ieventcount_plus << " " << eventtile.sensorid  << " " << eventtile.sensingmediaid << " " << eventtile.entityid;
                    TRACE_LOG_PRINT_EX2(ss);
#endif
                }
                entitylist_itor++;
            }
            sensorlist_itor++;
        }

        intersectInfos_itor++;
    }


    auto clear_exist_events_map_itor = new_exist_events_map.begin();
    while (clear_exist_events_map_itor != new_exist_events_map.end())
    {
        //其他实体不管
        if(clear_exist_events_map_itor->first.entityid != entityid)
        {
            clear_exist_events_map_itor++;
            continue;
        }
        if (!clear_exist_events_map_itor->second.second)
        {
            EVENT_INFO & event_info = clear_exist_events_map_itor->second.first;
            event_info.m_timestamp = curTimestamp;
            clearEventlist.insert(std::make_pair(clear_exist_events_map_itor->first, clear_exist_events_map_itor->second.first));
#ifdef ENABLE_TRACE_LOG_LEVEL_EVENT_DETECT
            std::stringstream ss;
            m_ieventcount_sub++;
            ss << "entity_event_detect remove_event "<< m_ieventcount_sub <<" "<< clear_exist_events_map_itor->first.sensorid  << " " << clear_exist_events_map_itor->first.sensingmediaid << " " << clear_exist_events_map_itor->first.entityid;
            TRACE_LOG_PRINT_EX2(ss);
#endif
            clear_exist_events_map_itor = new_exist_events_map.erase(clear_exist_events_map_itor);
            continue;
        }
        clear_exist_events_map_itor++;
    }
}

void GaeactorProcessor::refresh_events_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmedia_id, const HEXIDX_HGT_ARRAY &hexidxslistret,IDENTIFI_EVENT_INFO& identifi_event_info)
{
    EASY_FUNCTION(profiler::colors::Green)
    EVENTS_HASHMAP &addEventlist = std::get<0>(identifi_event_info);
    EVENTS_HASHMAP &clearEventlist = std::get<1>(identifi_event_info);
    EVENTS_HASHMAP &updateEventlist = std::get<2>(identifi_event_info);

    QWriteLocker locker(&m_exist_events_map_mutex);
    std::unordered_map<EVENT_KEY_TYPE, QPair<EVENT_INFO, bool>> &new_exist_events_map = m_exist_events_map;
    auto new_exist_events_map_itor = new_exist_events_map.begin();
    while(new_exist_events_map_itor != new_exist_events_map.end())
    {
        //当前实体感知域存在的事件 重置 ，其他实体的不管
        if(((new_exist_events_map_itor->first.sensorid == sensorid) &&
             (new_exist_events_map_itor->first.sensingmediaid == sensingmedia_id)))
        {
            new_exist_events_map_itor->second.second = false;
        }
        new_exist_events_map_itor++;
    }
#ifdef ENABLE_TRACE_LOG_LEVEL_EVENT_DETECT
    static UINT64 m_ieventcount_sub = 0;
    static UINT64 m_ieventcount_plus = 0;
#endif

#if 0
        UINT64 curTimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
#else
    UINT64 curTimestamp = SettingsConfig::getInstance().m_simparams.m_sim_timestamp * 1000;
#endif


    //获取实体感知域存在的碰撞信息
    std::unordered_map<H3INDEX, tagIntersectInfo> intersectInfos = m_H3IndexBufferManager.getIntersectInfos_by_sensors(sensorid, sensingmedia_id);

    //返回的结果可能必须只会存在 或者 没有
    auto intersectInfos_itor = intersectInfos.begin();
    while (intersectInfos_itor != intersectInfos.end())
    {
        std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, bool> &sensorlist = intersectInfos_itor->second.sensorlist;
        std::unordered_map<TYPE_ULID, bool> &entitylist = intersectInfos_itor->second.entitylist;

        auto sensorlist_itor = sensorlist.begin();
        while (sensorlist_itor != sensorlist.end())
        {
            const TYPE_ULID &sensorulid = sensorlist_itor->first.first;
            const TYPE_ULID &sensingmediaid = sensorlist_itor->first.second;

            //当前实体感知域存在的事件 重置 ，其他实体的不管
            if(!((sensorulid == sensorid) && (sensingmediaid == sensingmedia_id)))
            {
                sensorlist_itor++;
                continue;
            }

            auto entitylist_itor = entitylist.begin();
            while (entitylist_itor != entitylist.end())
            {
                const TYPE_ULID & entityulid = entitylist_itor->first;
                if (sensorulid == entityulid)
                {
                    entitylist_itor++;
                    continue;
                }
                EVENT_KEY_TYPE eventtile = EVENT_KEY_TYPE{ sensorulid, entityulid, sensingmediaid };

                auto _exist_events_map_itor = new_exist_events_map.find(eventtile);
                if (_exist_events_map_itor != new_exist_events_map.end())
                {
                    EVENT_INFO & event_info = _exist_events_map_itor->second.first;

                    _exist_events_map_itor->second.second = true;

                    //update exist event
                    auto new_sensorposinfo = gettransdata_entityposinfo(sensorulid);
                    auto new_entityposinfo = gettransdata_entityposinfo(entityulid);
                    //此处控制update事件的刷新频率，
                    //当场实体发生变化 或 运动实体发生变化 【感知位置发生变化】或 【感知位置未发生变化，但更新周期到了】上次更新时间间隔超过250ms，触发发送
                    //反之场实体未发生变化 且 运动实体未发生变化 且感知时间更新周期太短 ，则不触发该更新事件的发送
                    if(event_info.m_sensorposinfo.PARAM_pos_hexidx != new_sensorposinfo.PARAM_pos_hexidx ||
                        event_info.m_entityposinfo.PARAM_pos_hexidx != new_entityposinfo.PARAM_pos_hexidx /*||
//                        curTimestamp - event_info.m_timestamp > 250*/)
                    {
                        event_info.m_sensorposinfo = std::move(new_sensorposinfo);
                        event_info.m_entityposinfo = std::move(new_entityposinfo);

                        event_info.m_sensorproprety = gettransdata_sensorposinfo_by_sensingmedia(sensorulid, sensingmediaid);
                        event_info.m_distance = LocationHelper::greatDistanceM(event_info.m_sensorposinfo.PARAM_pos_hexidx, event_info.m_entityposinfo.PARAM_pos_hexidx);
                        event_info.m_entityisSensorProprety = isEntityHaveSensorProperty(entityulid);
                        if (event_info.m_entityisSensorProprety)
                        {
                            event_info.m_entityproprety = gettransdata_sensorposinfo(entityulid);
                        }
                        event_info.m_timestamp = curTimestamp;
                        updateEventlist.insert(std::make_pair(std::move(eventtile), _exist_events_map_itor->second.first));
#ifdef ENABLE_TRACE_LOG_LEVEL_EVENT_DETECT
                        std::stringstream ss;
                        ss << "sensor_event_detect update_event " << " " << eventtile.sensorid  << " " << eventtile.sensingmediaid << " " << eventtile.entityid;
                        TRACE_LOG_PRINT_EX2(ss);
#endif
                    }
                }
                else
                {
                    //add new event
                    auto _sensorposinfo = gettransdata_entityposinfo(sensorulid);
                    auto _entityposinfo = gettransdata_entityposinfo(entityulid);

                    auto distance = LocationHelper::greatDistanceM(_sensorposinfo.PARAM_pos_hexidx, _entityposinfo.PARAM_pos_hexidx);

                    transdata_sensorposinfo _entityproprety;
                    bool _entityisSensorProprety = isEntityHaveSensorProperty(entityulid);
                    if (_entityisSensorProprety)
                    {
                        _entityproprety = gettransdata_sensorposinfo(entityulid);
                    }

                    transdata_sensorposinfo _sensorproprety = gettransdata_sensorposinfo_by_sensingmedia(sensorulid, sensingmediaid);

                    EVENT_INFO eventinfo{ sensorulid,
                                         entityulid,
                                         sensingmediaid,
                                         std::move(_sensorposinfo),
                                         std::move(_entityposinfo),
                                         std::move(_sensorproprety),
                                         _entityisSensorProprety,
                                         std::move(_entityproprety),
                                         distance ,
                                         curTimestamp};

                    new_exist_events_map.insert(std::make_pair(eventtile, qMakePair(std::move(eventinfo), true)));
                    addEventlist.insert(std::make_pair(eventtile, new_exist_events_map.at(eventtile).first));
#ifdef ENABLE_TRACE_LOG_LEVEL_EVENT_DETECT
                    m_ieventcount_plus++;
                    std::stringstream ss;
                    ss << "sensor_event_detect add_event " << m_ieventcount_plus << " " << eventtile.sensorid  << " " << eventtile.sensingmediaid << " " << eventtile.entityid;
                    TRACE_LOG_PRINT_EX2(ss);
#endif
                }
                entitylist_itor++;
            }
            sensorlist_itor++;
        }

        intersectInfos_itor++;
    }


    auto clear_exist_events_map_itor = new_exist_events_map.begin();
    while (clear_exist_events_map_itor != new_exist_events_map.end())
    {
        //当前实体感知域存在的事件 重置 ，其他实体的不管
        if(!((clear_exist_events_map_itor->first.sensorid == sensorid) &&
              (clear_exist_events_map_itor->first.sensingmediaid == sensingmedia_id)))
        {
            clear_exist_events_map_itor++;
            continue;
        }

        if (!clear_exist_events_map_itor->second.second)
        {
            EVENT_INFO & event_info = clear_exist_events_map_itor->second.first;
            event_info.m_timestamp = curTimestamp;
            clearEventlist.insert(std::make_pair(clear_exist_events_map_itor->first, clear_exist_events_map_itor->second.first));
#ifdef ENABLE_TRACE_LOG_LEVEL_EVENT_DETECT
            std::stringstream ss;
            m_ieventcount_sub++;
            ss << "sensor_event_detect remove_event "<< m_ieventcount_sub <<" "<< clear_exist_events_map_itor->first.sensorid  << " " << clear_exist_events_map_itor->first.sensingmediaid << " " << clear_exist_events_map_itor->first.entityid;
            TRACE_LOG_PRINT_EX2(ss);
#endif
            clear_exist_events_map_itor = new_exist_events_map.erase(clear_exist_events_map_itor);
            continue;
        }
        clear_exist_events_map_itor++;
    }
}

void GaeactorProcessor::tagHexindexlistSensorInfo::deal_silent_time_out(HEXIDX_ARRAY &clearhexidxslist, UINT64 currentTimeStamp)
{
    EASY_FUNCTION(profiler::colors::Green)
	clearhexidxslist.clear();
    if (this->m_bValid && this->m_silent_time != 0)
	{
        clearhexidxslist.reserve(m_hexIndexslist.size());
        {
            QWriteLocker locker(&m_hexIndexslist_mutex);
            auto hexindexslist_itor = m_hexIndexslist.begin();
            while (hexindexslist_itor != m_hexIndexslist.end())
            {
                if (!hexindexslist_itor->second.m_bHexidxValid)
                {
                    hexindexslist_itor++;
                    continue;
                }
                else
                {
                    if (fabs(currentTimeStamp - hexindexslist_itor->second.m_timestamp) > this->m_silent_time)
                    {
                        hexindexslist_itor->second.m_bHexidxValid = !hexindexslist_itor->second.m_bHexidxValid;
                        clearhexidxslist.push_back(hexindexslist_itor->first);

                        hexindexslist_itor = m_hexIndexslist.erase(hexindexslist_itor);
                        continue;
                    }
                }
                hexindexslist_itor++;
            }
        }
        {

            QReadLocker locker(&m_hexIndexslist_mutex);
            if (m_hexIndexslist.empty())
            {
                this->m_bValid = false;
            }
        }
	}
}

void GaeactorProcessor::tagHexindexlistSensorInfo::deal_clear_all_hexindexs(HEXIDX_ARRAY &clearhexidxslist)
{
	clearhexidxslist.clear();
	{
		clearhexidxslist.reserve(this->m_hexIndexslist.size());
		QReadLocker locker(&m_hexIndexslist_mutex);
		auto h3indexlist_itor = this->m_hexIndexslist.begin();
		while (h3indexlist_itor != this->m_hexIndexslist.end())
		{
			clearhexidxslist.push_back(h3indexlist_itor->first);
			h3indexlist_itor->second.m_bHexidxValid = false;
			h3indexlist_itor++;
		}
	}
	{
		QWriteLocker locker(&m_hexIndexslist_mutex);
		this->m_hexIndexslist.clear();
	}

	{

		QReadLocker locker(&m_hexIndexslist_mutex);
		if (m_hexIndexslist.empty())
		{
			this->m_bValid = false;
		}
	}
}

void GaeactorProcessor::tagHexindexlistSensorInfo::deal_clear_hexindexs_by_h3list(HEXIDX_ARRAY& clearhexidxslist, const HEXIDX_ARRAY &hexidxslist)
{
	clearhexidxslist.clear();
	//新的存在 旧的不存在 / 新的存在 旧的存在但被设置无效的

	clearhexidxslist.reserve(m_hexIndexslist.size());
	for (auto h3 : hexidxslist)
	{
		QWriteLocker locker(&m_hexIndexslist_mutex);
		auto oldh3indexlist_itor = this->m_hexIndexslist.find(h3);
		if (oldh3indexlist_itor != this->m_hexIndexslist.end())
		{
			if (oldh3indexlist_itor->second.m_bHexidxValid)
			{
				oldh3indexlist_itor->second.m_bHexidxValid = false;
				clearhexidxslist.push_back(h3);
				this->m_hexIndexslist.erase(oldh3indexlist_itor);
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////////
	if (this->m_hexIndexslist.empty())
	{
		this->m_bValid = false;
	}
}

void GaeactorProcessor::tagHexindexlistSensorInfo::deal_invalid_hexindexs_by_h3list(HEXIDX_ARRAY &invalidhexidxslist, const HEXIDX_ARRAY &hexidxslist)
{
	invalidhexidxslist.clear();
	//清理旧的存在 新的不存在 的无效的
	invalidhexidxslist.reserve(this->m_hexIndexslist.size());
	///////////////////////////////////////////////////////////////////////////////
	QWriteLocker locker(&m_hexIndexslist_mutex);
	auto oldh3indexlist_itor = this->m_hexIndexslist.begin();
	while (oldh3indexlist_itor != this->m_hexIndexslist.end())
	{
		const H3INDEX & oldh3 = oldh3indexlist_itor->first;
		bool & bValid = oldh3indexlist_itor->second.m_bHexidxValid;
		if (bValid)
		{
			auto newhexidxslist_itor = std::find_if(hexidxslist.begin(),
													hexidxslist.end(), [&oldh3](const HEXIDX_ARRAY::value_type&vt) {
														return vt == oldh3;
													});
			if (newhexidxslist_itor == hexidxslist.end())
			{
				bValid = false;
				invalidhexidxslist.push_back(oldh3);
				oldh3indexlist_itor = this->m_hexIndexslist.erase(oldh3indexlist_itor);
				continue;
			}
		}
		oldh3indexlist_itor++;
	}
}

void GaeactorProcessor::tagHexindexlistSensorInfo::deal_invalid_hexindexs_by_h3list(HEXIDX_ARRAY &invalidhexidxslist, const HEXIDX_HGT_ARRAY &_hexidxs)
{
	invalidhexidxslist.clear();
	//清理旧的存在 新的不存在 的无效的
	invalidhexidxslist.reserve(this->m_hexIndexslist.size());

	///////////////////////////////////////////////////////////////////////////////
	QWriteLocker locker(&m_hexIndexslist_mutex);
	auto oldh3indexlist_itor = this->m_hexIndexslist.begin();
	while (oldh3indexlist_itor != this->m_hexIndexslist.end())
	{
		const H3INDEX & oldh3 = oldh3indexlist_itor->first;
		bool & bValid = oldh3indexlist_itor->second.m_bHexidxValid;
		if (bValid)
		{
			auto newhexidxslist_itor = std::find_if(_hexidxs.begin(),
													_hexidxs.end(), [&oldh3](const HEXIDX_HGT_ARRAY::value_type&vt) {
														return vt.PARAM_seq_hexidx_element == oldh3;
													});
			if (newhexidxslist_itor == _hexidxs.end())
			{
				bValid = false;
				invalidhexidxslist.push_back(oldh3);

				oldh3indexlist_itor = this->m_hexIndexslist.erase(oldh3indexlist_itor);
				continue;
			}
		}
		oldh3indexlist_itor++;
	}
}

void GaeactorProcessor::tagHexindexlistSensorInfo::deal_newappend_hexindexs_by_h3list(HEXIDX_ARRAY &newapendhexidxslist, const HEXIDX_ARRAY &hexidxslist, const UINT64 &currentTimeStamp)
{
	newapendhexidxslist.clear();
	//清理旧的存在 新的不存在 的无效的
	newapendhexidxslist.reserve(hexidxslist.size());
	///////////////////////////////////////////////////////////////////////////////
	//新的存在 旧的不存在 / 新的存在 旧的存在但被设置无效的
	for (auto h3 : hexidxslist)
	{
		bool bNewAppend = true;
		{
			QWriteLocker locker(&m_hexIndexslist_mutex);
			auto newoldh3indexlist_itor = this->m_hexIndexslist.find(h3);
			if (newoldh3indexlist_itor != this->m_hexIndexslist.end())
			{
				if (!newoldh3indexlist_itor->second.m_bHexidxValid)
				{
					newoldh3indexlist_itor->second.m_timestamp = currentTimeStamp;
					newoldh3indexlist_itor->second.m_bHexidxValid = true;
					newapendhexidxslist.push_back(h3);
				}
				else
				{
					newoldh3indexlist_itor->second.m_timestamp = currentTimeStamp;
				}
				bNewAppend = false;
			}
		}

		if(bNewAppend)
		{
			newapendhexidxslist.push_back(h3);
			append_hexindexinfo(h3, currentTimeStamp);
		}
	}
}

void GaeactorProcessor::tagHexindexlistSensorInfo::deal_newappend_hexindexs_by_h3list(HEXIDX_ARRAY &newapendhexidxslist, const HEXIDX_HGT_ARRAY &_hexidxs, const UINT64 &currentTimeStamp)
{
	newapendhexidxslist.clear();
	//清理旧的存在 新的不存在 的无效的
	newapendhexidxslist.reserve(_hexidxs.size());
	for (auto _hexidxs_item : _hexidxs)
	{
		const H3INDEX& h3 = _hexidxs_item.PARAM_seq_hexidx_element;
		bool bNewAppend = true;
		{
			QWriteLocker locker(&m_hexIndexslist_mutex);
			auto newoldh3indexlist_itor = this->m_hexIndexslist.find(h3);
			if (newoldh3indexlist_itor != this->m_hexIndexslist.end())
			{
				if (!newoldh3indexlist_itor->second.m_bHexidxValid)
				{
					newoldh3indexlist_itor->second.m_timestamp = currentTimeStamp;
					newoldh3indexlist_itor->second.m_bHexidxValid = true;
					newoldh3indexlist_itor->second.m_HexidxInfo = _hexidxs_item;
					newapendhexidxslist.push_back(h3);
				}
				else
				{
					newoldh3indexlist_itor->second.m_timestamp = currentTimeStamp;
				}

				bNewAppend = false;
			}
		}
		if(bNewAppend)
		{
			newapendhexidxslist.push_back(h3);
			append_hexindexinfo(h3, currentTimeStamp, _hexidxs_item);
		}
	}
}

void GaeactorProcessor::tagHexindexlistSensorInfo::append_hexindexinfo(const H3INDEX &h3, const UINT64 &currentTimeStamp, const transdata_param_seq_hexidx & _hexidxs_item)
{
	HEXIDX_TIME_STAMP_INFO hex_time_stamp_info;
	hex_time_stamp_info.m_bHexidxValid = true;
	hex_time_stamp_info.m_timestamp = currentTimeStamp;
	hex_time_stamp_info.m_HexidxInfo = _hexidxs_item;
	{
		QWriteLocker locker(&m_hexIndexslist_mutex);
		this->m_hexIndexslist.insert(std::make_pair(h3, std::move(hex_time_stamp_info)));
	}
}

GaeactorProcessor::tagHexindexSensorInfo::tagHexindexSensorInfo(H3IndexBufferManager *_pH3IndexBufferManager)
    :m_pH3IndexBufferManager(_pH3IndexBufferManager)
{

}

GaeactorProcessor::tagHexindexSensorInfo::~tagHexindexSensorInfo()
{
	QWriteLocker locker(&m_sensor_hexidx_info_map_mutex);
	auto _sensor_hexidx_info_map_itor = this->m_sensor_hexidx_info_map.begin();
	while (_sensor_hexidx_info_map_itor != this->m_sensor_hexidx_info_map.end())
	{
		tagHexindexlistSensorInfo* ptagHexindexlistSensorInfo = _sensor_hexidx_info_map_itor->second;
		if(ptagHexindexlistSensorInfo)
		{
			delete ptagHexindexlistSensorInfo;
		}

		_sensor_hexidx_info_map_itor++;
	}
	this->m_sensor_hexidx_info_map.clear();
}

void GaeactorProcessor::tagHexindexSensorInfo::append_HexindexlistSensorInfo(const TYPE_ULID &sensingmediaid, tagHexindexlistSensorInfo *info)
{
	QWriteLocker locker(&m_sensor_hexidx_info_map_mutex);
	this->m_sensor_hexidx_info_map.insert(std::make_pair(sensingmediaid, std::move(info)));
}

void GaeactorProcessor::tagHexindexSensorInfo::clear_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid)
{
	QReadLocker locker(&m_sensor_hexidx_info_map_mutex);
	auto _sensor_hexidx_info_map_itor = this->m_sensor_hexidx_info_map.find(sensingmediaid);
	if (_sensor_hexidx_info_map_itor != this->m_sensor_hexidx_info_map.end() && _sensor_hexidx_info_map_itor->second)
	{
		HEXIDX_ARRAY clearhexidxslist;
		_sensor_hexidx_info_map_itor->second->deal_clear_all_hexindexs(clearhexidxslist);
		for (auto clearitem : clearhexidxslist)
		{
            if(m_pH3IndexBufferManager)
            {
                m_pH3IndexBufferManager->deal_hexindex_sensor(sensorulid, sensingmediaid, clearitem, E_DISPLAY_MODE_WAVE, true);
            }
		}
	}
}

void GaeactorProcessor::tagHexindexSensorInfo::clear_hexindex_sensor_by_hexidexlist(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_ARRAY &hexidxslist)
{
	QReadLocker locker(&m_sensor_hexidx_info_map_mutex);
	auto _sensor_hexidx_info_map_itor = this->m_sensor_hexidx_info_map.find(sensingmediaid);
	if (_sensor_hexidx_info_map_itor != this->m_sensor_hexidx_info_map.end() && _sensor_hexidx_info_map_itor->second)
	{
		//新的存在 旧的不存在 / 新的存在 旧的存在但被设置无效的
		HEXIDX_ARRAY clearhexidxslist;
		_sensor_hexidx_info_map_itor->second->deal_clear_hexindexs_by_h3list(clearhexidxslist, hexidxslist);
		for (auto newitem : clearhexidxslist)
		{
            if(m_pH3IndexBufferManager)
            {
                m_pH3IndexBufferManager->deal_hexindex_sensor(sensorulid, sensingmediaid, newitem, E_DISPLAY_MODE_WAVE, true);
            }
		}
		///////////////////////////////////////////////////////////////////////////////
	}
}

transdata_sensorposinfo GaeactorProcessor::tagHexindexSensorInfo::gettransdata_sensorposinfo_by_sensingmedia(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid)
{
	QReadLocker locker(&m_sensor_hexidx_info_map_mutex);
	auto _sensor_hexidx_info_map_itor = this->m_sensor_hexidx_info_map.find(sensingmediaid);
	if (_sensor_hexidx_info_map_itor != this->m_sensor_hexidx_info_map.end() && _sensor_hexidx_info_map_itor->second)
	{
		//if(_sensor_hexidx_info_map_itor->second->m_bValid)
		{
			return _sensor_hexidx_info_map_itor->second->m_sensorInfo;
		}
	}
	return transdata_sensorposinfo();
}

GaeactorProcessor::tagHexindexlistSensorInfo *GaeactorProcessor::tagHexindexSensorInfo::getHexindexlistSensorInfo(const TYPE_ULID &sensingmediaid)
{
	QReadLocker locker(&m_sensor_hexidx_info_map_mutex);
	GaeactorProcessor::tagHexindexlistSensorInfo * ptagHexindexlistSensorInfo = nullptr;
	auto _sensor_hexidx_info_map_itor = this->m_sensor_hexidx_info_map.find(sensingmediaid);
	if (_sensor_hexidx_info_map_itor != this->m_sensor_hexidx_info_map.end() && _sensor_hexidx_info_map_itor->second)
	{
		ptagHexindexlistSensorInfo = _sensor_hexidx_info_map_itor->second;
	}
	return ptagHexindexlistSensorInfo;
}

void GaeactorProcessor::tagHexindexSensorInfo::refresh_silent_timeout(const TYPE_ULID &sensorulid, UINT64 currentTimeStamp)
{
	auto _sensor_hexidx_info_map_itor = this->m_sensor_hexidx_info_map.begin();
	while (_sensor_hexidx_info_map_itor != this->m_sensor_hexidx_info_map.end() && _sensor_hexidx_info_map_itor->second)
	{
		const TYPE_ULID &sensingmediaid = _sensor_hexidx_info_map_itor->first;
		if (_sensor_hexidx_info_map_itor->second->m_bValid)
		{
			if (_sensor_hexidx_info_map_itor->second->m_silent_time == 0)
			{
				_sensor_hexidx_info_map_itor++;
				continue;
			}
			else
			{
				HEXIDX_ARRAY clearhexidxslist;
				_sensor_hexidx_info_map_itor->second->deal_silent_time_out(clearhexidxslist, currentTimeStamp);
				for (auto clearitem : clearhexidxslist)
                {
                    if(m_pH3IndexBufferManager)
                    {
                        m_pH3IndexBufferManager->deal_hexindex_sensor(sensorulid, sensingmediaid, clearitem, E_DISPLAY_MODE_WAVE, true);
                    }
				}
			}
		}
		_sensor_hexidx_info_map_itor++;
	}
}
}
