#include "gaeactor_processor.h"
#include "src/OriginalDateTime.h"
#include <iostream>

#include "LocationHelper.h"
#include <h3Index.h>
#include <QTimer>

#include "easy/profiler.h"
#include "loghelper.h"
#include <sstream>
#include "runningmodeconfig.h"
#include <tbb/global_control.h>
#include "settingsconfig.h"

#define SUB_SIZE (7)

#define USING_LIST_TMP

#define H3_RES_BC_MASK ((uint64_t)(2047) << H3_BC_OFFSET)

template<typename T, typename U>
void get_keys(const T& srcmap, U& keys)
{
    keys.clear();
    keys.reserve(srcmap.size());
    typename  T::const_iterator _exist_events_map_itor = srcmap.begin();
    while (_exist_events_map_itor != srcmap.end())
    {
        keys.push_back(_exist_events_map_itor->first);
        _exist_events_map_itor++;
    }
}


#define PARALLEL_BEGIN \
tbb::parallel_for(tbb::blocked_range<size_t>(0, keys.size()), \
    [&](const tbb::blocked_range<size_t>& r) {\
            for (size_t i = r.begin(); i != r.end(); ++i)\
        {\
                const auto& key = keys[i];


#define PARALLEL_END \
        }\
    });


namespace gaeactorenvironment
{
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HexIdexInfo::HexIdexInfo()
    :m_h3Index(H3_INIT),
    m_eHexidexStatus(E_HEXINDEX_STATUS_FREE)
{
    m_bValid.store(false);
}
HexIdexInfo::~HexIdexInfo()
{

}


HexIdexInfo::HexIdexInfo(const HexIdexInfo &other)
{
    this->m_h3Index = other.m_h3Index;
    this->m_h3CellInfo = other.m_h3CellInfo;
    this->m_bValid.store(other.m_bValid);

    this->m_eHexidexStatus.store(other.m_eHexidexStatus);
    this->m_sensorlist = other.m_sensorlist;
    this->m_entitylist = other.m_entitylist;
}

HexIdexInfo::HexIdexInfo(HexIdexInfo &&other)
{
    this->m_h3Index = std::move(other.m_h3Index);
    this->m_h3CellInfo = other.m_h3CellInfo;
    this->m_bValid.store(other.m_bValid);

    this->m_eHexidexStatus.store(other.m_eHexidexStatus);
    this->m_sensorlist = other.m_sensorlist;
    this->m_entitylist = other.m_entitylist;
}

HexIdexInfo &HexIdexInfo::operator=(const HexIdexInfo &other)
{
    if(this == &other)
    {
        return *this;
    }

    this->m_h3Index = other.m_h3Index;
    this->m_h3CellInfo = other.m_h3CellInfo;
    this->m_bValid.store(other.m_bValid);

    this->m_eHexidexStatus.store(other.m_eHexidexStatus);
    this->m_sensorlist = other.m_sensorlist;
    this->m_entitylist = other.m_entitylist;

    return *this;
}

void HexIdexInfo::init(const H3INDEX& h3, H3CellInfo &&h3cellinfo)
{
    m_h3Index = h3;
    m_h3CellInfo = std::move(h3cellinfo);
}

bool HexIdexInfo::updateData(const ENTITY_KEY &entity_key, const H3INDEX& h3,const FLOAT64 &hgt, const UINT8& basecell, const UINT8& resolution, const UINT64& digit_origin, E_DISPLAY_MODE eDdisplayMode, bool bRemove, bool bUpdateIntersect)
{
    EASY_FUNCTION(profiler::colors::Green)
    UINT64 digit_valid = digit_origin >> ((INDEX_MAPPING_RESOLUTION_MAX - m_h3CellInfo.resolution) * 3);
    if ((H3_INIT == m_h3Index) ||
        m_h3CellInfo.resolution > resolution ||
        m_h3CellInfo.basecell != basecell ||
        m_h3CellInfo.digit_valid != digit_valid)
    {
        return m_bValid.load();
    }

    switch (eDdisplayMode)
    {
    case E_DISPLAY_MODE_ENTITY:
    {
        update_list(bRemove, m_entitylist, entity_key, hgt);

        E_HEXINDEX_STATUS old_eHexidexStatus = m_eHexidexStatus;
        if (!m_entitylist.empty())
        {
            m_eHexidexStatus = static_cast<E_HEXINDEX_STATUS>(m_eHexidexStatus | E_HEXINDEX_STATUS_ENTITY);
        }
        else
        {
            m_eHexidexStatus = static_cast<E_HEXINDEX_STATUS>(m_eHexidexStatus & ~E_HEXINDEX_STATUS_ENTITY);
        }
        if(bUpdateIntersect)
        {
            //            if((old_eHexidexStatus != gaeactorenvironment::E_HEXINDEX_STATUS_ALL && m_eHexidexStatus == gaeactorenvironment::E_HEXINDEX_STATUS_ALL) ||
            //                (old_eHexidexStatus == gaeactorenvironment::E_HEXINDEX_STATUS_ALL && m_eHexidexStatus != gaeactorenvironment::E_HEXINDEX_STATUS_ALL))
            {
                updateIntersect();
            }
        }
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
    return m_bValid.load();
}

bool HexIdexInfo::updateData(const SENSOR_KEY &sensor_key, const transdata_param_seq_hexidx &h3, E_DISPLAY_MODE eDdisplayMode, bool bRemove)
{
    EASY_FUNCTION(profiler::colors::Green)
    if (H3_INIT == m_h3Index || m_h3Index != h3.PARAM_seq_hexidx_element)
    {
        return m_bValid.load();
    }

    switch (eDdisplayMode)
    {
    case E_DISPLAY_MODE_WAVE:
    {
        update_list(bRemove, m_sensorlist, sensor_key, h3.PARAM_seq_hexidx_hgt);

        E_HEXINDEX_STATUS old_eHexidexStatus = m_eHexidexStatus;
        if (!m_sensorlist.empty())
        {
            m_eHexidexStatus = static_cast<E_HEXINDEX_STATUS>(m_eHexidexStatus | E_HEXINDEX_STATUS_SENSOR);
        }
        else
        {
            m_eHexidexStatus = static_cast<E_HEXINDEX_STATUS>(m_eHexidexStatus & ~E_HEXINDEX_STATUS_SENSOR);
        }
        //        if((old_eHexidexStatus != gaeactorenvironment::E_HEXINDEX_STATUS_ALL && m_eHexidexStatus == gaeactorenvironment::E_HEXINDEX_STATUS_ALL) ||
        //            (old_eHexidexStatus == gaeactorenvironment::E_HEXINDEX_STATUS_ALL && m_eHexidexStatus != gaeactorenvironment::E_HEXINDEX_STATUS_ALL))
        {
            updateIntersect();
        }
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
    return m_bValid.load();
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
    if(m_eHexidexStatus == gaeactorenvironment::E_HEXINDEX_STATUS_ALL)
    {
        SENESOR_LIST_CONCURRENT_HASHMAP sensorlist = this->sensorlist();
        ENTITY_LIST_CONCURRENT_HASHMAP entitylist = this->entitylist();

        tagConcurrentIntersectInfo interscet;

        interscet.sensorlist = std::move(sensorlist);
        interscet.entitylist = std::move(entitylist);

        gaeactorenvironment::H3IndexBufferManager::getInstance().appendIntersect(m_h3Index, std::move(interscet));
    }
    else
    {
        gaeactorenvironment::H3IndexBufferManager::getInstance().removeIntersect(m_h3Index);
    }
}
SENESOR_LIST_CONCURRENT_HASHMAP HexIdexInfo::sensorlist() const
{
    return m_sensorlist;
}

ENTITY_LIST_CONCURRENT_HASHMAP HexIdexInfo::entitylist() const
{
    return m_entitylist;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

H3IndexBufferManager::H3IndexBufferManager()
{

}

H3IndexBufferManager::~H3IndexBufferManager()
{

}

H3IndexBufferManager &H3IndexBufferManager::getInstance()
{
    static H3IndexBufferManager h3indexbuffer;
    return h3indexbuffer;
}

void H3IndexBufferManager::lockIntersectForWrite()
{
    EASY_FUNCTION(profiler::colors::Green)
    m_IntersectInfoMap_update_mutex.lockForWrite();
}

void H3IndexBufferManager::lockIntersectForRead()
{
    EASY_FUNCTION(profiler::colors::Green)
    m_IntersectInfoMap_update_mutex.lockForRead();
}

void H3IndexBufferManager::unlockIntersect()
{
    EASY_FUNCTION(profiler::colors::Yellow)
    m_IntersectInfoMap_update_mutex.unlock();
}

void H3IndexBufferManager::trigger_refresh_event_by_entity_update(const TYPE_ULID& entityid, const tbb::concurrent_hash_map<UINT8, std::tuple<H3INDEX, H3INDEX> > &_hexidx_array_need_replace, IDENTIFI_EVENT_INFO &eventinfo)
{
    gaeactorenvironment::GaeactorProcessor::getInstance().trigger_refresh_event_by_entity_update(entityid, _hexidx_array_need_replace, eventinfo);
}

void H3IndexBufferManager::trigger_refresh_event_by_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const transdata_sensorposinfo &_sensorinfo, const bool &bNeedClear, const HEXIDX_HGT_ARRAY &remove_hexidxslist, const HEXIDX_HGT_ARRAY &reserve_hexidxslist, const HEXIDX_HGT_ARRAY &append_hexidxslist, IDENTIFI_EVENT_INFO &eventinfo)
{
    gaeactorenvironment::GaeactorProcessor::getInstance().trigger_refresh_event_by_sensor(sensorulid, sensingmediaid, _sensorinfo, bNeedClear, remove_hexidxslist, reserve_hexidxslist, append_hexidxslist, eventinfo);
}



H3CellInfo H3IndexBufferManager::getCellInfo(const H3INDEX& h3)
{
    H3CellInfo cellinfo;
    cellinfo.basecell = (UINT8)H3_GET_BASE_CELL(h3);
    cellinfo.resolution = (UINT8)H3_GET_RESOLUTION(h3);
    //	cellinfo.resolution_basecell = ((int)((((h3)&H3_RES_BC_MASK) >> H3_BC_OFFSET)));
#if 0
    UINT64 offset =  (H3_NUM_BITS - H3_RES_OFFSET);
    UINT64 resolution_basecell_digit_origin = (UINT64)(h3 << offset);
    cellinfo.resolution_basecell_digit_valid = resolution_basecell_digit_origin >> ((INDEX_MAPPING_RESOLUTION_MAX - cellinfo.resolution) * 3);
#else
    UINT64 offset =  (H3_NUM_BITS - H3_RESERVED_OFFSET);
    UINT64 resolution_basecell_digit_origin = (UINT64)(h3 << offset);
    cellinfo.resolution_basecell_digit_valid = resolution_basecell_digit_origin >> offset;
#endif
    UINT64 digit_origin = (UINT64)(h3)&H3_INIT;
    cellinfo.digit_valid = digit_origin >> ((INDEX_MAPPING_RESOLUTION_MAX - cellinfo.resolution) * 3);
    return cellinfo;
}

std::unordered_map<UINT64, std::tuple<H3INDEX, bool, uint32_t>> H3IndexBufferManager::getCellbuffers()
{
    std::unordered_map<UINT64, std::tuple<H3INDEX, bool, uint32_t>> ret;
    std::vector<UINT64> keys;
    get_keys(m_Cellbuffer, keys);

    for(int i = 0; i < keys.size(); i++)
    {
        auto key = keys.at(i);
        CELLBUFFER_CONCURRENT_HASHMAP_ConstAccessor _Cellbuffer_accessor;
        if (this->m_Cellbuffer.find(_Cellbuffer_accessor, key))
        {
            const HexIdexInfo &_HexIdexInfo =  _Cellbuffer_accessor->second;
            ret.insert(std::make_pair(key, std::make_tuple(_HexIdexInfo.m_h3Index, _HexIdexInfo.m_bValid.load(), (uint32_t)_HexIdexInfo.m_eHexidexStatus.load())));
        }
    }
    return ret;
}

std::unordered_map<UINT64, std::tuple<H3INDEX, bool, uint32_t, std::vector<std::tuple<TYPE_ULID, TYPE_ULID,transdata_param_seq_hexidx_hgt>>,std::vector<std::tuple<TYPE_ULID,FLOAT64>>>>  H3IndexBufferManager::getCellbuffersInfo()
{
    std::unordered_map<UINT64, std::tuple<H3INDEX, bool, uint32_t, std::vector<std::tuple<TYPE_ULID, TYPE_ULID,transdata_param_seq_hexidx_hgt>>,std::vector<std::tuple<TYPE_ULID,FLOAT64>>>>   ret;
    std::vector<UINT64> keys;
    get_keys(m_Cellbuffer, keys);

    for(int i = 0; i < keys.size(); i++)
    {
        auto key = keys.at(i);
        CELLBUFFER_CONCURRENT_HASHMAP_ConstAccessor _Cellbuffer_accessor;
        if (this->m_Cellbuffer.find(_Cellbuffer_accessor, key))
        {

            std::vector<std::tuple<TYPE_ULID, TYPE_ULID,transdata_param_seq_hexidx_hgt>> sensorlist_keys;
            auto sensorlist = _Cellbuffer_accessor->second.sensorlist();
            sensorlist_keys.reserve(sensorlist.size());
            SENESOR_LIST_CONCURRENT_HASHMAP::const_iterator _sensorlist_itor = sensorlist.begin();
            while (_sensorlist_itor != sensorlist.end())
            {
                sensorlist_keys.push_back(std::make_tuple(_sensorlist_itor->first.sensorid, _sensorlist_itor->first.sensingmediaid,_sensorlist_itor->second));
                _sensorlist_itor++;
            }


            std::vector<std::tuple<TYPE_ULID,FLOAT64>> entitylist_keys;
            auto entitylist = _Cellbuffer_accessor->second.entitylist();
            entitylist_keys.reserve(entitylist.size());
            ENTITY_LIST_CONCURRENT_HASHMAP::const_iterator _entitylist_itor = entitylist.begin();
            while (_entitylist_itor != entitylist.end())
            {
                entitylist_keys.push_back(std::make_tuple(_entitylist_itor->first.entityid,_entitylist_itor->second));
                _entitylist_itor++;
            }

            const HexIdexInfo &_HexIdexInfo =  _Cellbuffer_accessor->second;
            std::tuple<H3INDEX, bool, uint32_t, std::vector<std::tuple<TYPE_ULID, TYPE_ULID,transdata_param_seq_hexidx_hgt>>, std::vector<std::tuple<TYPE_ULID,FLOAT64>>> val = std::make_tuple(_HexIdexInfo.m_h3Index, _HexIdexInfo.m_bValid.load(), (uint32_t)_HexIdexInfo.m_eHexidexStatus.load(), std::move(sensorlist_keys), std::move(entitylist_keys));
            ret.insert(std::make_pair(key,  std::move(val)));
        }
    }
    return ret;
}

void H3IndexBufferManager::appendIntersect(const H3INDEX &h3, tagConcurrentIntersectInfo &&info)
{
    EASY_FUNCTION(profiler::colors::Green)
    INTERSECTINFO_CONCURRENT_HASHMAP_Accessor _IntersectInfoMap_accessor;
    if(m_IntersectInfoMap.find(_IntersectInfoMap_accessor, h3))
    {
        //???为什么必须得要clear 才行  ？ //tbb::concurrent_hash_map特殊之处，不然内存暴涨  std::move 的行为是追加？
        //       _IntersectInfoMap_accessor->second = std::move(info);

        //        _IntersectInfoMap_accessor->second = info;
        _IntersectInfoMap_accessor->second.sensorlist.clear();
        _IntersectInfoMap_accessor->second.entitylist.clear();
        _IntersectInfoMap_accessor->second = std::move(info);
        //        _IntersectInfoMap_accessor->second.sensorlist = std::move(info.sensorlist);
        //        _IntersectInfoMap_accessor->second.entitylist = std::move(info.entitylist);
    }
    else
    {
        m_IntersectInfoMap.insert(_IntersectInfoMap_accessor, h3);
        _IntersectInfoMap_accessor->second = std::move(info);
    }
}


void H3IndexBufferManager::removeIntersect(const H3INDEX &h3)
{
    EASY_FUNCTION(profiler::colors::Green)
    INTERSECTINFO_CONCURRENT_HASHMAP_Accessor _IntersectInfoMap_accessor;
    if(m_IntersectInfoMap.find(_IntersectInfoMap_accessor, h3))
    {
        m_IntersectInfoMap.erase(_IntersectInfoMap_accessor);
    }
}

void H3IndexBufferManager::removeIntersect_ex(const ENTITY_KEY &entity_key, const H3INDEX &h3)
{
    EASY_FUNCTION(profiler::colors::Green)
    INTERSECTINFO_CONCURRENT_HASHMAP_Accessor _IntersectInfoMap_accessor;
    if(m_IntersectInfoMap.find(_IntersectInfoMap_accessor, h3))
    {
        //移除旧的
        ENTITY_LIST_CONCURRENT_HASHMAP::accessor entitylist_accessor;
        if(_IntersectInfoMap_accessor->second.entitylist.find(entitylist_accessor, entity_key))
        {
            _IntersectInfoMap_accessor->second.entitylist.erase(entitylist_accessor);
        }
        if(_IntersectInfoMap_accessor->second.entitylist.empty())
        {
            m_IntersectInfoMap.erase(_IntersectInfoMap_accessor);
        }
    }
}

void H3IndexBufferManager::replaceIntersect(const ENTITY_KEY &entity_key,const H3INDEX &oldh3, const H3INDEX &newh3, tagConcurrentIntersectInfo &&info)
{
    INTERSECTINFO_CONCURRENT_HASHMAP_Accessor _IntersectInfoMap_accessor;
    if(m_IntersectInfoMap.find(_IntersectInfoMap_accessor, oldh3))
    {
        //移除旧的
        {
            ENTITY_LIST_CONCURRENT_HASHMAP::accessor entitylist_accessor;
            if(_IntersectInfoMap_accessor->second.entitylist.find(entitylist_accessor, entity_key))
            {
                _IntersectInfoMap_accessor->second.entitylist.erase(entitylist_accessor);
            }
            if(_IntersectInfoMap_accessor->second.entitylist.empty())
            {
                m_IntersectInfoMap.erase(_IntersectInfoMap_accessor);
            }
        }

        //增加新的
        m_IntersectInfoMap.insert(_IntersectInfoMap_accessor, newh3);
        _IntersectInfoMap_accessor->second = std::move(info);
    }
}

INTERSECTINFO_CONCURRENT_HASHMAP H3IndexBufferManager::getIntersectInfos()
{
    EASY_FUNCTION(profiler::colors::Green)

    QReadLocker _IntersectInfoMap_update_locker(&m_IntersectInfoMap_update_mutex);
    return m_IntersectInfoMap;
}


void H3IndexBufferManager::deal_hexindex_entity(const ENTITY_KEY &entity_key, const H3INDEX& h3, const FLOAT64 &hgt, bool bRemove)
{
    EASY_FUNCTION(profiler::colors::Green)
    UINT8 basecell = (UINT8)H3_GET_BASE_CELL(h3);
    UINT8 resolution = (UINT8)H3_GET_RESOLUTION(h3);
    UINT64 digit_origin = (UINT64)(h3)&H3_INIT;

    tbb::parallel_for(tbb::blocked_range<size_t>(0, resolution+1),
                      [&](const tbb::blocked_range<size_t>& r) {
                          for (size_t i = r.begin(); i != r.end(); ++i)
                          {
                              int res = i;
                              H3INDEX parentindex;
                              if (E_SUCCESS == cellToParent(h3, res, &parentindex))
                              {
                                  H3CellInfo h3cellinfo = getCellInfo(parentindex);
                                  bool bExist = false;
                                  {
                                      CELLBUFFER_CONCURRENT_HASHMAP_Accessor _Cellbuffer_accessor;

                                      if(m_Cellbuffer.find(_Cellbuffer_accessor, h3cellinfo.resolution_basecell_digit_valid))
                                      {
                                          bExist = true;
                                          bool bvalid = _Cellbuffer_accessor->second.updateData(entity_key, h3, hgt, basecell, resolution, digit_origin, E_DISPLAY_MODE_ENTITY, bRemove);
                                          if(bRemove && !bvalid &&
                                              !_Cellbuffer_accessor->second.m_bValid&&
                                              _Cellbuffer_accessor->second.m_eHexidexStatus == E_HEXINDEX_STATUS_FREE)
                                          {
                                              m_Cellbuffer.erase(_Cellbuffer_accessor);
                                          }
                                      }
                                  }
                                  if(!bExist && !bRemove)
                                  {
                                      HexIdexInfo _hexindexinfo;
                                      _hexindexinfo.init(parentindex, std::move(h3cellinfo));

                                      {
                                          CELLBUFFER_CONCURRENT_HASHMAP_Accessor _Cellbuffer_accessor;
                                          m_Cellbuffer.insert(_Cellbuffer_accessor, h3cellinfo.resolution_basecell_digit_valid);
                                          _Cellbuffer_accessor->second = std::move(_hexindexinfo);

                                          bool bvalid = _Cellbuffer_accessor->second.updateData(entity_key, h3, hgt, basecell, resolution, digit_origin, E_DISPLAY_MODE_ENTITY, bRemove);
                                      }
                                  }
                              }
                          }
                      });
}

void H3IndexBufferManager::deal_hexindex_entity_remove_old_append_new(const ENTITY_KEY &entity_key, const H3INDEX &oldh3, const H3INDEX &newh3, const FLOAT64 &hgt,tbb::concurrent_hash_map<UINT8, std::tuple<H3INDEX, H3INDEX> > &_hexidx_array_need_replace)
{
    EASY_FUNCTION(profiler::colors::Green)
    UINT8 old_resolution = (UINT8)H3_GET_RESOLUTION(oldh3);
    UINT8 new_resolution = (UINT8)H3_GET_RESOLUTION(newh3);
    
    //获取旧的位置网格和新的位置网格中，最大的分辨率
    UINT8 _resolution = (std::max)(old_resolution, new_resolution);

    auto update_hexidx_info=[&](const H3INDEX &h3, const H3INDEX &parentindex, bool bRemove, bool bUpdateIntersect = true){
        EASY_FUNCTION(profiler::colors::Green)
        UINT8 basecell = (UINT8)H3_GET_BASE_CELL(h3);
        UINT8 resolution = (UINT8)H3_GET_RESOLUTION(h3);
        UINT64 digit_origin = (UINT64)(h3)&H3_INIT;

        H3CellInfo h3cellinfo = getCellInfo(parentindex);
        bool bExist = false;
        {
            CELLBUFFER_CONCURRENT_HASHMAP_Accessor _Cellbuffer_accessor;

            if(m_Cellbuffer.find(_Cellbuffer_accessor, h3cellinfo.resolution_basecell_digit_valid))
            {
                bExist = true;
                bool bvalid = _Cellbuffer_accessor->second.updateData(entity_key, h3, hgt, basecell, resolution, digit_origin, E_DISPLAY_MODE_ENTITY, bRemove, bUpdateIntersect);
                if(bRemove && !bvalid &&
                    !_Cellbuffer_accessor->second.m_bValid&&
                    _Cellbuffer_accessor->second.m_eHexidexStatus == E_HEXINDEX_STATUS_FREE)
                {
                    m_Cellbuffer.erase(_Cellbuffer_accessor);
//                    std::cout<<" rem hex------------------ "<<" "<<std::hex<<h3<<" "<<std::dec<<resolution<<" "<<std::hex<<parentindex<<" "<<std::dec<<H3_GET_RESOLUTION(parentindex)<<" "<<m_Cellbuffer.size()<<" \n";
                }
            }
        }
        if(!bExist && !bRemove)
        {
            HexIdexInfo _hexindexinfo;
            _hexindexinfo.init(parentindex, std::move(h3cellinfo));

            {
                CELLBUFFER_CONCURRENT_HASHMAP_Accessor _Cellbuffer_accessor;
                m_Cellbuffer.insert(_Cellbuffer_accessor, h3cellinfo.resolution_basecell_digit_valid);
                _Cellbuffer_accessor->second = std::move(_hexindexinfo);
                bool bvalid = _Cellbuffer_accessor->second.updateData(entity_key, h3, hgt, basecell, resolution, digit_origin, E_DISPLAY_MODE_ENTITY, bRemove ,bUpdateIntersect);
            }
//            std::cout<<" add hex++++++++++++++++++ "<<" "<<std::hex<<h3<<" "<<std::dec<<resolution<<" "<<std::hex<<parentindex<<" "<<std::dec<<H3_GET_RESOLUTION(parentindex)<<" "<<m_Cellbuffer.size()<<" \n";
        }
    };
    //检查新旧位置映射的不同分辨率网格的情况
    tbb::parallel_for(tbb::blocked_range<size_t>(0, _resolution+1),
                      [&](const tbb::blocked_range<size_t>& r) {
                          for (UINT8 i = r.begin(); i != r.end(); ++i)
                          {
                              //遍历新旧位置能映射的分辨率等级【0~r+1】的网格
                              const UINT8 & res = i;
                              //建立不同分辨率等级下，新旧网格的对应关系【存在旧-存在新  更新】/【存在旧-不存在新  移除】/【不存在旧-存在新  添加】
                              if(oldh3 != 0 && res < old_resolution+1)
                              {
                                  //获取旧的网格索引指定分辨率的父索引
                                  H3INDEX parentindex;
                                  if (E_SUCCESS == cellToParent(oldh3, res, &parentindex))
                                  {
                                      //旧有 新不存在 移除
                                      tbb::concurrent_hash_map<UINT8,std::tuple<H3INDEX, H3INDEX>>::accessor replace_accessor;
                                      if(_hexidx_array_need_replace.find(replace_accessor, res))
                                      {
                                          //
                                          std::get<0>(replace_accessor->second) = parentindex;
                                      }
                                      else
                                      {
                                          _hexidx_array_need_replace.insert(replace_accessor,res);
                                          replace_accessor->second = std::make_tuple(parentindex, 0);
                                      }

                                  }
                              }

                              if(newh3 != 0 && res < new_resolution+1)
                              {
                                  //获取新的网格索引指定分辨率的父索引
                                  H3INDEX parentindex;
                                  if (E_SUCCESS == cellToParent(newh3, res, &parentindex))
                                  {
                                      //新有 旧不存在 新增
                                      tbb::concurrent_hash_map<UINT8,std::tuple<H3INDEX, H3INDEX>>::accessor replace_accessor;
                                      if(_hexidx_array_need_replace.find(replace_accessor, res))
                                      {
                                          std::get<1>(replace_accessor->second) = parentindex;
                                      }
                                      else
                                      {
                                          _hexidx_array_need_replace.insert(replace_accessor,res);
                                          replace_accessor->second = std::make_tuple(0, parentindex);
                                      }
                                  }
                              }
                          }
                      });

    std::vector<UINT8> keys;
    get_keys(_hexidx_array_need_replace, keys);
#if 0

    for (UINT8 i = 0; i < keys.size(); ++i)
    {
        const UINT8& res = keys[i];

        tbb::concurrent_hash_map<UINT8,std::tuple<H3INDEX, H3INDEX>>::const_accessor replace_accessor;

        if(_hexidx_array_need_replace.find(replace_accessor,res))
        {
            const std::tuple<H3INDEX, H3INDEX>& replace_info = replace_accessor->second;
            const H3INDEX& old_hexidx = std::get<0>(replace_info);
            const H3INDEX& new_hexidx = std::get<1>(replace_info);

            if(old_hexidx != 0 || new_hexidx != 0)
            {
                E_EVENT_MODE _E_EVENT_MODE = E_EVENT_MODE_NULL;
                if(old_hexidx != 0 && new_hexidx != 0 && new_hexidx != old_hexidx)
                {
                    _E_EVENT_MODE = E_EVENT_MODE_UPDATE;
                }
                else if(old_hexidx == 0 && new_hexidx != 0)
                {
                    _E_EVENT_MODE = E_EVENT_MODE_ADD;
                }
                else if(old_hexidx != 0 && new_hexidx == 0)
                {
                    _E_EVENT_MODE = E_EVENT_MODE_REMOVE;
                }

                switch (_E_EVENT_MODE)
                {
                case E_EVENT_MODE_ADD:
                {
                    //旧的不存在 新的存在 新增
                    update_hexidx_info(newh3, new_hexidx, false);
                }
                break;
                case E_EVENT_MODE_UPDATE:
                {
                    update_hexidx_info(oldh3, old_hexidx, true, false);
                    update_hexidx_info(newh3, new_hexidx, false, false);

                    {
                        //此处添加交互关系map ，是因为存在更新，避免出现移除再添加时候的中间时间差，导致产生多余的移除再添加事件，当同个飞机的h3索引更新，移除之前先上锁，添加之后再解锁

                        //H3CellInfo old_h3cellinfo = getCellInfo(old_hexidx);
                        H3CellInfo new_h3cellinfo = getCellInfo(new_hexidx);
                        CELLBUFFER_CONCURRENT_HASHMAP_ConstAccessor _Cellbuffer_constaccessor;

                        if(m_Cellbuffer.find(_Cellbuffer_constaccessor, new_h3cellinfo.resolution_basecell_digit_valid))
                        {
                            if(_Cellbuffer_constaccessor->second.m_eHexidexStatus == E_HEXINDEX_STATUS_ALL)
                            {
                                SENESOR_LIST_CONCURRENT_HASHMAP sensorlist = _Cellbuffer_constaccessor->second.sensorlist();
                                ENTITY_LIST_CONCURRENT_HASHMAP entitylist = _Cellbuffer_constaccessor->second.entitylist();

                                tagConcurrentIntersectInfo interscet;

                                interscet.sensorlist = std::move(sensorlist);
                                interscet.entitylist = std::move(entitylist);
                                if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
                                {
                                    gaeactorenvironment::H3IndexBufferManager::getInstance().lockIntersectForWrite();
                                }
                                gaeactorenvironment::H3IndexBufferManager::getInstance().replaceIntersect(entity_key, old_hexidx, new_hexidx, std::move(interscet));
                                if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
                                {
                                    gaeactorenvironment::H3IndexBufferManager::getInstance().unlockIntersect();
                                }
                            }
                            else
                            {
                                gaeactorenvironment::H3IndexBufferManager::getInstance().removeIntersect_ex(entity_key,old_hexidx);
                            }
                        }

                    }
                }
                break;
                case E_EVENT_MODE_REMOVE:
                {
                    //旧的存在 新的不存在 移除
                    update_hexidx_info(oldh3, old_hexidx, true);
                }
                break;
                default:
                {
                    //均存在 且一致 保留更新
//                    update_hexidx_info(newh3, new_hexidx, false);
                }
                break;
                }
            }
        }
    }
#else
    tbb::parallel_for(tbb::blocked_range<size_t>(0, keys.size()),
                      [&](const tbb::blocked_range<size_t>& r) {
                          for (UINT8 i = r.begin(); i != r.end(); ++i)
                          {
                              const UINT8& res = keys[i];
                              //遍历所有存在的分辨率

                              tbb::concurrent_hash_map<UINT8,std::tuple<H3INDEX, H3INDEX>>::const_accessor replace_accessor;

                              if(_hexidx_array_need_replace.find(replace_accessor,res))
                              {
                                  const std::tuple<H3INDEX, H3INDEX>& replace_info = replace_accessor->second;
                                  const H3INDEX& old_hexidx = std::get<0>(replace_info);
                                  const H3INDEX& new_hexidx = std::get<1>(replace_info);
                                  //获取该分辨率下的新旧网格索引
                                  if(old_hexidx != 0 || new_hexidx != 0)
                                  {
                                      E_EVENT_MODE _E_EVENT_MODE = E_EVENT_MODE_NULL;
                                      //判断新旧网格的对应关系【存在旧-存在新  更新】/【存在旧-不存在新  移除】/【不存在旧-存在新  添加】
                                      if(old_hexidx != 0 && new_hexidx != 0 && new_hexidx != old_hexidx)
                                      {
                                          _E_EVENT_MODE = E_EVENT_MODE_UPDATE;
                                      }
                                      else if(old_hexidx == 0 && new_hexidx != 0)
                                      {
                                          _E_EVENT_MODE = E_EVENT_MODE_ADD;
                                      }
                                      else if(old_hexidx != 0 && new_hexidx == 0)
                                      {
                                          _E_EVENT_MODE = E_EVENT_MODE_REMOVE;
                                      }

                                      switch (_E_EVENT_MODE)
                                      {
                                      case E_EVENT_MODE_ADD:
                                      {
                                          //旧的不存在 新的存在 新增
                                          update_hexidx_info(newh3, new_hexidx, false);
                                      }
                                      break;
                                      case E_EVENT_MODE_UPDATE:
                                      {
                                          update_hexidx_info(oldh3, old_hexidx, true, false);
                                          update_hexidx_info(newh3, new_hexidx, false, false);

                                          {
                                              //此处添加交互关系map ，是因为存在更新，避免出现移除再添加时候的中间时间差，导致产生多余的移除再添加事件，当同个飞机的h3索引更新，移除之前先上锁，添加之后再解锁

                                              //H3CellInfo old_h3cellinfo = getCellInfo(old_hexidx);
                                              H3CellInfo new_h3cellinfo = getCellInfo(new_hexidx);
                                              CELLBUFFER_CONCURRENT_HASHMAP_ConstAccessor _Cellbuffer_constaccessor;

                                              if(m_Cellbuffer.find(_Cellbuffer_constaccessor, new_h3cellinfo.resolution_basecell_digit_valid))
                                              {
                                                  if(_Cellbuffer_constaccessor->second.m_eHexidexStatus == E_HEXINDEX_STATUS_ALL)
                                                  {
                                                      SENESOR_LIST_CONCURRENT_HASHMAP sensorlist = _Cellbuffer_constaccessor->second.sensorlist();
                                                      ENTITY_LIST_CONCURRENT_HASHMAP entitylist = _Cellbuffer_constaccessor->second.entitylist();

                                                      tagConcurrentIntersectInfo interscet;

                                                      interscet.sensorlist = std::move(sensorlist);
                                                      interscet.entitylist = std::move(entitylist);
                                                      if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
                                                      {
                                                          gaeactorenvironment::H3IndexBufferManager::getInstance().lockIntersectForWrite();
                                                      }
                                                      gaeactorenvironment::H3IndexBufferManager::getInstance().replaceIntersect(entity_key, old_hexidx, new_hexidx, std::move(interscet));
                                                      if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
                                                      {
                                                          gaeactorenvironment::H3IndexBufferManager::getInstance().unlockIntersect();
                                                      }
                                                  }
                                                  else
                                                  {
                                                      gaeactorenvironment::H3IndexBufferManager::getInstance().removeIntersect_ex(entity_key,old_hexidx);
                                                  }
                                              }
                                          }
                                      }
                                      break;
                                      case E_EVENT_MODE_REMOVE:
                                      {
                                          //旧的存在 新的不存在 移除
                                          update_hexidx_info(oldh3, old_hexidx, true);
                                      }
                                      break;
                                      default:
                                      {
                                          //均存在 且一致 保留更新
                                          update_hexidx_info(newh3, new_hexidx, false);
                                      }
                                      break;
                                      }
                                  }
                              }
                          }
                      });
#endif
}

void H3IndexBufferManager::deal_hexindex_sensor(const SENSOR_KEY &sensor_key, const transdata_param_seq_hexidx& h3, bool bRemove)
{
    EASY_FUNCTION(profiler::colors::Green)
    H3CellInfo h3cellinfo = getCellInfo(h3.PARAM_seq_hexidx_element);
    bool bExist = false;
    {
        CELLBUFFER_CONCURRENT_HASHMAP_Accessor _Cellbuffer_accessor;
        if(m_Cellbuffer.find(_Cellbuffer_accessor, h3cellinfo.resolution_basecell_digit_valid))
        {
            bExist = true;
            bool bvalid =_Cellbuffer_accessor->second.updateData(sensor_key, h3, E_DISPLAY_MODE_WAVE, bRemove);
            if(bRemove && !bvalid &&
                !_Cellbuffer_accessor->second.m_bValid&&
                _Cellbuffer_accessor->second.m_eHexidexStatus == E_HEXINDEX_STATUS_FREE)
            {
                m_Cellbuffer.erase(_Cellbuffer_accessor);
                //            std::cout<<" rem hex------------------ "<<" "<<std::hex<<h3<<" "<<std::dec<<std::dec<<H3_GET_RESOLUTION(h3)<<" "<<m_Cellbuffer.size()<<" \n";
            }
        }
    }

    if(!bExist && !bRemove)
    {
        HexIdexInfo _hexindexinfo;
        _hexindexinfo.init(h3.PARAM_seq_hexidx_element, std::move(h3cellinfo));
        {
            CELLBUFFER_CONCURRENT_HASHMAP_Accessor _Cellbuffer_accessor;
            m_Cellbuffer.insert(_Cellbuffer_accessor, h3cellinfo.resolution_basecell_digit_valid);
            _Cellbuffer_accessor->second = std::move(_hexindexinfo);

            bool bvalid = _Cellbuffer_accessor->second.updateData(sensor_key, h3, E_DISPLAY_MODE_WAVE, bRemove);
        }
        //                std::cout<<" add hex++++++++++++++++++ "<<" "<<std::hex<<h3<<" "<<std::dec<<H3_GET_RESOLUTION(h3)<<" "<<m_Cellbuffer.size()<<" \n";
    }
}

void H3IndexBufferManager::reset()
{
    std::vector<UINT64> keys;
    get_keys(m_Cellbuffer, keys);

    for(int i = 0; i < keys.size(); i++)
    {
        auto key = keys.at(i);
        CELLBUFFER_CONCURRENT_HASHMAP_Accessor _Cellbuffer_accessor;
        if (this->m_Cellbuffer.find(_Cellbuffer_accessor, key))
        {

            std::vector<SENSOR_KEY> sensorlist_keys;
            get_keys(_Cellbuffer_accessor->second.sensorlist(), sensorlist_keys);

            for(int j = 0; j < sensorlist_keys.size(); j++)
            {
                const SENSOR_KEY &sensor_key = sensorlist_keys.at(j);
                transdata_param_seq_hexidx _transdata_param_seq_hexidx{_Cellbuffer_accessor->second.m_h3Index, transdata_param_seq_hexidx_hgt{0,0,0}};
                bool bvalid =_Cellbuffer_accessor->second.updateData(sensor_key, _transdata_param_seq_hexidx, E_DISPLAY_MODE_WAVE, true);
                if(!bvalid &&
                    !_Cellbuffer_accessor->second.m_bValid&&
                    _Cellbuffer_accessor->second.m_eHexidexStatus == E_HEXINDEX_STATUS_FREE)
                {
                    m_Cellbuffer.erase(_Cellbuffer_accessor);
                    //            std::cout<<" rem hex------------------ "<<" "<<std::hex<<h3<<" "<<std::dec<<std::dec<<H3_GET_RESOLUTION(h3)<<" "<<m_Cellbuffer.size()<<" \n";
                }
            }
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GaeactorProcessor::GaeactorProcessor()
{
    //    // 设置线程池大小为 4
    //    tbb::global_control control(tbb::global_control::max_allowed_parallelism, 4);
}

GaeactorProcessor::~GaeactorProcessor()
{
}

GaeactorProcessor& GaeactorProcessor::getInstance()
{
    static GaeactorProcessor gaeactorprocessor;
    return gaeactorprocessor;
}


void GaeactorProcessor::cleardata()
{
    std::vector<TYPE_ULID> keys;
    get_keys(this->m_sensorInfosMap, keys);


    PARALLEL_BEGIN
        SENSORINFOS_CONCURRENT_HASHMAP_ConstAccessor _sensorInfosMap_constaccessor;
    if (this->m_sensorInfosMap.find(_sensorInfosMap_constaccessor, key))
    {
        tagHexindexSensorInfo* ptagHexindexSensorInfo =  _sensorInfosMap_constaccessor->second;
        if(ptagHexindexSensorInfo)
        {
            ptagHexindexSensorInfo->clear_data();
            delete ptagHexindexSensorInfo;
        }
        this->m_sensorInfosMap.erase(_sensorInfosMap_constaccessor);
    }
    PARALLEL_END
}

void GaeactorProcessor::clearSensorInfo(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid)
{
    SENSORINFOS_CONCURRENT_HASHMAP_ConstAccessor _sensorInfosMap_constaccessor;
    if (this->m_sensorInfosMap.find(_sensorInfosMap_constaccessor, sensorulid))
    {
        bool bRemove = false;
        tagHexindexSensorInfo* ptagHexindexSensorInfo = _sensorInfosMap_constaccessor->second;
        if(ptagHexindexSensorInfo)
        {
            bRemove = ptagHexindexSensorInfo->clear_hexindex_sensor(sensingmediaid);
        }
        if(bRemove)
        {
            delete ptagHexindexSensorInfo;
            this->m_sensorInfosMap.erase(_sensorInfosMap_constaccessor);
        }
    }
}

void GaeactorProcessor::clearEntityInfo(const TYPE_ULID &entityid)
{
    ENTITYINFOS_CONCURRENT_HASHMAP_Accessor _entityInfosMap_accessor;
    if(m_entityInfosMap.find(_entityInfosMap_accessor, entityid))
    {
        EntityInfoManager::getInstance().release_entityinfo_element(_entityInfosMap_accessor->second.m_pEntitySensorInfoElement);
        m_entityInfosMap.erase(_entityInfosMap_accessor);
    }
}

H3IndexBufferManager& GaeactorProcessor::getBuffer()
{
    return gaeactorenvironment::H3IndexBufferManager::getInstance();
}

#define ENABLE_CLEAR_SENSOR_ENTITY
void GaeactorProcessor::update_hexindex_entity(const TYPE_ULID& uildsrc, const H3INDEX& new_h3Index,const FLOAT64 &hgt,
                                               const transdata_entityposinfo& _entityinfo,
                                               IDENTIFI_EVENT_INFO& eventinfo)
{
    EASY_FUNCTION(profiler::colors::Green)

    bool bNeedClear = (new_h3Index == 0) ? true : false;
    bool bUpdateEvent = true;
    bool bDeal = false;
    H3INDEX oldh3 = 0;
    H3INDEX newh3 = new_h3Index;

    BufferInfoElement<tagEntityPosInfo>* _pEntitySensorInfoElement = nullptr;
    {
        ENTITYINFOS_CONCURRENT_HASHMAP_Accessor _entityInfosMap_accessor;
        if(m_entityInfosMap.find(_entityInfosMap_accessor, uildsrc))
        {
            oldh3 = _entityInfosMap_accessor->second.m_h3Index;
            _entityInfosMap_accessor->second.m_bValid = !bNeedClear;
            _entityInfosMap_accessor->second.m_h3Index = newh3;
            tagEntityPosInfo _tagEntityPosInfo{_entityinfo, (PROPERTY_GET_TYPE(_entityinfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR)? true : false};
            if(_entityInfosMap_accessor->second.m_pEntitySensorInfoElement)
            {
                _entityInfosMap_accessor->second.m_pEntitySensorInfoElement->update(std::move(_tagEntityPosInfo));
            }
            _pEntitySensorInfoElement = _entityInfosMap_accessor->second.m_pEntitySensorInfoElement;
            bDeal = (oldh3 != newh3) ? true : false;
        }
        else
        {
            if(!bNeedClear)
            {
                tagHexindexEntityInfo hexindexentityinfo;
                hexindexentityinfo.m_bValid = !bNeedClear;
                hexindexentityinfo.m_h3Index = newh3;
                tagEntityPosInfo _tagEntityPosInfo{_entityinfo, (PROPERTY_GET_TYPE(_entityinfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR)? true : false};
                hexindexentityinfo.m_pEntitySensorInfoElement = EntityInfoManager::getInstance().alloc_entityinfo_element(std::move(_tagEntityPosInfo));
                _pEntitySensorInfoElement = hexindexentityinfo.m_pEntitySensorInfoElement;
                m_entityInfosMap.insert(_entityInfosMap_accessor, uildsrc);
                _entityInfosMap_accessor->second = std::move(hexindexentityinfo);
                bDeal = true;
            }
        }
    }

    tbb::concurrent_hash_map<UINT8, std::tuple<H3INDEX, H3INDEX> > _hexidx_array_need_replace;
    if(bDeal)
    {
        gaeactorenvironment::H3IndexBufferManager::getInstance().deal_hexindex_entity_remove_old_append_new(ENTITY_KEY{uildsrc, _pEntitySensorInfoElement}, oldh3, newh3, hgt, _hexidx_array_need_replace);
    }
    else
    {
#ifdef ENABLE_CLEAR_SENSOR_ENTITY
        bUpdateEvent = false;
#endif
    }

    if(bUpdateEvent)
    {
        if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_GENERATE_DETECT_EVENT_AFTER_AGENT_STEP_UPDATE_POS() /*&& !runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE()*/)
        {
            gaeactorenvironment::H3IndexBufferManager::getInstance().trigger_refresh_event_by_entity_update(uildsrc, _hexidx_array_need_replace, eventinfo);
        }
    }

#ifdef ENABLE_CLEAR_SENSOR_ENTITY
    if(bNeedClear)
    {
        clearEntityInfo(uildsrc);
    }
#endif
//    std::cout<<"+++++++++++++++++++++++++++m_entityInfosMap size "<<m_entityInfosMap.size()<<"\n";

}

std::vector<UINT8> GaeactorProcessor::get_using_resolutions()
{
    QReadLocker locker(&m_resolutions_count_map_mutex);
    return m_resolutions_count_map_keys;
}


UINT8 GaeactorProcessor::get_using_max_resolution()
{
    QReadLocker locker(&m_resolutions_count_map_mutex);
    return m_resolution_max;
}

UINT8 GaeactorProcessor::get_using_min_resolution()
{
    QReadLocker locker(&m_resolutions_count_map_mutex);
    return m_resolution_min;
}


void GaeactorProcessor::update_hexindex_sensor(const TYPE_ULID &sensorulid,
                                               const TYPE_ULID &sensingmediaid,
                                               const HEXIDX_HGT_ARRAY  &_hexidxs,
                                               transdata_sensorposinfo &&_sensorinfo,
                                               POLYGON_LIST &&_polygon,
                                               IDENTIFI_EVENT_INFO &eventinfo)
{
    EASY_FUNCTION(profiler::colors::Green)
    UINT32 silent_time = _sensorinfo.PARAM_wave_silent_time_gap;
    UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    bool bNewAppend = true;

    UINT8 usage = _sensorinfo.PARAM_wave_usage;

    bool bNeedClear = (_hexidxs.empty() && _polygon.empty()) ? true : false;
    transdata_sensorposinfo tmp_sensorinfo = _sensorinfo;
    HEXIDX_HGT_ARRAY remove_hexidxslist;
    HEXIDX_HGT_ARRAY reserve_hexidxslist;
    HEXIDX_HGT_ARRAY append_hexidxslist;

    auto appendnewitem=[&](tagHexindexSensorInfo *hexindexsensorinfo ){
        EASY_FUNCTION(profiler::colors::Green)
        tagHexindexlistSensorInfo hexindexlistensorinfo;

        if (!bNeedClear)
        {
            hexindexlistensorinfo.m_pSensorInfoElement = SensorInfoManager::getInstance().alloc_sensorinfo_element(std::move(_sensorinfo));
            //            hexindexlistensorinfo.m_polygon = std::move(_polygon);
            hexindexlistensorinfo.m_bValid = true;
            hexindexlistensorinfo.m_silent_time = silent_time;
            hexindexlistensorinfo.m_lastUpdateTimestamp = currentTimeStamp;
        }

        tbb::parallel_for(tbb::blocked_range<size_t>(0, _hexidxs.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (size_t i = r.begin(); i != r.end(); ++i)
                              {
                                  const auto &_hexidxs_item = _hexidxs[i];
                                  {
                                      UINT8 res = (UINT8)H3_GET_RESOLUTION(_hexidxs_item.PARAM_seq_hexidx_element);
                                      tbb::concurrent_hash_map<UINT8, UINT64>::accessor _resolutions_count_map_accessor;
                                      if (m_resolutions_count_map.find(_resolutions_count_map_accessor, res))
                                      {
                                          _resolutions_count_map_accessor->second = _resolutions_count_map_accessor->second+1;
                                      }
                                      else
                                      {
                                          m_resolutions_count_map.insert(_resolutions_count_map_accessor, res);
                                          _resolutions_count_map_accessor->second = 1;
                                          {
                                              QWriteLocker locker(&m_resolutions_count_map_mutex);
                                              m_resolutions_count_map_keys.push_back(res);
                                              std::sort(m_resolutions_count_map_keys.begin(), m_resolutions_count_map_keys.end());
                                              m_resolution_min = (std::min)(m_resolution_min, res);
                                              m_resolution_max = (std::max)(m_resolution_max, res);
                                          }
                                      }                                      
                                  }
                                  hexindexlistensorinfo.append_hexindexinfo(_hexidxs_item.PARAM_seq_hexidx_element, currentTimeStamp, _hexidxs_item);
                                  gaeactorenvironment::H3IndexBufferManager::getInstance().deal_hexindex_sensor(SENSOR_KEY{hexindexsensorinfo->m_sensorulid, sensingmediaid, hexindexlistensorinfo.m_pSensorInfoElement}, _hexidxs_item, false);
                              }
                          });

        hexindexsensorinfo->append_HexindexlistSensorInfo(sensingmediaid, std::move(hexindexlistensorinfo));

        append_hexidxslist.resize(_hexidxs.size());
        append_hexidxslist = _hexidxs;
    };

    bool bUpdateEvent = true;
    {
        {
            tagHexindexSensorInfo *hexindexsensorinfo = nullptr;
            auto update_hexindexsensorinfo=[&](){                
                EASY_FUNCTION(profiler::colors::Green)
                if(hexindexsensorinfo != nullptr)
                {
                    hexindexsensorinfo->m_bValid = true;

                    if(hexindexsensorinfo->is_contain_sensor(sensingmediaid))
                    {
                        hexindexsensorinfo->update_hexindex_sensor(sensingmediaid,
                                                                   _hexidxs,
                                                                   std::move(_sensorinfo),
                                                                   std::move(_polygon),
                                                                   currentTimeStamp,
                                                                   remove_hexidxslist,
                                                                   reserve_hexidxslist,
                                                                   append_hexidxslist);
                    }
                    else
                    {
#ifdef ENABLE_CLEAR_SENSOR_ENTITY
                        if(!bNeedClear)
#endif
                        {
                            appendnewitem(hexindexsensorinfo);
                        }
                    }
                    bNewAppend = false;
                }
            };
            {
                SENSORINFOS_CONCURRENT_HASHMAP_Accessor _sensorInfosMap_accessor;
                if(m_sensorInfosMap.find(_sensorInfosMap_accessor, sensorulid))
                {
                    hexindexsensorinfo =  _sensorInfosMap_accessor->second;
                }
            }
            update_hexindexsensorinfo();
        }
#ifdef ENABLE_CLEAR_SENSOR_ENTITY
        if(!bNeedClear && bNewAppend)
#else
        if(bNewAppend)
#endif
        {
            tagHexindexSensorInfo *hexindexsensorinfo = new tagHexindexSensorInfo;
            hexindexsensorinfo->m_bValid = true;
            hexindexsensorinfo->m_sensorulid = sensorulid;

            appendnewitem(hexindexsensorinfo);

            {
                SENSORINFOS_CONCURRENT_HASHMAP_Accessor _sensorInfosMap_accessor;
                m_sensorInfosMap.insert(_sensorInfosMap_accessor, sensorulid);
                _sensorInfosMap_accessor->second = hexindexsensorinfo;
            }
        }
    }

    if(remove_hexidxslist.empty() && append_hexidxslist.empty())
    {
#ifdef ENABLE_CLEAR_SENSOR_ENTITY
        bUpdateEvent = false;
#endif
    }
    if(bUpdateEvent)
    {
        if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_GENERATE_DETECT_EVENT_AFTER_AGENT_STEP_UPDATE_SENSOR()  /*&& !runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE()*/)
        {
            if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
            {
                if(_sensorinfo.PARAM_wave_usage == 0x08)
                {
                    gaeactorenvironment::H3IndexBufferManager::getInstance().trigger_refresh_event_by_sensor(sensorulid,
                                                                                                             sensingmediaid,
                                                                                                             tmp_sensorinfo,
                                                                                                             bNeedClear,
                                                                                                             remove_hexidxslist,
                                                                                                             reserve_hexidxslist,
                                                                                                             append_hexidxslist,
                                                                                                             eventinfo);
                }
            }

            else
            {
                gaeactorenvironment::H3IndexBufferManager::getInstance().trigger_refresh_event_by_sensor(sensorulid,
                                                                                                         sensingmediaid,
                                                                                                         tmp_sensorinfo,
                                                                                                         bNeedClear,
                                                                                                         remove_hexidxslist,
                                                                                                         reserve_hexidxslist,
                                                                                                         append_hexidxslist,
                                                                                                         eventinfo);
            }
        }
    }
#ifdef ENABLE_CLEAR_SENSOR_ENTITY
    if(bNeedClear)
    {
        clearSensorInfo(sensorulid, sensingmediaid);
    }
#endif
//    stdutils::OriDateTime::sleep(20);
    //此处应该是需要休眠，但只有输出才有效？
//    std::cout<<"+++++++++++++++++++++++++++m_sensorInfosMap size "<<m_sensorInfosMap.size()<<"\n";
}

void GaeactorProcessor::force_cover_deal_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslist, const UINT32 &_silent_time)
{
    //    EASY_FUNCTION(profiler::colors::Green)

    SENSORINFOS_CONCURRENT_HASHMAP_Accessor _sensorInfosMap_accessor;
    if(m_sensorInfosMap.find(_sensorInfosMap_accessor, sensorulid))
    {
        tagHexindexSensorInfo* ptagHexindexSensorInfo =  _sensorInfosMap_accessor->second;
        if(ptagHexindexSensorInfo)
        {
            ptagHexindexSensorInfo->force_cover_deal_hexindex_sensor(sensingmediaid, hexidxslist);
        }
    }
}


void GaeactorProcessor::force_cover_update_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &_hexidxs, transdata_sensorposinfo &&_sensorinfo, POLYGON_LIST &&_polygon)
{
    EASY_FUNCTION(profiler::colors::Green)
    SENSORINFOS_CONCURRENT_HASHMAP_Accessor _sensorInfosMap_accessor;
    if(m_sensorInfosMap.find(_sensorInfosMap_accessor, sensorulid))
    {
        tagHexindexSensorInfo* ptagHexindexSensorInfo =  _sensorInfosMap_accessor->second;
        if(ptagHexindexSensorInfo)
        {
            ptagHexindexSensorInfo->m_bValid = true;
            ptagHexindexSensorInfo->force_cover_update_hexindex_sensor(sensingmediaid, _hexidxs);
        }
    }
}

void GaeactorProcessor::refresh_silent_timeout()
{
    UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    std::vector<TYPE_ULID> keys;
    get_keys(this->m_sensorInfosMap, keys);

    PARALLEL_BEGIN
        SENSORINFOS_CONCURRENT_HASHMAP_Accessor _sensorInfosMap_accessor;
    if (m_sensorInfosMap.find(_sensorInfosMap_accessor, key))
    {
        tagHexindexSensorInfo* ptagHexindexSensorInfo =  _sensorInfosMap_accessor->second;
        if(ptagHexindexSensorInfo)
        {
            ptagHexindexSensorInfo->refresh_silent_timeout( currentTimeStamp);
        }
    }
    PARALLEL_END
}

transdata_entityposinfo GaeactorProcessor::gettransdata_entityposinfo(const TYPE_ULID &sensorulid)
{
    ENTITYINFOS_CONCURRENT_HASHMAP_ConstAccessor _entityInfosMap_constaccessor;
    if(m_entityInfosMap.find(_entityInfosMap_constaccessor, sensorulid))
    {
        if(_entityInfosMap_constaccessor->second.m_pEntitySensorInfoElement)
        {
            return _entityInfosMap_constaccessor->second.m_pEntitySensorInfoElement->m_bufferinfo.m_entityInfo;
        }
    }
    return transdata_entityposinfo();
}

bool GaeactorProcessor::isEntityHaveSensorProperty(const TYPE_ULID &entityulid)
{
    //EASY_FUNCTION(profiler::colors::DeepOrange)
    ENTITYINFOS_CONCURRENT_HASHMAP_ConstAccessor _entityInfosMap_constaccessor;
    if(m_entityInfosMap.find(_entityInfosMap_constaccessor, entityulid))
    {
        if(_entityInfosMap_constaccessor->second.m_pEntitySensorInfoElement)
        {
            return _entityInfosMap_constaccessor->second.m_pEntitySensorInfoElement->m_bufferinfo.m_isSensor;
        }
    }
    return false;
}


transdata_sensorposinfo GaeactorProcessor::gettransdata_sensorposinfo_by_sensingmedia(const SENSOR_KEY &sensor_key)
{
    EASY_FUNCTION(profiler::colors::Green)

    SENSORINFOS_CONCURRENT_HASHMAP_ConstAccessor _sensorInfosMap_constaccessor;
    if (this->m_sensorInfosMap.find(_sensorInfosMap_constaccessor, sensor_key.sensorid))
    {
        tagHexindexSensorInfo* ptagHexindexSensorInfo = _sensorInfosMap_constaccessor->second;
        if(ptagHexindexSensorInfo &&
            ptagHexindexSensorInfo->m_sensorulid == sensor_key.sensorid &&
            ptagHexindexSensorInfo->is_contain_sensor(sensor_key.sensingmediaid))
        {
            return ptagHexindexSensorInfo->gettransdata_sensorposinfo_by_sensingmedia(sensor_key.sensingmediaid);
        }
    }
    return transdata_sensorposinfo();
}

void GaeactorProcessor::refresh_event(IDENTIFI_EVENT_INFO &identifi_event_info)
{
    EASY_FUNCTION(profiler::colors::RichRed)
    refresh_event_type(identifi_event_info,0,0,0,0);
}

void GaeactorProcessor::refresh_events_by_cores(const TYPE_ULID &entityid, const H3INDEX &h3Indexsrc, const FLOAT64 &hgt, const transdata_entityposinfo& eninfo,IDENTIFI_EVENT_INFO& identifi_event_info)
{
    EASY_FUNCTION(profiler::colors::DarkMagenta)
#if 1
    refresh_event_type(identifi_event_info,entityid,0,0,1);
#else
#endif
}

void GaeactorProcessor::refresh_events_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmedia_id, const HEXIDX_HGT_ARRAY &hexidxslistret,IDENTIFI_EVENT_INFO& identifi_event_info)
{
    EASY_FUNCTION(profiler::colors::Green)
#if 1
    refresh_event_type(identifi_event_info,0,sensorid,sensingmedia_id,2);
#else
#endif
}

void GaeactorProcessor::trigger_refresh_event_by_entity_update(const TYPE_ULID &entityid, const tbb::concurrent_hash_map<UINT8, std::tuple<H3INDEX, H3INDEX> > &_hexidx_array_need_replace, IDENTIFI_EVENT_INFO &eventinfo)
{
    EASY_FUNCTION(profiler::colors::DeepOrange900)
    EVENTS_CONCURRENT_HASHMAP concurrent_clearEventlist;
    EVENTS_CONCURRENT_HASHMAP concurrent_addEventlist;
    EVENTS_CONCURRENT_HASHMAP concurrent_updateEventlist;


    auto clearEventRecord=[&](const EVENT_KEY_TYPE& key)
    {
        {

            ENTITY_EVENTS_CONCURRENT_HASHMAP_Accessor _entity_events_map_accessor;
            if(m_entity_events_map.find(_entity_events_map_accessor, key.entityid))
            {
                EVENTS_VALID_CONCURRENT_HASHMAP_Accessor _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor;
                if( _entity_events_map_accessor->second.find(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key))
                {
                    _entity_events_map_accessor->second.erase(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor);
                }
            }
        }

        {
            SENSOR_KEY _SENSOR_KEY{key.sensorid, key.sensingmediaid, 0};
            SENSOR_EVENTS_CONCURRENT_HASHMAP_Accessor _sensor_events_map_accessor;
            if(m_sensor_events_map.find(_sensor_events_map_accessor, _SENSOR_KEY))
            {
                EVENTS_VALID_CONCURRENT_HASHMAP_Accessor _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor;
                if( _sensor_events_map_accessor->second.find(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key))
                {
                    _sensor_events_map_accessor->second.erase(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor);
                }
            }
        }
    };



    auto appendEventRecord=[&](const EVENT_KEY_TYPE& key)
    {
        {

            ENTITY_EVENTS_CONCURRENT_HASHMAP_Accessor _entity_events_map_accessor;
            if(m_entity_events_map.find(_entity_events_map_accessor, key.entityid))
            {
                EVENTS_VALID_CONCURRENT_HASHMAP_Accessor _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor;
                if(!_entity_events_map_accessor->second.find(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key))
                {
                    _entity_events_map_accessor->second.insert(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key);
                    _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor->second = true;
                }
            }
            else
            {
                EVENTS_VALID_CONCURRENT_HASHMAP _EVENTS_VALID_CONCURRENT_HASHMAP;
                _EVENTS_VALID_CONCURRENT_HASHMAP.insert(std::make_pair(key, true));
                m_entity_events_map.insert(_entity_events_map_accessor, key.entityid);
                _entity_events_map_accessor->second = std::move(_EVENTS_VALID_CONCURRENT_HASHMAP);

            }
        }

        {
            SENSOR_KEY _SENSOR_KEY{key.sensorid, key.sensingmediaid, 0};
            SENSOR_EVENTS_CONCURRENT_HASHMAP_Accessor _sensor_events_map_accessor;
            if(m_sensor_events_map.find(_sensor_events_map_accessor, _SENSOR_KEY))
            {
                EVENTS_VALID_CONCURRENT_HASHMAP_Accessor _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor;
                if(!_sensor_events_map_accessor->second.find(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key))
                {
                    _sensor_events_map_accessor->second.insert(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key);
                    _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor->second = true;
                }
            }
            else
            {
                EVENTS_VALID_CONCURRENT_HASHMAP _EVENTS_VALID_CONCURRENT_HASHMAP;
                _EVENTS_VALID_CONCURRENT_HASHMAP.insert(std::make_pair(key, true));
                m_sensor_events_map.insert(_sensor_events_map_accessor, _SENSOR_KEY);
                _sensor_events_map_accessor->second = std::move(_EVENTS_VALID_CONCURRENT_HASHMAP);
            }
        }
    };
    {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //获取与该实体相关的事件
        EVENTS_VALID_CONCURRENT_HASHMAP _EVENTS_VALID_CONCURRENT_HASHMAP;
        {
            ENTITY_EVENTS_CONCURRENT_HASHMAP_ConstAccessor _entity_events_map_constaccessor;
            if(m_entity_events_map.find(_entity_events_map_constaccessor, entityid))
            {
                _EVENTS_VALID_CONCURRENT_HASHMAP = _entity_events_map_constaccessor->second;
            }
        }

        std::vector<EVENT_KEY_TYPE> _EVENTS_VALID_CONCURRENT_HASHMAP_keys;
        get_keys(_EVENTS_VALID_CONCURRENT_HASHMAP, _EVENTS_VALID_CONCURRENT_HASHMAP_keys);

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //将该实体相关的事件的重置
        tbb::parallel_for(tbb::blocked_range<size_t>(0, _EVENTS_VALID_CONCURRENT_HASHMAP_keys.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (size_t i = r.begin(); i != r.end(); ++i)
                              {
                                  const auto& key = _EVENTS_VALID_CONCURRENT_HASHMAP_keys[i];
                                  EXIST_EVENTS_CONCURRENT_HASHMAP_Accessor _exist_events_map_accessor;
                                  if (this->m_exist_events_map.find(_exist_events_map_accessor, key))
                                  {
                                      //当前实体的存在的事件 重置 ，其他实体的不管
                                      if(_exist_events_map_accessor->first.entityid == entityid)
                                      {
                                          _exist_events_map_accessor->second.second = false;
                                      }
                                  }
                              }
                          });
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //获取与该实体相关的相交关系


        INTERSECTINFO_CONCURRENT_HASHMAP intersectInfos;

        auto getintersectInfos=[&](const H3INDEX h3index)
        {
            if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
            {
                gaeactorenvironment::H3IndexBufferManager::getInstance().lockIntersectForRead();
            }
            INTERSECTINFO_CONCURRENT_HASHMAP_ConstAccessor _exist_events_map_constaccessor;
            if (gaeactorenvironment::H3IndexBufferManager::getInstance().m_IntersectInfoMap.find(_exist_events_map_constaccessor, h3index))
            {
                INTERSECTINFO_CONCURRENT_HASHMAP_Accessor intersectInfos_accessor;
                intersectInfos.insert(intersectInfos_accessor, _exist_events_map_constaccessor->first);
                intersectInfos_accessor->second = _exist_events_map_constaccessor->second;
            }
            if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
            {
                gaeactorenvironment::H3IndexBufferManager::getInstance().unlockIntersect();
            }
        };


        std::vector<UINT8> _hexidx_array_need_replace_keys;
        get_keys(_hexidx_array_need_replace, _hexidx_array_need_replace_keys);

        tbb::parallel_for(tbb::blocked_range<size_t>(0, _hexidx_array_need_replace_keys.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (UINT8 i = r.begin(); i != r.end(); ++i)
                              {
                                  const UINT8& res = _hexidx_array_need_replace_keys[i];

                                  tbb::concurrent_hash_map<UINT8,std::tuple<H3INDEX, H3INDEX>>::const_accessor replace_accessor;

                                  if(_hexidx_array_need_replace.find(replace_accessor,res))
                                  {
                                      const std::tuple<H3INDEX, H3INDEX>& replace_info = replace_accessor->second;
                                      const H3INDEX& old_hexidx = std::get<0>(replace_info);
                                      const H3INDEX& new_hexidx = std::get<1>(replace_info);

                                      if(old_hexidx != 0 || new_hexidx != 0)
                                      {
                                          if(new_hexidx != old_hexidx)
                                          {
                                              if(old_hexidx != 0 && new_hexidx != 0 && new_hexidx != old_hexidx)
                                              {
                                                  getintersectInfos(old_hexidx);
                                                  getintersectInfos(new_hexidx);
                                              }
                                              else if(old_hexidx == 0 && new_hexidx != 0)
                                              {
                                                  getintersectInfos(new_hexidx);
                                              }
                                              else if(old_hexidx != 0 && new_hexidx == 0)
                                              {
                                                  getintersectInfos(old_hexidx);
                                              }

                                          }
                                          else
                                          {
                                              getintersectInfos(new_hexidx);
                                          }
                                      }
                                  }
                              }
                          });
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //处理与该实体相关的相交关系


        std::vector<H3INDEX> intersectInfos_keys;
        get_keys(intersectInfos, intersectInfos_keys);

#if 0
        UINT64 curTimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
#else
        UINT64 curTimestamp = SettingsConfig::getInstance().m_simparams.m_sim_timestamp * 1000;
#endif



#if 0
        for(auto _intersectInfos_itor = intersectInfos.begin();_intersectInfos_itor != intersectInfos.end(); _intersectInfos_itor++)
        {
            const SENESOR_LIST_CONCURRENT_HASHMAP &sensorlist = _intersectInfos_itor->second.sensorlist;
            const ENTITY_LIST_CONCURRENT_HASHMAP &entitylist = _intersectInfos_itor->second.entitylist;

            for(auto sensor_list_itor = sensorlist.begin(); sensor_list_itor != sensorlist.end(); sensor_list_itor++)
            {
                const auto& sensorlist_key = sensor_list_itor->first;
                const transdata_param_seq_hexidx_hgt& sensor_hgt_range = _sensorlist_constaccessor->second;
                const TYPE_ULID &sensorulid = sensorlist_key.sensorid;
                const TYPE_ULID &sensingmediaid = sensorlist_key.sensingmediaid;
                const gaeactorenvironment::BufferInfoElement<transdata_sensorposinfo>*_sensorinfo_ptr = sensorlist_key._sensorinfo_ptr;


                for(auto entitylist_itor = entitylist.begin(); entitylist_itor != entitylist.end(); entitylist_itor++)
                {
                    const ENTITY_KEY& entitylist_key = entitylist_itor->first;
                    const FLOAT64 &entity_hgt = _entitylist_constaccessor->second;
                    const TYPE_ULID & entityulid = entitylist_key.entityid;
                    const gaeactorenvironment::BufferInfoElement<tagEntityPosInfo>* _entityinfo_ptr = entitylist_key._entityinfo_ptr;
                    if (sensorulid == entityulid)
                    {
                        continue;
                    }

                    //其他实体不管
                    if (entityid != entityulid)
                    {
                        continue;
                    }

                    EVENT_KEY_TYPE eventtile = EVENT_KEY_TYPE{ sensorulid, entityulid, sensingmediaid };

                    {
                        EXIST_EVENTS_CONCURRENT_HASHMAP_Accessor _exist_events_map_accessor;
                        if (m_exist_events_map.find(_exist_events_map_accessor, eventtile))
                        {
                            EVENT_INFO & event_info = _exist_events_map_accessor->second.first;

                            _exist_events_map_accessor->second.second = true;

                            //update exist event
                            transdata_entityposinfo new_sensorposinfo = gettransdata_entityposinfo(sensorulid);
                            transdata_entityposinfo new_entityposinfo;
                            if(_entityinfo_ptr)
                            {
                                new_entityposinfo = _entityinfo_ptr->m_bufferinfo.m_entityInfo;
                            }
                            //此处控制update事件的刷新频率，
                            //当场实体发生变化 或 运动实体发生变化 【感知位置发生变化】或 【感知位置未发生变化，但更新周期到了】上次更新时间间隔超过250ms，触发发送
                            //反之场实体未发生变化 且 运动实体未发生变化 且感知时间更新周期太短 ，则不触发该更新事件的发送
                            if(event_info.m_sensorposinfo.PARAM_pos_hexidx != new_sensorposinfo.PARAM_pos_hexidx ||
                                event_info.m_entityposinfo.PARAM_pos_hexidx != new_entityposinfo.PARAM_pos_hexidx /*||
                                                                                                      curTimestamp - event_info.m_timestamp > 250*/)
                            {
                                event_info.m_sensorposinfo = std::move(new_sensorposinfo);
                                event_info.m_entityposinfo = std::move(new_entityposinfo);

                                if(_sensorinfo_ptr)
                                {
                                    event_info.m_sensorproprety = _sensorinfo_ptr->m_bufferinfo;
                                }
                                event_info.m_distance = LocationHelper::greatDistanceM(event_info.m_sensorposinfo.PARAM_pos_hexidx, event_info.m_entityposinfo.PARAM_pos_hexidx);
                                if(_entityinfo_ptr)
                                {
                                    event_info.m_entityisSensorProprety  = _entityinfo_ptr->m_bufferinfo.m_isSensor;
                                }
//                                if (event_info.m_entityisSensorProprety)
//                                {
//                                    event_info.m_entityproprety = gettransdata_sensorposinfo(entityulid);
//                                }
                                event_info.m_sensor_hgt_range = sensor_hgt_range;
                                event_info.m_entity_hgt = entity_hgt;
                                event_info.m_timestamp = curTimestamp;
                                EVENTS_CONCURRENT_HASHMAP_Accessor concurrent_updateEventlist_accessor;
                                concurrent_updateEventlist.insert(concurrent_updateEventlist_accessor, eventtile);
                                concurrent_updateEventlist_accessor->second = _exist_events_map_accessor->second.first;
                            }
                        }
                        else
                        {
                            //add new event
                            transdata_entityposinfo _sensorposinfo = gettransdata_entityposinfo(sensorulid);
                            transdata_entityposinfo _entityposinfo;
                            bool _entityisSensorProprety;
                            if(_entityinfo_ptr)
                            {
                                _entityposinfo = _entityinfo_ptr->m_bufferinfo.m_entityInfo;
                                _entityisSensorProprety = _entityinfo_ptr->m_bufferinfo.m_isSensor;
                            }

                            auto distance = LocationHelper::greatDistanceM(_sensorposinfo.PARAM_pos_hexidx, _entityposinfo.PARAM_pos_hexidx);

                            transdata_sensorposinfo _entityproprety;
//                            if (_entityisSensorProprety)
//                            {
//                                _entityproprety = gettransdata_sensorposinfo(entityulid);
//                            }
                            transdata_sensorposinfo _sensorproprety;
                            if(_sensorinfo_ptr)
                            {
                                _sensorproprety = _sensorinfo_ptr->m_bufferinfo;
                            }
                            EVENT_INFO eventinfo{ sensorulid,
                                                 entityulid,
                                                 sensingmediaid,
                                                 std::move(_sensorposinfo),
                                                 std::move(_entityposinfo),
                                                 std::move(_sensorproprety),
                                                 _entityisSensorProprety,
                                                 std::move(_entityproprety),
                                                 distance ,
                                                 curTimestamp,
                                                 sensor_hgt_range,
                                                 entity_hgt};

                            m_exist_events_map.insert(_exist_events_map_accessor, eventtile);
                            _exist_events_map_accessor->second = qMakePair(std::move(eventinfo), true);
                            appendEventRecord(eventtile);

                            EVENTS_CONCURRENT_HASHMAP_Accessor concurrent_addEventlist_accessor;
                            concurrent_addEventlist.insert(concurrent_addEventlist_accessor, eventtile);
                            concurrent_addEventlist_accessor->second = _exist_events_map_accessor->second.first;

                        }
                    }
                }
            }
        }
#else
        tbb::parallel_for(tbb::blocked_range<size_t>(0, intersectInfos_keys.size()),
                          [&](const tbb::blocked_range<size_t>& intersectInfos_keys_r) {
                              for (size_t intersectInfos_keys_i = intersectInfos_keys_r.begin(); intersectInfos_keys_i != intersectInfos_keys_r.end(); ++intersectInfos_keys_i)
                              {
                                  const auto& intersectInfos_key = intersectInfos_keys[intersectInfos_keys_i];
                                  INTERSECTINFO_CONCURRENT_HASHMAP_ConstAccessor _intersectInfos_constaccessor;
                                  if (intersectInfos.find(_intersectInfos_constaccessor, intersectInfos_key))
                                  {
                                      std::vector<SENSOR_KEY> sensorlist_keys;
                                      get_keys(_intersectInfos_constaccessor->second.sensorlist, sensorlist_keys);

                                      std::vector<ENTITY_KEY> entitylist_keys;
                                      get_keys(_intersectInfos_constaccessor->second.entitylist, entitylist_keys);

                                      tbb::parallel_for(tbb::blocked_range<size_t>(0, sensorlist_keys.size()),
                                                        [&](const tbb::blocked_range<size_t>& sensorlist_keys_r) {
                                                            for (size_t sensorlist_keys_i = sensorlist_keys_r.begin(); sensorlist_keys_i != sensorlist_keys_r.end(); ++sensorlist_keys_i)
                                                            {
                                                                const auto& sensorlist_key = sensorlist_keys[sensorlist_keys_i];
                                                                SENESOR_LIST_CONCURRENT_HASHMAP_ConstAccessor _sensorlist_constaccessor;
                                                                if (_intersectInfos_constaccessor->second.sensorlist.find(_sensorlist_constaccessor, sensorlist_key))
                                                                {
                                                                    const transdata_param_seq_hexidx_hgt& sensor_hgt_range = _sensorlist_constaccessor->second;
                                                                    const TYPE_ULID &sensorulid = _sensorlist_constaccessor->first.sensorid;
                                                                    const TYPE_ULID &sensingmediaid = _sensorlist_constaccessor->first.sensingmediaid;
                                                                    const gaeactorenvironment::BufferInfoElement<transdata_sensorposinfo>*_sensorinfo_ptr = _sensorlist_constaccessor->first._sensorinfo_ptr;

                                                                    tbb::parallel_for(tbb::blocked_range<size_t>(0, entitylist_keys.size()),
                                                                                      [&](const tbb::blocked_range<size_t>& entitylist_keys_r) {
                                                                                          for (size_t entitylist_keys_i = entitylist_keys_r.begin(); entitylist_keys_i != entitylist_keys_r.end(); ++entitylist_keys_i)
                                                                                          {
                                                                                              const ENTITY_KEY& entitylist_key = entitylist_keys[entitylist_keys_i];
                                                                                              ENTITY_LIST_CONCURRENT_HASHMAP_ConstAccessor _entitylist_constaccessor;
                                                                                              if (_intersectInfos_constaccessor->second.entitylist.find(_entitylist_constaccessor, entitylist_key))
                                                                                              {
                                                                                                  const FLOAT64 &entity_hgt = _entitylist_constaccessor->second;
                                                                                                  const TYPE_ULID & entityulid = _entitylist_constaccessor->first.entityid;
                                                                                                  const gaeactorenvironment::BufferInfoElement<tagEntityPosInfo>*_entityinfo_ptr = _entitylist_constaccessor->first._entityinfo_ptr;
                                                                                                  if (sensorulid == entityulid)
                                                                                                  {
                                                                                                      continue;
                                                                                                  }

                                                                                                  //其他实体不管
                                                                                                  if (entityid != entityulid)
                                                                                                  {
                                                                                                      continue;
                                                                                                  }

                                                                                                  EVENT_KEY_TYPE eventtile = EVENT_KEY_TYPE{ sensorulid, entityulid, sensingmediaid };

                                                                                                  {
                                                                                                      EXIST_EVENTS_CONCURRENT_HASHMAP_Accessor _exist_events_map_accessor;
                                                                                                      if (m_exist_events_map.find(_exist_events_map_accessor, eventtile))
                                                                                                      {
                                                                                                          EVENT_INFO & event_info = _exist_events_map_accessor->second.first;

                                                                                                          _exist_events_map_accessor->second.second = true;

                                                                                                          //update exist event
                                                                                                          transdata_entityposinfo new_sensorposinfo = gettransdata_entityposinfo(sensorulid);
                                                                                                          transdata_entityposinfo new_entityposinfo;
                                                                                                          if(_entityinfo_ptr)
                                                                                                          {
                                                                                                              new_entityposinfo = _entityinfo_ptr->m_bufferinfo.m_entityInfo;
                                                                                                          }
                                                                                                          //此处控制update事件的刷新频率，
                                                                                                          //当场实体发生变化 或 运动实体发生变化 【感知位置发生变化】或 【感知位置未发生变化，但更新周期到了】上次更新时间间隔超过250ms，触发发送
                                                                                                          //反之场实体未发生变化 且 运动实体未发生变化 且感知时间更新周期太短 ，则不触发该更新事件的发送
                                                                                                          if(event_info.m_sensorposinfo.PARAM_pos_hexidx != new_sensorposinfo.PARAM_pos_hexidx ||
                                                                                                              event_info.m_entityposinfo.PARAM_pos_hexidx != new_entityposinfo.PARAM_pos_hexidx /*||
                                                                                                      curTimestamp - event_info.m_timestamp > 250*/)
                                                                                                          {
                                                                                                              event_info.m_sensorposinfo = std::move(new_sensorposinfo);
                                                                                                              event_info.m_entityposinfo = std::move(new_entityposinfo);

                                                                                                              if(_sensorinfo_ptr)
                                                                                                              {
                                                                                                                  event_info.m_sensorproprety = _sensorinfo_ptr->m_bufferinfo;
                                                                                                              }
                                                                                                              event_info.m_distance = LocationHelper::greatDistanceM(event_info.m_sensorposinfo.PARAM_pos_hexidx, event_info.m_entityposinfo.PARAM_pos_hexidx);
                                                                                                              if(_entityinfo_ptr)
                                                                                                              {
                                                                                                                  event_info.m_entityisSensorProprety  = _entityinfo_ptr->m_bufferinfo.m_isSensor;
                                                                                                              }
//                                                                                                              if (event_info.m_entityisSensorProprety)
//                                                                                                              {
//                                                                                                                  event_info.m_entityproprety = gettransdata_sensorposinfo(entityulid);
//                                                                                                              }
                                                                                                              event_info.m_sensor_hgt_range = sensor_hgt_range;
                                                                                                              event_info.m_entity_hgt = entity_hgt;
                                                                                                              event_info.m_timestamp = curTimestamp;
                                                                                                              EVENTS_CONCURRENT_HASHMAP_Accessor concurrent_updateEventlist_accessor;
                                                                                                              concurrent_updateEventlist.insert(concurrent_updateEventlist_accessor, eventtile);
                                                                                                              concurrent_updateEventlist_accessor->second = _exist_events_map_accessor->second.first;
                                                                                                          }
                                                                                                      }
                                                                                                      else
                                                                                                      {
                                                                                                          //add new event
                                                                                                          transdata_entityposinfo _sensorposinfo = gettransdata_entityposinfo(sensorulid);
                                                                                                          transdata_entityposinfo _entityposinfo;
                                                                                                          bool _entityisSensorProprety = false;
                                                                                                          if(_entityinfo_ptr)
                                                                                                          {
                                                                                                              _entityposinfo = _entityinfo_ptr->m_bufferinfo.m_entityInfo;
                                                                                                              _entityisSensorProprety = _entityinfo_ptr->m_bufferinfo.m_isSensor;
                                                                                                          }
                                                                                                          auto distance = LocationHelper::greatDistanceM(_sensorposinfo.PARAM_pos_hexidx, _entityposinfo.PARAM_pos_hexidx);

                                                                                                          transdata_sensorposinfo _entityproprety;
//                                                                                                          if (_entityisSensorProprety)
//                                                                                                          {
//                                                                                                              _entityproprety = gettransdata_sensorposinfo(entityulid);
//                                                                                                          }

                                                                                                          transdata_sensorposinfo _sensorproprety;
                                                                                                          if(_sensorinfo_ptr)
                                                                                                          {
                                                                                                            _sensorproprety = _sensorinfo_ptr->m_bufferinfo;
                                                                                                          }

                                                                                                          EVENT_INFO eventinfo{ sensorulid,
                                                                                                                               entityulid,
                                                                                                                               sensingmediaid,
                                                                                                                               std::move(_sensorposinfo),
                                                                                                                               std::move(_entityposinfo),
                                                                                                                               std::move(_sensorproprety),
                                                                                                                               _entityisSensorProprety,
                                                                                                                               std::move(_entityproprety),
                                                                                                                               distance ,
                                                                                                                               curTimestamp,
                                                                                                                               sensor_hgt_range,
                                                                                                                               entity_hgt};

                                                                                                          m_exist_events_map.insert(_exist_events_map_accessor, eventtile);
                                                                                                          _exist_events_map_accessor->second = qMakePair(std::move(eventinfo), true);
                                                                                                          appendEventRecord(eventtile);

                                                                                                          EVENTS_CONCURRENT_HASHMAP_Accessor concurrent_addEventlist_accessor;
                                                                                                          concurrent_addEventlist.insert(concurrent_addEventlist_accessor, eventtile);
                                                                                                          concurrent_addEventlist_accessor->second = _exist_events_map_accessor->second.first;

                                                                                                      }
                                                                                                  }

                                                                                              }
                                                                                          }
                                                                                      });
                                                                }
                                                            }
                                                        });
                                  }
                              }
                          });
#endif

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //整理 与该实体相关的事件  清理无效的

        tbb::parallel_for(tbb::blocked_range<size_t>(0, _EVENTS_VALID_CONCURRENT_HASHMAP_keys.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (size_t i = r.begin(); i != r.end(); ++i)
                              {
                                  const EVENT_KEY_TYPE& key = _EVENTS_VALID_CONCURRENT_HASHMAP_keys[i];
                                  EXIST_EVENTS_CONCURRENT_HASHMAP_Accessor _exist_events_map_accessor;
                                  if (this->m_exist_events_map.find(_exist_events_map_accessor, key))
                                  {
                                      //其他实体不管
                                      if(_exist_events_map_accessor->first.entityid != entityid)
                                      {
                                          continue;
                                      }
                                      if (!_exist_events_map_accessor->second.second)
                                      {
                                          EVENT_INFO event_info = _exist_events_map_accessor->second.first;
                                          event_info.m_timestamp = curTimestamp;
                                          EVENTS_CONCURRENT_HASHMAP_Accessor _clearEventlist_accessor;
                                          concurrent_clearEventlist.insert(_clearEventlist_accessor, _exist_events_map_accessor->first);
                                          _clearEventlist_accessor->second = std::move(event_info);
                                          m_exist_events_map.erase(_exist_events_map_accessor);

                                          clearEventRecord(key);
                                          continue;
                                      }
                                  }

                              }
                          });


    }

    EVENTS_HASHMAP &addEventlist = std::get<0>(eventinfo);

    EVENTS_CONCURRENT_HASHMAP_ConstIterator _addlist_itor = concurrent_addEventlist.begin();
    while (_addlist_itor != concurrent_addEventlist.end())
    {
        addEventlist.insert(std::make_pair(std::move(_addlist_itor->first), std::move(_addlist_itor->second)));
        _addlist_itor++;
    }

    EVENTS_HASHMAP &updateEventlist = std::get<2>(eventinfo);
    EVENTS_CONCURRENT_HASHMAP_ConstIterator _updatelist_itor = concurrent_updateEventlist.begin();
    while (_updatelist_itor != concurrent_updateEventlist.end())
    {
        updateEventlist.insert(std::make_pair(std::move(_updatelist_itor->first), std::move(_updatelist_itor->second)));
        _updatelist_itor++;
    }


    EVENTS_HASHMAP &clearEventlist = std::get<1>(eventinfo);
    EVENTS_CONCURRENT_HASHMAP_ConstIterator _clearlist_itor = concurrent_clearEventlist.begin();
    while (_clearlist_itor != concurrent_clearEventlist.end())
    {
        clearEventlist.insert(std::make_pair(std::move(_clearlist_itor->first), std::move(_clearlist_itor->second)));
        _clearlist_itor++;
    }
}

void GaeactorProcessor::trigger_refresh_event_by_sensor(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmedia_id, const transdata_sensorposinfo &_sensorinfo, const bool &bNeedClear, const HEXIDX_HGT_ARRAY &remove_hexidxslist, const HEXIDX_HGT_ARRAY &reserve_hexidxslist, const HEXIDX_HGT_ARRAY &append_hexidxslist, IDENTIFI_EVENT_INFO &eventinfo)
{
    EASY_FUNCTION(profiler::colors::AmberA400)
    EVENTS_CONCURRENT_HASHMAP concurrent_clearEventlist;
    EVENTS_CONCURRENT_HASHMAP concurrent_addEventlist;
    EVENTS_CONCURRENT_HASHMAP concurrent_updateEventlist;


    auto clearEventRecord=[&](const EVENT_KEY_TYPE& key)
    {
        {

            ENTITY_EVENTS_CONCURRENT_HASHMAP_Accessor _entity_events_map_accessor;
            if(m_entity_events_map.find(_entity_events_map_accessor, key.entityid))
            {
                EVENTS_VALID_CONCURRENT_HASHMAP_Accessor _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor;
                if( _entity_events_map_accessor->second.find(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key))
                {
                    _entity_events_map_accessor->second.erase(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor);
                }
            }
        }

        {
            SENSOR_KEY _SENSOR_KEY{key.sensorid, key.sensingmediaid, 0};
            SENSOR_EVENTS_CONCURRENT_HASHMAP_Accessor _sensor_events_map_accessor;
            if(m_sensor_events_map.find(_sensor_events_map_accessor, _SENSOR_KEY))
            {
                EVENTS_VALID_CONCURRENT_HASHMAP_Accessor _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor;
                if( _sensor_events_map_accessor->second.find(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key))
                {
                    _sensor_events_map_accessor->second.erase(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor);
                }
            }
        }
    };



    auto appendEventRecord=[&](const EVENT_KEY_TYPE& key)
    {
        {

            ENTITY_EVENTS_CONCURRENT_HASHMAP_Accessor _entity_events_map_accessor;
            if(m_entity_events_map.find(_entity_events_map_accessor, key.entityid))
            {
                EVENTS_VALID_CONCURRENT_HASHMAP_Accessor _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor;
                if(!_entity_events_map_accessor->second.find(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key))
                {
                    _entity_events_map_accessor->second.insert(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key);
                    _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor->second = true;
                }
            }
            else
            {
                EVENTS_VALID_CONCURRENT_HASHMAP _EVENTS_VALID_CONCURRENT_HASHMAP;
                _EVENTS_VALID_CONCURRENT_HASHMAP.insert(std::make_pair(key, true));
                m_entity_events_map.insert(_entity_events_map_accessor, key.entityid);
                _entity_events_map_accessor->second = std::move(_EVENTS_VALID_CONCURRENT_HASHMAP);

            }
        }

        {
            SENSOR_KEY _SENSOR_KEY{key.sensorid, key.sensingmediaid, 0};
            SENSOR_EVENTS_CONCURRENT_HASHMAP_Accessor _sensor_events_map_accessor;
            if(m_sensor_events_map.find(_sensor_events_map_accessor, _SENSOR_KEY))
            {
                EVENTS_VALID_CONCURRENT_HASHMAP_Accessor _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor;
                if(!_sensor_events_map_accessor->second.find(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key))
                {
                    _sensor_events_map_accessor->second.insert(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key);
                    _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor->second = true;
                }
            }
            else
            {
                EVENTS_VALID_CONCURRENT_HASHMAP _EVENTS_VALID_CONCURRENT_HASHMAP;
                _EVENTS_VALID_CONCURRENT_HASHMAP.insert(std::make_pair(key, true));
                m_sensor_events_map.insert(_sensor_events_map_accessor, _SENSOR_KEY);
                _sensor_events_map_accessor->second = std::move(_EVENTS_VALID_CONCURRENT_HASHMAP);
            }
        }
    };
    {
        SENSOR_KEY _SENSOR_KEY{sensorid, sensingmedia_id, 0};
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //获取与该实体相关的事件
        EVENTS_VALID_CONCURRENT_HASHMAP _EVENTS_VALID_CONCURRENT_HASHMAP;
        {

            SENSOR_EVENTS_CONCURRENT_HASHMAP_ConstAccessor _entity_events_map_constaccessor;
            if(m_sensor_events_map.find(_entity_events_map_constaccessor, _SENSOR_KEY))
            {
                _EVENTS_VALID_CONCURRENT_HASHMAP = _entity_events_map_constaccessor->second;
            }
        }

        std::vector<EVENT_KEY_TYPE> _EVENTS_VALID_CONCURRENT_HASHMAP_keys;
        get_keys(_EVENTS_VALID_CONCURRENT_HASHMAP, _EVENTS_VALID_CONCURRENT_HASHMAP_keys);

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //将该实体相关的事件的重置
        tbb::parallel_for(tbb::blocked_range<size_t>(0, _EVENTS_VALID_CONCURRENT_HASHMAP_keys.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (size_t i = r.begin(); i != r.end(); ++i)
                              {
                                  const auto& key = _EVENTS_VALID_CONCURRENT_HASHMAP_keys[i];
                                  EXIST_EVENTS_CONCURRENT_HASHMAP_Accessor _exist_events_map_accessor;
                                  if (this->m_exist_events_map.find(_exist_events_map_accessor, key))
                                  {
                                      //当前实体感知域存在的事件 重置 ，其他实体的不管
                                      if(((_exist_events_map_accessor->first.sensorid == sensorid) &&
                                           (_exist_events_map_accessor->first.sensingmediaid == sensingmedia_id)))
                                      {
                                          _exist_events_map_accessor->second.second = false;
                                      }
                                  }
                              }
                          });
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //获取与该实体相关的相交关系
        INTERSECTINFO_CONCURRENT_HASHMAP intersectInfos;

        auto getintersectInfos=[&](const H3INDEX h3index)
        {
            if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
            {
                gaeactorenvironment::H3IndexBufferManager::getInstance().lockIntersectForRead();
            }
            INTERSECTINFO_CONCURRENT_HASHMAP_ConstAccessor _exist_events_map_constaccessor;
            if (gaeactorenvironment::H3IndexBufferManager::getInstance().m_IntersectInfoMap.find(_exist_events_map_constaccessor, h3index))
            {
                INTERSECTINFO_CONCURRENT_HASHMAP_Accessor intersectInfos_accessor;
                intersectInfos.insert(intersectInfos_accessor, _exist_events_map_constaccessor->first);
                intersectInfos_accessor->second = _exist_events_map_constaccessor->second;
            }
            if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
            {
                gaeactorenvironment::H3IndexBufferManager::getInstance().unlockIntersect();
            }
        };

        if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_CLEAR_OLD_SENSOR_SENSINGMEDIA_FORCE_COVER_MODE())
        {
            tbb::parallel_for(tbb::blocked_range<size_t>(0, remove_hexidxslist.size()),
                              [&](const tbb::blocked_range<size_t>& r) {
                                  for (size_t i = r.begin(); i != r.end(); ++i)
                                  {
                                      const auto& h3 = remove_hexidxslist[i];
                                      getintersectInfos(h3.PARAM_seq_hexidx_element);
                                  }
                              });
        }

        tbb::parallel_for(tbb::blocked_range<size_t>(0, append_hexidxslist.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (size_t i = r.begin(); i != r.end(); ++i)
                              {
                                  const auto& h3 = append_hexidxslist[i];
                                  getintersectInfos(h3.PARAM_seq_hexidx_element);
                              }
                          });

        tbb::parallel_for(tbb::blocked_range<size_t>(0, reserve_hexidxslist.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (size_t i = r.begin(); i != r.end(); ++i)
                              {
                                  const auto& h3 = reserve_hexidxslist[i];
                                  getintersectInfos(h3.PARAM_seq_hexidx_element);
                              }
                          });

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //处理与该实体相关的相交关系


        std::vector<H3INDEX> intersectInfos_keys;
        get_keys(intersectInfos, intersectInfos_keys);


#if 0
        UINT64 curTimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
#else
        UINT64 curTimestamp = SettingsConfig::getInstance().m_simparams.m_sim_timestamp * 1000;
#endif



#if 0
        for(auto _intersectInfos_itor = intersectInfos.begin();_intersectInfos_itor != intersectInfos.end(); _intersectInfos_itor++)
        {
            const SENESOR_LIST_CONCURRENT_HASHMAP &sensorlist = _intersectInfos_itor->second.sensorlist;
            const ENTITY_LIST_CONCURRENT_HASHMAP &entitylist = _intersectInfos_itor->second.entitylist;
            for(auto sensor_list_itor = sensorlist.begin(); sensor_list_itor != sensorlist.end(); sensor_list_itor++)
            {
                const auto& sensorlist_key = sensor_list_itor->first;
                const transdata_param_seq_hexidx_hgt& sensor_hgt_range = _sensorlist_constaccessor->second;
                const TYPE_ULID &sensorulid = sensorlist_key.sensorid;
                const TYPE_ULID &sensingmediaid = sensorlist_key.sensingmediaid;
                const gaeactorenvironment::BufferInfoElement<transdata_sensorposinfo>*_sensorinfo_ptr = sensorlist_key._sensorinfo_ptr;
                //当前实体感知域存在的事件 重置 ，其他实体的不管
                if(!((sensorulid == sensorid) && (sensingmediaid == sensingmedia_id)))
                {
                    continue;
                }

                for(auto entitylist_itor = entitylist.begin(); entitylist_itor != entitylist.end(); entitylist_itor++)
                {
                    const FLOAT64 &entity_hgt = _entitylist_constaccessor->second;
                    const ENTITY_KEY& entitylist_key = entitylist_itor->first;
                    const TYPE_ULID & entityulid = entitylist_key.entityid;
                    const gaeactorenvironment::BufferInfoElement<tagEntityPosInfo>* _entityinfo_ptr = entitylist_key._entityinfo_ptr;
                    if (sensorulid == entityulid)
                    {
                        continue;
                    }

                    EVENT_KEY_TYPE eventtile = EVENT_KEY_TYPE{ sensorulid, entityulid, sensingmediaid };

                    {
                        EXIST_EVENTS_CONCURRENT_HASHMAP_Accessor _exist_events_map_accessor;
                        if (m_exist_events_map.find(_exist_events_map_accessor, eventtile))
                        {
                            EVENT_INFO & event_info = _exist_events_map_accessor->second.first;

                            _exist_events_map_accessor->second.second = true;

                            //update exist event
                            transdata_entityposinfo new_sensorposinfo = gettransdata_entityposinfo(sensorulid);
                            transdata_entityposinfo new_entityposinfo;
                            if(_entityinfo_ptr)
                            {
                                new_entityposinfo = _entityinfo_ptr->m_bufferinfo.m_entityInfo;
                            }
                            //此处控制update事件的刷新频率，
                            //当场实体发生变化 或 运动实体发生变化 【感知位置发生变化】或 【感知位置未发生变化，但更新周期到了】上次更新时间间隔超过250ms，触发发送
                            //反之场实体未发生变化 且 运动实体未发生变化 且感知时间更新周期太短 ，则不触发该更新事件的发送
                            if(event_info.m_sensorposinfo.PARAM_pos_hexidx != new_sensorposinfo.PARAM_pos_hexidx ||
                                event_info.m_entityposinfo.PARAM_pos_hexidx != new_entityposinfo.PARAM_pos_hexidx /*||
                                                                                                      curTimestamp - event_info.m_timestamp > 250*/)
                            {
                                event_info.m_sensorposinfo = std::move(new_sensorposinfo);
                                event_info.m_entityposinfo = std::move(new_entityposinfo);

                                event_info.m_sensorproprety = _sensorinfo;
                                event_info.m_distance = LocationHelper::greatDistanceM(event_info.m_sensorposinfo.PARAM_pos_hexidx, event_info.m_entityposinfo.PARAM_pos_hexidx);
                                if(_entityinfo_ptr)
                                {
                                    event_info.m_entityisSensorProprety  = _entityinfo_ptr->m_bufferinfo.m_isSensor;
                                }
//                                if (event_info.m_entityisSensorProprety)
//                                {
//                                    event_info.m_entityproprety = gettransdata_sensorposinfo(entityulid);
//                                }
                                event_info.m_sensor_hgt_range = sensor_hgt_range;
                                event_info.m_entity_hgt = entity_hgt;
                                event_info.m_timestamp = curTimestamp;
                                EVENTS_CONCURRENT_HASHMAP_Accessor concurrent_updateEventlist_accessor;
                                concurrent_updateEventlist.insert(concurrent_updateEventlist_accessor, eventtile);
                                concurrent_updateEventlist_accessor->second = _exist_events_map_accessor->second.first;
                            }
                        }
                        else
                        {
                            //add new event
                            transdata_entityposinfo _sensorposinfo = gettransdata_entityposinfo(sensorulid);
                            transdata_entityposinfo _entityposinfo;
                            bool _entityisSensorProprety;
                            if(_entityinfo_ptr)
                            {
                                _entityposinfo = _entityinfo_ptr->m_bufferinfo.m_entityInfo;
                                _entityisSensorProprety = _entityinfo_ptr->m_bufferinfo.m_isSensor;
                            }

                            auto distance = LocationHelper::greatDistanceM(_sensorposinfo.PARAM_pos_hexidx, _entityposinfo.PARAM_pos_hexidx);

                            transdata_sensorposinfo _entityproprety;
//                            if (_entityisSensorProprety)
//                            {
//                                _entityproprety = gettransdata_sensorposinfo(entityulid);
//                            }
                            transdata_sensorposinfo _sensorproprety = _sensorinfo;

                            EVENT_INFO eventinfo{ sensorulid,
                                                 entityulid,
                                                 sensingmediaid,
                                                 std::move(_sensorposinfo),
                                                 std::move(_entityposinfo),
                                                 std::move(_sensorproprety),
                                                 _entityisSensorProprety,
                                                 std::move(_entityproprety),
                                                 distance ,
                                                 curTimestamp,
                                                 sensor_hgt_range,
                                                 entity_hgt};

                            m_exist_events_map.insert(_exist_events_map_accessor, eventtile);
                            _exist_events_map_accessor->second = qMakePair(std::move(eventinfo), true);
                            appendEventRecord(eventtile);

                            EVENTS_CONCURRENT_HASHMAP_Accessor concurrent_addEventlist_accessor;
                            concurrent_addEventlist.insert(concurrent_addEventlist_accessor, eventtile);
                            concurrent_addEventlist_accessor->second = _exist_events_map_accessor->second.first;

                        }
                    }
                }
            }
        }

#else
        tbb::parallel_for(tbb::blocked_range<size_t>(0, intersectInfos_keys.size()),
                          [&](const tbb::blocked_range<size_t>& intersectInfos_keys_r) {
                              for (size_t intersectInfos_keys_i = intersectInfos_keys_r.begin(); intersectInfos_keys_i != intersectInfos_keys_r.end(); ++intersectInfos_keys_i)
                              {
                                  const auto& intersectInfos_key = intersectInfos_keys[intersectInfos_keys_i];
                                  INTERSECTINFO_CONCURRENT_HASHMAP_ConstAccessor _intersectInfos_constaccessor;
                                  if (intersectInfos.find(_intersectInfos_constaccessor, intersectInfos_key))
                                  {
                                      std::vector<SENSOR_KEY> sensorlist_keys;
                                      get_keys(_intersectInfos_constaccessor->second.sensorlist, sensorlist_keys);

                                      std::vector<ENTITY_KEY> entitylist_keys;
                                      get_keys(_intersectInfos_constaccessor->second.entitylist, entitylist_keys);

                                      tbb::parallel_for(tbb::blocked_range<size_t>(0, sensorlist_keys.size()),
                                                        [&](const tbb::blocked_range<size_t>& sensorlist_keys_r) {
                                                            for (size_t sensorlist_keys_i = sensorlist_keys_r.begin(); sensorlist_keys_i != sensorlist_keys_r.end(); ++sensorlist_keys_i)
                                                            {
                                                                const auto& sensorlist_key = sensorlist_keys[sensorlist_keys_i];
                                                                SENESOR_LIST_CONCURRENT_HASHMAP_ConstAccessor _sensorlist_constaccessor;
                                                                if (_intersectInfos_constaccessor->second.sensorlist.find(_sensorlist_constaccessor, sensorlist_key))
                                                                {
                                                                    const transdata_param_seq_hexidx_hgt& sensor_hgt_range = _sensorlist_constaccessor->second;
                                                                    const TYPE_ULID &sensorulid = _sensorlist_constaccessor->first.sensorid;
                                                                    const TYPE_ULID &sensingmediaid = _sensorlist_constaccessor->first.sensingmediaid;
                                                                    const gaeactorenvironment::BufferInfoElement<transdata_sensorposinfo>*_sensorinfo_ptr = _sensorlist_constaccessor->first._sensorinfo_ptr;
                                                                    //当前实体感知域存在的事件 重置 ，其他实体的不管
                                                                    if(!((sensorulid == sensorid) && (sensingmediaid == sensingmedia_id)))
                                                                    {
                                                                        continue;
                                                                    }
                                                                    tbb::parallel_for(tbb::blocked_range<size_t>(0, entitylist_keys.size()),
                                                                                      [&](const tbb::blocked_range<size_t>& entitylist_keys_r) {
                                                                                          for (size_t entitylist_keys_i = entitylist_keys_r.begin(); entitylist_keys_i != entitylist_keys_r.end(); ++entitylist_keys_i)
                                                                                          {
                                                                                              const ENTITY_KEY& entitylist_key = entitylist_keys[entitylist_keys_i];
                                                                                              ENTITY_LIST_CONCURRENT_HASHMAP_ConstAccessor _entitylist_constaccessor;
                                                                                              if (_intersectInfos_constaccessor->second.entitylist.find(_entitylist_constaccessor, entitylist_key))
                                                                                              {
                                                                                                  const FLOAT64 &entity_hgt = _entitylist_constaccessor->second;
                                                                                                  const TYPE_ULID & entityulid = _entitylist_constaccessor->first.entityid;
                                                                                                  const gaeactorenvironment::BufferInfoElement<tagEntityPosInfo>*_entityinfo_ptr = _entitylist_constaccessor->first._entityinfo_ptr;
                                                                                                  if (sensorulid == entityulid)
                                                                                                  {
                                                                                                      continue;
                                                                                                  }

                                                                                                  EVENT_KEY_TYPE eventtile = EVENT_KEY_TYPE{ sensorulid, entityulid, sensingmediaid };

                                                                                                  {
                                                                                                      EXIST_EVENTS_CONCURRENT_HASHMAP_Accessor _exist_events_map_accessor;
                                                                                                      if (m_exist_events_map.find(_exist_events_map_accessor, eventtile))
                                                                                                      {
                                                                                                          EVENT_INFO & event_info = _exist_events_map_accessor->second.first;

                                                                                                          _exist_events_map_accessor->second.second = true;

                                                                                                          //update exist event
                                                                                                          transdata_entityposinfo new_sensorposinfo = gettransdata_entityposinfo(sensorulid);
                                                                                                          transdata_entityposinfo new_entityposinfo;
                                                                                                          if(_entityinfo_ptr)
                                                                                                          {
                                                                                                              new_entityposinfo = _entityinfo_ptr->m_bufferinfo.m_entityInfo;
                                                                                                          }
                                                                                                          //此处控制update事件的刷新频率，
                                                                                                          //当场实体发生变化 或 运动实体发生变化 【感知位置发生变化】或 【感知位置未发生变化，但更新周期到了】上次更新时间间隔超过250ms，触发发送
                                                                                                          //反之场实体未发生变化 且 运动实体未发生变化 且感知时间更新周期太短 ，则不触发该更新事件的发送
                                                                                                          if(event_info.m_sensorposinfo.PARAM_pos_hexidx != new_sensorposinfo.PARAM_pos_hexidx ||
                                                                                                              event_info.m_entityposinfo.PARAM_pos_hexidx != new_entityposinfo.PARAM_pos_hexidx /*||
                                                                                                      curTimestamp - event_info.m_timestamp > 250*/)
                                                                                                          {
                                                                                                              event_info.m_sensorposinfo = std::move(new_sensorposinfo);
                                                                                                              event_info.m_entityposinfo = std::move(new_entityposinfo);

                                                                                                              event_info.m_sensorproprety = _sensorinfo;
                                                                                                              event_info.m_distance = LocationHelper::greatDistanceM(event_info.m_sensorposinfo.PARAM_pos_hexidx, event_info.m_entityposinfo.PARAM_pos_hexidx);
                                                                                                              if(_entityinfo_ptr)
                                                                                                              {
                                                                                                                  event_info.m_entityisSensorProprety  = _entityinfo_ptr->m_bufferinfo.m_isSensor;
                                                                                                              }
//                                                                                                              if (event_info.m_entityisSensorProprety)
//                                                                                                              {
//                                                                                                                  event_info.m_entityproprety = gettransdata_sensorposinfo(entityulid);
//                                                                                                              }
                                                                                                              event_info.m_sensor_hgt_range = sensor_hgt_range;
                                                                                                              event_info.m_entity_hgt = entity_hgt;
                                                                                                              event_info.m_timestamp = curTimestamp;
                                                                                                              EVENTS_CONCURRENT_HASHMAP_Accessor concurrent_updateEventlist_accessor;
                                                                                                              concurrent_updateEventlist.insert(concurrent_updateEventlist_accessor, eventtile);
                                                                                                              concurrent_updateEventlist_accessor->second = _exist_events_map_accessor->second.first;
                                                                                                          }
                                                                                                      }
                                                                                                      else
                                                                                                      {
                                                                                                          //add new event
                                                                                                          transdata_entityposinfo _sensorposinfo = gettransdata_entityposinfo(sensorulid);
                                                                                                          transdata_entityposinfo _entityposinfo;
                                                                                                          bool _entityisSensorProprety = false;
                                                                                                          if(_entityinfo_ptr)
                                                                                                          {
                                                                                                              _entityposinfo = _entityinfo_ptr->m_bufferinfo.m_entityInfo;
                                                                                                              _entityisSensorProprety = _entityinfo_ptr->m_bufferinfo.m_isSensor;
                                                                                                          }

                                                                                                          auto distance = LocationHelper::greatDistanceM(_sensorposinfo.PARAM_pos_hexidx, _entityposinfo.PARAM_pos_hexidx);

                                                                                                          transdata_sensorposinfo _entityproprety;
//                                                                                                          if (_entityisSensorProprety)
//                                                                                                          {
//                                                                                                              _entityproprety = gettransdata_sensorposinfo(entityulid);
//                                                                                                          }
                                                                                                          transdata_sensorposinfo _sensorproprety = _sensorinfo;
                                                                                                          EVENT_INFO eventinfo{ sensorulid,
                                                                                                                               entityulid,
                                                                                                                               sensingmediaid,
                                                                                                                               std::move(_sensorposinfo),
                                                                                                                               std::move(_entityposinfo),
                                                                                                                               std::move(_sensorproprety),
                                                                                                                               _entityisSensorProprety,
                                                                                                                               std::move(_entityproprety),
                                                                                                                               distance ,
                                                                                                                               curTimestamp,
                                                                                                                               sensor_hgt_range,
                                                                                                                               entity_hgt};

                                                                                                          m_exist_events_map.insert(_exist_events_map_accessor, eventtile);
                                                                                                          _exist_events_map_accessor->second = qMakePair(std::move(eventinfo), true);
                                                                                                          appendEventRecord(eventtile);

                                                                                                          EVENTS_CONCURRENT_HASHMAP_Accessor concurrent_addEventlist_accessor;
                                                                                                          concurrent_addEventlist.insert(concurrent_addEventlist_accessor, eventtile);
                                                                                                          concurrent_addEventlist_accessor->second = _exist_events_map_accessor->second.first;

                                                                                                      }
                                                                                                  }

                                                                                              }
                                                                                          }
                                                                                      });
                                                                }
                                                            }
                                                        });
                    }
                }
            });

#endif
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //整理 与该实体相关的事件  清理无效的

        tbb::parallel_for(tbb::blocked_range<size_t>(0, _EVENTS_VALID_CONCURRENT_HASHMAP_keys.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (size_t i = r.begin(); i != r.end(); ++i)
                              {
                                  const EVENT_KEY_TYPE& key = _EVENTS_VALID_CONCURRENT_HASHMAP_keys[i];
                                  EXIST_EVENTS_CONCURRENT_HASHMAP_Accessor _exist_events_map_accessor;
                                  if (this->m_exist_events_map.find(_exist_events_map_accessor, key))
                                  {
                                      //当前实体感知域存在的事件 重置 ，其他实体的不管
                                      if(!((_exist_events_map_accessor->first.sensorid == sensorid) &&
                                            (_exist_events_map_accessor->first.sensingmediaid == sensingmedia_id)))
                                      {
                                          continue;
                                      }
                                      if (!_exist_events_map_accessor->second.second)
                                      {
                                          EVENT_INFO event_info = _exist_events_map_accessor->second.first;
                                          event_info.m_timestamp = curTimestamp;
                                          EVENTS_CONCURRENT_HASHMAP_Accessor _clearEventlist_accessor;
                                          concurrent_clearEventlist.insert(_clearEventlist_accessor, _exist_events_map_accessor->first);
                                          _clearEventlist_accessor->second = std::move(event_info);
                                          m_exist_events_map.erase(_exist_events_map_accessor);

                                          clearEventRecord(key);
                                          continue;
                                      }
                                  }

                              }
                          });
    }

    EVENTS_HASHMAP &addEventlist = std::get<0>(eventinfo);

    EVENTS_CONCURRENT_HASHMAP_ConstIterator _addlist_itor = concurrent_addEventlist.begin();
    while (_addlist_itor != concurrent_addEventlist.end())
    {
        addEventlist.insert(std::make_pair(std::move(_addlist_itor->first), std::move(_addlist_itor->second)));
        _addlist_itor++;
    }

    EVENTS_HASHMAP &updateEventlist = std::get<2>(eventinfo);
    EVENTS_CONCURRENT_HASHMAP_ConstIterator _updatelist_itor = concurrent_updateEventlist.begin();
    while (_updatelist_itor != concurrent_updateEventlist.end())
    {
        updateEventlist.insert(std::make_pair(std::move(_updatelist_itor->first), std::move(_updatelist_itor->second)));
        _updatelist_itor++;
    }


    EVENTS_HASHMAP &clearEventlist = std::get<1>(eventinfo);
    EVENTS_CONCURRENT_HASHMAP_ConstIterator _clearlist_itor = concurrent_clearEventlist.begin();
    while (_clearlist_itor != concurrent_clearEventlist.end())
    {
        clearEventlist.insert(std::make_pair(std::move(_clearlist_itor->first), std::move(_clearlist_itor->second)));
        _clearlist_itor++;
    }
}


void GaeactorProcessor::refresh_event_type(IDENTIFI_EVENT_INFO& identifi_event_info, const TYPE_ULID &entityid, const TYPE_ULID &sensorid, const TYPE_ULID &sensingmedia_id, int itype)
{
    EASY_FUNCTION(profiler::colors::Brown900)
    EVENTS_CONCURRENT_HASHMAP concurrent_clearEventlist;
    EVENTS_CONCURRENT_HASHMAP concurrent_addEventlist;
    EVENTS_CONCURRENT_HASHMAP concurrent_updateEventlist;

    {

        auto clearEventRecord=[&](const EVENT_KEY_TYPE& key)
        {
            {

                ENTITY_EVENTS_CONCURRENT_HASHMAP_Accessor _entity_events_map_accessor;
                if(m_entity_events_map.find(_entity_events_map_accessor, key.entityid))
                {
                    EVENTS_VALID_CONCURRENT_HASHMAP_Accessor _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor;
                    if( _entity_events_map_accessor->second.find(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key))
                    {
                        _entity_events_map_accessor->second.erase(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor);
                    }
                }
            }

            {
                SENSOR_KEY _SENSOR_KEY{key.sensorid, key.sensingmediaid, 0};
                SENSOR_EVENTS_CONCURRENT_HASHMAP_Accessor _sensor_events_map_accessor;
                if(m_sensor_events_map.find(_sensor_events_map_accessor, _SENSOR_KEY))
                {
                    EVENTS_VALID_CONCURRENT_HASHMAP_Accessor _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor;
                    if( _sensor_events_map_accessor->second.find(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key))
                    {
                        _sensor_events_map_accessor->second.erase(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor);
                    }
                }
            }
        };



        auto appendEventRecord=[&](const EVENT_KEY_TYPE& key)
        {
            {

                ENTITY_EVENTS_CONCURRENT_HASHMAP_Accessor _entity_events_map_accessor;
                if(m_entity_events_map.find(_entity_events_map_accessor, key.entityid))
                {
                    EVENTS_VALID_CONCURRENT_HASHMAP_Accessor _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor;
                    if(!_entity_events_map_accessor->second.find(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key))
                    {
                        _entity_events_map_accessor->second.insert(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key);
                        _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor->second = true;
                    }
                }
                else
                {
                    EVENTS_VALID_CONCURRENT_HASHMAP _EVENTS_VALID_CONCURRENT_HASHMAP;
                    _EVENTS_VALID_CONCURRENT_HASHMAP.insert(std::make_pair(key, true));
                    m_entity_events_map.insert(_entity_events_map_accessor, key.entityid);
                    _entity_events_map_accessor->second = std::move(_EVENTS_VALID_CONCURRENT_HASHMAP);

                }
            }

            {
                SENSOR_KEY _SENSOR_KEY{key.sensorid, key.sensingmediaid, 0};
                SENSOR_EVENTS_CONCURRENT_HASHMAP_Accessor _sensor_events_map_accessor;
                if(m_sensor_events_map.find(_sensor_events_map_accessor, _SENSOR_KEY))
                {
                    EVENTS_VALID_CONCURRENT_HASHMAP_Accessor _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor;
                    if(!_sensor_events_map_accessor->second.find(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key))
                    {
                        _sensor_events_map_accessor->second.insert(_EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor, key);
                        _EVENTS_VALID_CONCURRENT_HASHMAP_tmp_accessor->second = true;
                    }
                }
                else
                {
                    EVENTS_VALID_CONCURRENT_HASHMAP _EVENTS_VALID_CONCURRENT_HASHMAP;
                    _EVENTS_VALID_CONCURRENT_HASHMAP.insert(std::make_pair(key, true));
                    m_sensor_events_map.insert(_sensor_events_map_accessor, _SENSOR_KEY);
                    _sensor_events_map_accessor->second = std::move(_EVENTS_VALID_CONCURRENT_HASHMAP);
                }
            }
        };

        {
            std::vector<EVENT_KEY_TYPE> _exist_events_map_keys;
            get_keys(m_exist_events_map, _exist_events_map_keys);

            tbb::parallel_for(tbb::blocked_range<size_t>(0, _exist_events_map_keys.size()),
                              [&](const tbb::blocked_range<size_t>& r) {
                                  for (size_t i = r.begin(); i != r.end(); ++i)
                                  {
                                      const auto& key = _exist_events_map_keys[i];
                                      EXIST_EVENTS_CONCURRENT_HASHMAP_Accessor _exist_events_map_accessor;
                                      if (this->m_exist_events_map.find(_exist_events_map_accessor, key))
                                      {
                                          if(itype == 0)
                                          {
                                              _exist_events_map_accessor->second.second = false;
                                          }
                                          else if(itype == 1)
                                          {
                                              //当前实体的存在的事件 重置 ，其他实体的不管
                                              if(_exist_events_map_accessor->first.entityid == entityid)
                                              {
                                                  _exist_events_map_accessor->second.second = false;
                                              }
                                          }
                                          else if(itype == 2)
                                          {
                                              //当前实体感知域存在的事件 重置 ，其他实体的不管
                                              if(((_exist_events_map_accessor->first.sensorid == sensorid) &&
                                                   (_exist_events_map_accessor->first.sensingmediaid == sensingmedia_id)))
                                              {
                                                  _exist_events_map_accessor->second.second = false;
                                              }
                                          }
                                      }

                                  }
                              });
        }

#if 0
        UINT64 curTimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
#else
        UINT64 curTimestamp = SettingsConfig::getInstance().m_simparams.m_sim_timestamp * 1000;
#endif

        if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
        {
            gaeactorenvironment::H3IndexBufferManager::getInstance().lockIntersectForRead();
        }
        //    INTERSECTINFO_CONCURRENT_HASHMAP intersectInfos = gaeactorenvironment::H3IndexBufferManager::getInstance().getIntersectInfos();

        INTERSECTINFO_CONCURRENT_HASHMAP intersectInfos;

        std::vector<H3INDEX> intersectInfos_keys;
        get_keys(gaeactorenvironment::H3IndexBufferManager::getInstance().m_IntersectInfoMap, intersectInfos_keys);


        tbb::parallel_for(tbb::blocked_range<size_t>(0, intersectInfos_keys.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (size_t i = r.begin(); i != r.end(); ++i)
                              {
                                  INTERSECTINFO_CONCURRENT_HASHMAP_ConstAccessor _exist_events_map_constaccessor;
                                  if (gaeactorenvironment::H3IndexBufferManager::getInstance().m_IntersectInfoMap.find(_exist_events_map_constaccessor, intersectInfos_keys[i]))
                                  {
                                      INTERSECTINFO_CONCURRENT_HASHMAP_Accessor intersectInfos_accessor;
                                      intersectInfos.insert(intersectInfos_accessor, _exist_events_map_constaccessor->first);
                                      intersectInfos_accessor->second = _exist_events_map_constaccessor->second;
                                  }
                              }
                          });

        //    for (size_t i = 0; i < intersectInfos_keys.size(); ++i)
        //    {
        //        INTERSECTINFO_CONCURRENT_HASHMAP_ConstAccessor _exist_events_map_constaccessor;
        //        if (gaeactorenvironment::H3IndexBufferManager::getInstance().m_IntersectInfoMap.find(_exist_events_map_constaccessor, intersectInfos_keys[i]))
        //        {
        //            INTERSECTINFO_CONCURRENT_HASHMAP_Accessor intersectInfos_accessor;
        //            intersectInfos.insert(intersectInfos_accessor, _exist_events_map_constaccessor->first);
        //            intersectInfos_accessor->second = _exist_events_map_constaccessor->second;
        //        }
        //    }
        if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
        {
            gaeactorenvironment::H3IndexBufferManager::getInstance().unlockIntersect();
        }


        tbb::parallel_for(tbb::blocked_range<size_t>(0, intersectInfos_keys.size()),
                          [&](const tbb::blocked_range<size_t>& intersectInfos_keys_r) {
                              for (size_t intersectInfos_keys_i = intersectInfos_keys_r.begin(); intersectInfos_keys_i != intersectInfos_keys_r.end(); ++intersectInfos_keys_i)
                              {
                                  const auto& intersectInfos_key = intersectInfos_keys[intersectInfos_keys_i];
                                  INTERSECTINFO_CONCURRENT_HASHMAP_ConstAccessor _intersectInfos_constaccessor;
                                  if (intersectInfos.find(_intersectInfos_constaccessor, intersectInfos_key))
                                  {
                                      //                                  const SENESOR_LIST_CONCURRENT_HASHMAP &sensorlist = _intersectInfos_constaccessor->second.sensorlist;
                                      //                                  const ENTITY_LIST_CONCURRENT_HASHMAP &entitylist = _intersectInfos_constaccessor->second.entitylist;

                                      std::vector<SENSOR_KEY> sensorlist_keys;
                                      get_keys(_intersectInfos_constaccessor->second.sensorlist, sensorlist_keys);

                                      std::vector<ENTITY_KEY> entitylist_keys;
                                      get_keys(_intersectInfos_constaccessor->second.entitylist, entitylist_keys);

                                      tbb::parallel_for(tbb::blocked_range<size_t>(0, sensorlist_keys.size()),
                                                        [&](const tbb::blocked_range<size_t>& sensorlist_keys_r) {
                                                            for (size_t sensorlist_keys_i = sensorlist_keys_r.begin(); sensorlist_keys_i != sensorlist_keys_r.end(); ++sensorlist_keys_i)
                                                            {
                                                                const auto& sensorlist_key = sensorlist_keys[sensorlist_keys_i];
                                                                SENESOR_LIST_CONCURRENT_HASHMAP_ConstAccessor _sensorlist_constaccessor;
                                                                if (_intersectInfos_constaccessor->second.sensorlist.find(_sensorlist_constaccessor, sensorlist_key))
                                                                {
                                                                    const transdata_param_seq_hexidx_hgt& sensor_hgt_range = _sensorlist_constaccessor->second;
                                                                    const TYPE_ULID &sensorulid = _sensorlist_constaccessor->first.sensorid;
                                                                    const TYPE_ULID &sensingmediaid = _sensorlist_constaccessor->first.sensingmediaid;
                                                                    const gaeactorenvironment::BufferInfoElement<transdata_sensorposinfo>*_sensorinfo_ptr = _sensorlist_constaccessor->first._sensorinfo_ptr;

                                                                    if(itype == 2)
                                                                    {
                                                                        //当前实体感知域存在的事件 重置 ，其他实体的不管
                                                                        if(!((sensorulid == sensorid) && (sensingmediaid == sensingmedia_id)))
                                                                        {
                                                                            continue;
                                                                        }
                                                                    }

                                                                    tbb::parallel_for(tbb::blocked_range<size_t>(0, entitylist_keys.size()),
                                                                                      [&](const tbb::blocked_range<size_t>& entitylist_keys_r) {
                                                                                          for (size_t entitylist_keys_i = entitylist_keys_r.begin(); entitylist_keys_i != entitylist_keys_r.end(); ++entitylist_keys_i)
                                                                                          {
                                                                                              const ENTITY_KEY& entitylist_key = entitylist_keys[entitylist_keys_i];
                                                                                              ENTITY_LIST_CONCURRENT_HASHMAP_ConstAccessor _entitylist_constaccessor;
                                                                                              if (_intersectInfos_constaccessor->second.entitylist.find(_entitylist_constaccessor, entitylist_key))
                                                                                              {
                                                                                                  const FLOAT64 &entity_hgt = _entitylist_constaccessor->second;
                                                                                                  const TYPE_ULID & entityulid = _entitylist_constaccessor->first.entityid;
                                                                                                  const gaeactorenvironment::BufferInfoElement<tagEntityPosInfo>*_entityinfo_ptr = _entitylist_constaccessor->first._entityinfo_ptr;
                                                                                                  if (sensorulid == entityulid)
                                                                                                  {
                                                                                                      continue;
                                                                                                  }

                                                                                                  if(itype == 1)
                                                                                                  {
                                                                                                      //其他实体不管
                                                                                                      if (entityid != entityulid)
                                                                                                      {
                                                                                                          continue;
                                                                                                      }
                                                                                                  }

                                                                                                  EVENT_KEY_TYPE eventtile = EVENT_KEY_TYPE{ sensorulid, entityulid, sensingmediaid };

                                                                                                  {
                                                                                                      EXIST_EVENTS_CONCURRENT_HASHMAP_Accessor _exist_events_map_accessor;
                                                                                                      if (m_exist_events_map.find(_exist_events_map_accessor, eventtile))
                                                                                                      {
                                                                                                          EVENT_INFO & event_info = _exist_events_map_accessor->second.first;

                                                                                                          _exist_events_map_accessor->second.second = true;

                                                                                                          //update exist event
                                                                                                          transdata_entityposinfo new_sensorposinfo = gettransdata_entityposinfo(sensorulid);
                                                                                                          transdata_entityposinfo new_entityposinfo;
                                                                                                          if(_entityinfo_ptr)
                                                                                                          {
                                                                                                              new_entityposinfo = _entityinfo_ptr->m_bufferinfo.m_entityInfo;
                                                                                                          }
                                                                                                          //此处控制update事件的刷新频率，
                                                                                                          //当场实体发生变化 或 运动实体发生变化 【感知位置发生变化】或 【感知位置未发生变化，但更新周期到了】上次更新时间间隔超过250ms，触发发送
                                                                                                          //反之场实体未发生变化 且 运动实体未发生变化 且感知时间更新周期太短 ，则不触发该更新事件的发送
                                                                                                          if(event_info.m_sensorposinfo.PARAM_pos_hexidx != new_sensorposinfo.PARAM_pos_hexidx ||
                                                                                                              event_info.m_entityposinfo.PARAM_pos_hexidx != new_entityposinfo.PARAM_pos_hexidx /*||
                                                                                                      curTimestamp - event_info.m_timestamp > 250*/)
                                                                                                          {
                                                                                                              event_info.m_sensorposinfo = std::move(new_sensorposinfo);
                                                                                                              event_info.m_entityposinfo = std::move(new_entityposinfo);

                                                                                                              if(_sensorinfo_ptr)
                                                                                                              {
                                                                                                                  event_info.m_sensorproprety = _sensorinfo_ptr->m_bufferinfo;
                                                                                                              }
                                                                                                              event_info.m_distance = LocationHelper::greatDistanceM(event_info.m_sensorposinfo.PARAM_pos_hexidx, event_info.m_entityposinfo.PARAM_pos_hexidx);
                                                                                                              if(_entityinfo_ptr)
                                                                                                              {
                                                                                                                  event_info.m_entityisSensorProprety  = _entityinfo_ptr->m_bufferinfo.m_isSensor;
                                                                                                              }
//                                                                                                              if (event_info.m_entityisSensorProprety)
//                                                                                                              {
//                                                                                                                  event_info.m_entityproprety = gettransdata_sensorposinfo(entityulid);
//                                                                                                              }
                                                                                                              event_info.m_sensor_hgt_range = sensor_hgt_range;
                                                                                                              event_info.m_entity_hgt = entity_hgt;
                                                                                                              event_info.m_timestamp = curTimestamp;
                                                                                                              EVENTS_CONCURRENT_HASHMAP_Accessor concurrent_updateEventlist_accessor;
                                                                                                              concurrent_updateEventlist.insert(concurrent_updateEventlist_accessor, eventtile);
                                                                                                              concurrent_updateEventlist_accessor->second = _exist_events_map_accessor->second.first;
                                                                                                          }
                                                                                                      }
                                                                                                      else
                                                                                                      {
                                                                                                          //add new event
                                                                                                          transdata_entityposinfo _sensorposinfo = gettransdata_entityposinfo(sensorulid);
                                                                                                          transdata_entityposinfo _entityposinfo;
                                                                                                          bool _entityisSensorProprety = false;
                                                                                                          if(_entityinfo_ptr)
                                                                                                          {
                                                                                                              _entityposinfo = _entityinfo_ptr->m_bufferinfo.m_entityInfo;
                                                                                                              _entityisSensorProprety = _entityinfo_ptr->m_bufferinfo.m_isSensor;
                                                                                                          }

                                                                                                          auto distance = LocationHelper::greatDistanceM(_sensorposinfo.PARAM_pos_hexidx, _entityposinfo.PARAM_pos_hexidx);

                                                                                                          transdata_sensorposinfo _entityproprety;
//                                                                                                          if (_entityisSensorProprety)
//                                                                                                          {
//                                                                                                              _entityproprety = gettransdata_sensorposinfo(entityulid);
//                                                                                                          }
                                                                                                          transdata_sensorposinfo _sensorproprety;
                                                                                                          if(_sensorinfo_ptr)
                                                                                                          {
                                                                                                              _sensorproprety = _sensorinfo_ptr->m_bufferinfo;
                                                                                                          }

                                                                                                          EVENT_INFO eventinfo{ sensorulid,
                                                                                                                               entityulid,
                                                                                                                               sensingmediaid,
                                                                                                                               std::move(_sensorposinfo),
                                                                                                                               std::move(_entityposinfo),
                                                                                                                               std::move(_sensorproprety),
                                                                                                                               _entityisSensorProprety,
                                                                                                                               std::move(_entityproprety),
                                                                                                                               distance ,
                                                                                                                               curTimestamp,
                                                                                                                               sensor_hgt_range,
                                                                                                                               entity_hgt};

                                                                                                          m_exist_events_map.insert(_exist_events_map_accessor, eventtile);
                                                                                                          _exist_events_map_accessor->second = qMakePair(std::move(eventinfo), true);
                                                                                                          appendEventRecord(eventtile);

                                                                                                          EVENTS_CONCURRENT_HASHMAP_Accessor concurrent_addEventlist_accessor;
                                                                                                          concurrent_addEventlist.insert(concurrent_addEventlist_accessor, eventtile);
                                                                                                          concurrent_addEventlist_accessor->second = _exist_events_map_accessor->second.first;

                                                                                                      }
                                                                                                  }

                                                                                              }
                                                                                          }
                                                                                      });
                                                                }
                                                            }
                                                        });
                                  }
                              }
                          });


        {
            std::vector<EVENT_KEY_TYPE> _exist_events_map_keys;
            get_keys(m_exist_events_map, _exist_events_map_keys);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, _exist_events_map_keys.size()),
                              [&](const tbb::blocked_range<size_t>& r) {
                                  for (size_t i = r.begin(); i != r.end(); ++i)
                                  {
                                      const auto& key = _exist_events_map_keys[i];
                                      EXIST_EVENTS_CONCURRENT_HASHMAP_Accessor _exist_events_map_accessor;
                                      if (this->m_exist_events_map.find(_exist_events_map_accessor, key))
                                      {
                                          if(itype == 1)
                                          {
                                              //其他实体不管
                                              if(_exist_events_map_accessor->first.entityid != entityid)
                                              {
                                                  continue;
                                              }
                                          }
                                          else if(itype == 2)
                                          {
                                              //当前实体感知域存在的事件 重置 ，其他实体的不管
                                              if(!((_exist_events_map_accessor->first.sensorid == sensorid) &&
                                                    (_exist_events_map_accessor->first.sensingmediaid == sensingmedia_id)))
                                              {
                                                  continue;
                                              }
                                          }
                                          if (!_exist_events_map_accessor->second.second)
                                          {
                                              EVENT_INFO event_info = _exist_events_map_accessor->second.first;
                                              event_info.m_timestamp = curTimestamp;
                                              EVENTS_CONCURRENT_HASHMAP_Accessor _clearEventlist_accessor;
                                              concurrent_clearEventlist.insert(_clearEventlist_accessor, _exist_events_map_accessor->first);
                                              _clearEventlist_accessor->second = std::move(event_info);
                                              m_exist_events_map.erase(_exist_events_map_accessor);


                                              clearEventRecord(key);
                                              continue;
                                          }
                                      }

                                  }
                              });
        }
    }



    EVENTS_HASHMAP &addEventlist = std::get<0>(identifi_event_info);

    EVENTS_CONCURRENT_HASHMAP_ConstIterator _addlist_itor = concurrent_addEventlist.begin();
    while (_addlist_itor != concurrent_addEventlist.end())
    {
        addEventlist.insert(std::make_pair(std::move(_addlist_itor->first), std::move(_addlist_itor->second)));
        _addlist_itor++;
    }

    EVENTS_HASHMAP &updateEventlist = std::get<2>(identifi_event_info);
    EVENTS_CONCURRENT_HASHMAP_ConstIterator _updatelist_itor = concurrent_updateEventlist.begin();
    while (_updatelist_itor != concurrent_updateEventlist.end())
    {
        updateEventlist.insert(std::make_pair(std::move(_updatelist_itor->first), std::move(_updatelist_itor->second)));
        _updatelist_itor++;
    }


    EVENTS_HASHMAP &clearEventlist = std::get<1>(identifi_event_info);
    EVENTS_CONCURRENT_HASHMAP_ConstIterator _clearlist_itor = concurrent_clearEventlist.begin();
    while (_clearlist_itor != concurrent_clearEventlist.end())
    {
        clearEventlist.insert(std::make_pair(std::move(_clearlist_itor->first), std::move(_clearlist_itor->second)));
        _clearlist_itor++;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

tagHexindexlistSensorInfo::tagHexindexlistSensorInfo()
{

}

tagHexindexlistSensorInfo::~tagHexindexlistSensorInfo()
{

}

void tagHexindexlistSensorInfo::deal_silent_time_out(HEXIDX_HGT_ARRAY &clearhexidxslist, UINT64 currentTimeStamp)
{
    EASY_FUNCTION(profiler::colors::Green)
    clearhexidxslist.clear();
    if (this->m_bValid && this->m_silent_time != 0)
    {
        std::vector<H3INDEX> keys;
        get_keys(this->m_hexIndexslist, keys);

        BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP build_hexIndexslist_;

        PARALLEL_BEGIN
            HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _hexIndexslist_accessor;
        if (this->m_hexIndexslist.find(_hexIndexslist_accessor, key))
        {
            if (_hexIndexslist_accessor->second.m_bHexidxValid &&
                fabs(currentTimeStamp - _hexIndexslist_accessor->second.m_timestamp) > this->m_silent_time)
            {
                BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _build_hexIndexslist_accessor;
                build_hexIndexslist_.insert(_build_hexIndexslist_accessor, _hexIndexslist_accessor->first);
                _build_hexIndexslist_accessor->second = _hexIndexslist_accessor->second.m_HexidxInfo.PARAM_seq_hexidx_hgt;
                m_hexIndexslist.erase(_hexIndexslist_accessor);
            }
        }
        PARALLEL_END

        if (m_hexIndexslist.empty())
        {
            this->m_bValid = false;
        }

        clearhexidxslist.reserve(build_hexIndexslist_.size());
        BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP::const_iterator _exist_events_map_itor = build_hexIndexslist_.begin();
        while (_exist_events_map_itor != build_hexIndexslist_.end())
        {
            clearhexidxslist.push_back(transdata_param_seq_hexidx{_exist_events_map_itor->first, _exist_events_map_itor->second});
            _exist_events_map_itor++;
        }
    }
}

void tagHexindexlistSensorInfo::deal_clear_all_hexindexs(HEXIDX_HGT_ARRAY &clearhexidxslist)
{
    clearhexidxslist.clear();
    std::vector<H3INDEX> keys;
    get_keys(this->m_hexIndexslist, keys);
    BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP build_hexIndexslist_;

    PARALLEL_BEGIN
        HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _hexIndexslist_accessor;
    if (this->m_hexIndexslist.find(_hexIndexslist_accessor, key))
    {
        BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _build_hexIndexslist_accessor;
        build_hexIndexslist_.insert(_build_hexIndexslist_accessor, _hexIndexslist_accessor->first);
        _build_hexIndexslist_accessor->second = _hexIndexslist_accessor->second.m_HexidxInfo.PARAM_seq_hexidx_hgt;
        this->m_hexIndexslist.erase(_hexIndexslist_accessor);
    }
    PARALLEL_END

    if (m_hexIndexslist.empty())
    {
        this->m_bValid = false;
    }
    SensorInfoManager::getInstance().release_sensorinfo_element(this->m_pSensorInfoElement);

    clearhexidxslist.reserve(build_hexIndexslist_.size());
    BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP::const_iterator _exist_events_map_itor = build_hexIndexslist_.begin();
    while (_exist_events_map_itor != build_hexIndexslist_.end())
    {
        clearhexidxslist.push_back(transdata_param_seq_hexidx{_exist_events_map_itor->first, _exist_events_map_itor->second});
        _exist_events_map_itor++;
    }
}

void tagHexindexlistSensorInfo::deal_clear_hexindexs_by_h3list(HEXIDX_HGT_ARRAY& clearhexidxslist, const HEXIDX_HGT_ARRAY &hexidxslist)
{
    clearhexidxslist.clear();
    //新的存在 旧的不存在 / 新的存在 旧的存在但被设置无效的

    BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP build_hexIndexslist_;

    tbb::parallel_for(tbb::blocked_range<size_t>(0, hexidxslist.size()),
                      [&](const tbb::blocked_range<size_t>& r) {
                          for (size_t i = r.begin(); i != r.end(); ++i)
                          {
                              const auto& h3 = hexidxslist[i];
                              HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _hexIndexslist_accessor;
                              if(this->m_hexIndexslist.find(_hexIndexslist_accessor, h3.PARAM_seq_hexidx_element))
                              {
                                  if (_hexIndexslist_accessor->second.m_bHexidxValid)
                                  {
                                      BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _build_hexIndexslist_accessor;
                                      build_hexIndexslist_.insert(_build_hexIndexslist_accessor, _hexIndexslist_accessor->first);
                                      _build_hexIndexslist_accessor->second = _hexIndexslist_accessor->second.m_HexidxInfo.PARAM_seq_hexidx_hgt;
                                      this->m_hexIndexslist.erase(_hexIndexslist_accessor);
                                  }
                              }
                          }
                      });

    clearhexidxslist.reserve(build_hexIndexslist_.size());
    BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP::const_iterator _exist_events_map_itor = build_hexIndexslist_.begin();
    while (_exist_events_map_itor != build_hexIndexslist_.end())
    {
        clearhexidxslist.push_back(transdata_param_seq_hexidx{_exist_events_map_itor->first, _exist_events_map_itor->second});
        _exist_events_map_itor++;
    }
}

void tagHexindexlistSensorInfo::deal_invalid_hexindexs_by_h3list(HEXIDX_HGT_ARRAY &invalidhexidxslist, const HEXIDX_HGT_ARRAY &_hexidxs)
{
    invalidhexidxslist.clear();
    //清理旧的存在 新的不存在 的无效的
    BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP build_hexIndexslist_;

    std::vector<H3INDEX> keys;
    get_keys(this->m_hexIndexslist, keys);


    PARALLEL_BEGIN
        HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _hexIndexslist_accessor;
    if(this->m_hexIndexslist.find(_hexIndexslist_accessor, key))
    {
        const H3INDEX & oldh3 = _hexIndexslist_accessor->first;
        if (_hexIndexslist_accessor->second.m_bHexidxValid)
        {
#if 0
                    auto newhexidxslist_itor = std::find_if(_hexidxs.begin(),
                                                            _hexidxs.end(), [&oldh3](const HEXIDX_HGT_ARRAY::value_type&vt) {
                                                                return vt.PARAM_seq_hexidx_element == oldh3;
                                                            });
                    if (newhexidxslist_itor == _hexidxs.end())
                    {
                        _hexIndexslist_accessor->second.m_bHexidxValid = false;

                        BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _build_hexIndexslist_accessor;
                        build_hexIndexslist_.insert(_build_hexIndexslist_accessor, _hexIndexslist_accessor->first);
                        _build_hexIndexslist_accessor->second = _hexIndexslist_accessor->second.m_HexidxInfo.PARAM_seq_hexidx_hgt;

                        this->m_hexIndexslist.erase(_hexIndexslist_accessor);
                    }
#else
            bool bexist = false;
            tbb::parallel_for(tbb::blocked_range<size_t>(0, _hexidxs.size()),
                              [&](const tbb::blocked_range<size_t>& _hexidxs_r) {
                                  for (size_t _hexidxs_i = _hexidxs_r.begin(); _hexidxs_i != _hexidxs_r.end(); ++_hexidxs_i)
                                  {
                                      if (_hexidxs[_hexidxs_i].PARAM_seq_hexidx_element == oldh3)
                                      {
                                          bexist = true;
                                          return;
                                      }
                                  }
                              });
            if(!bexist)
            {
                _hexIndexslist_accessor->second.m_bHexidxValid = false;

                BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _build_hexIndexslist_accessor;
                build_hexIndexslist_.insert(_build_hexIndexslist_accessor, _hexIndexslist_accessor->first);
                _build_hexIndexslist_accessor->second = _hexIndexslist_accessor->second.m_HexidxInfo.PARAM_seq_hexidx_hgt;
                this->m_hexIndexslist.erase(_hexIndexslist_accessor);

            }
#endif
        }
    }
    PARALLEL_END

    invalidhexidxslist.reserve(build_hexIndexslist_.size());
    BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP::const_iterator _exist_events_map_itor = build_hexIndexslist_.begin();
    while (_exist_events_map_itor != build_hexIndexslist_.end())
    {
        invalidhexidxslist.push_back(transdata_param_seq_hexidx{_exist_events_map_itor->first, _exist_events_map_itor->second});
        _exist_events_map_itor++;
    }
}


void tagHexindexlistSensorInfo::deal_newappend_hexindexs_by_h3list(HEXIDX_HGT_ARRAY &newapendhexidxslist, const HEXIDX_HGT_ARRAY &_hexidxs, const UINT64 &currentTimeStamp)
{
    newapendhexidxslist.clear();
    //清理旧的存在 新的不存在 的无效的
    BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP build_hexIndexslist_;

    tbb::parallel_for(tbb::blocked_range<size_t>(0, _hexidxs.size()),
                      [&](const tbb::blocked_range<size_t>& r) {
                          for (size_t i = r.begin(); i != r.end(); ++i)
                          {
                              const auto& _hexidxs_item = _hexidxs[i];

                              HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _hexIndexslist_accessor;
                              if (this->m_hexIndexslist.find(_hexIndexslist_accessor, _hexidxs_item.PARAM_seq_hexidx_element))
                              {
                                  if (!_hexIndexslist_accessor->second.m_bHexidxValid)
                                  {
                                      _hexIndexslist_accessor->second.m_timestamp = currentTimeStamp;
                                      _hexIndexslist_accessor->second.m_bHexidxValid = true;
                                      _hexIndexslist_accessor->second.m_HexidxInfo = _hexidxs_item;

                                      BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _build_hexIndexslist_accessor;
                                      build_hexIndexslist_.insert(_build_hexIndexslist_accessor, _hexidxs_item.PARAM_seq_hexidx_element);
                                      _build_hexIndexslist_accessor->second = _hexidxs_item.PARAM_seq_hexidx_hgt;
                                  }
                                  else
                                  {
                                      _hexIndexslist_accessor->second.m_timestamp = currentTimeStamp;
                                  }
                              }
                              else
                              {
                                  BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _build_hexIndexslist_accessor;
                                  build_hexIndexslist_.insert(_build_hexIndexslist_accessor, _hexidxs_item.PARAM_seq_hexidx_element);
                                  _build_hexIndexslist_accessor->second = _hexidxs_item.PARAM_seq_hexidx_hgt;
                                  append_hexindexinfo(_hexidxs_item.PARAM_seq_hexidx_element, currentTimeStamp, _hexidxs_item);
                              }
                          }
                      });

    newapendhexidxslist.reserve(build_hexIndexslist_.size());
    BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP::const_iterator _exist_events_map_itor = build_hexIndexslist_.begin();
    while (_exist_events_map_itor != build_hexIndexslist_.end())
    {
        newapendhexidxslist.push_back(transdata_param_seq_hexidx{_exist_events_map_itor->first, _exist_events_map_itor->second});
        _exist_events_map_itor++;
    }
}

void tagHexindexlistSensorInfo::append_hexindexinfo(const H3INDEX &h3, const UINT64 &currentTimeStamp, const transdata_param_seq_hexidx & _hexidxs_item)
{
    HEXIDX_TIME_STAMP_INFO hex_time_stamp_info;
    hex_time_stamp_info.m_bHexidxValid = true;
    hex_time_stamp_info.m_timestamp = currentTimeStamp;
    hex_time_stamp_info.m_HexidxInfo = _hexidxs_item;
    HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _hexIndexslist_accessor;
    this->m_hexIndexslist.insert(_hexIndexslist_accessor, h3);
    _hexIndexslist_accessor->second = std::move(hex_time_stamp_info);
}

void tagHexindexlistSensorInfo::deal_hexindex_sensor_remove_old_append_new(HEXIDX_HGT_ARRAY &remove_hexidxslist,
                                                                           HEXIDX_HGT_ARRAY &reserve_hexidxslist,
                                                                           HEXIDX_HGT_ARRAY &append_hexidxslist,
                                                                           const HEXIDX_HGT_ARRAY &new_hexidxslist,
                                                                           const UINT64 &currentTimeStamp)
{
    remove_hexidxslist.clear();
    reserve_hexidxslist.clear();
    append_hexidxslist.clear();
    BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP build_remove_hexIndexslist_;
    BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP build_reserve_hexIndexslist_;
    BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP build_append_hexIndexslist_;

    std::vector<H3INDEX> keys;
    get_keys(this->m_hexIndexslist, keys);


    //整理需要被清理的
    PARALLEL_BEGIN
        HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _hexIndexslist_accessor;
    if(this->m_hexIndexslist.find(_hexIndexslist_accessor, key))
    {
        const H3INDEX & oldh3 = _hexIndexslist_accessor->first;
        if (_hexIndexslist_accessor->second.m_bHexidxValid)
        {
#if 0
                auto newhexidxslist_itor = std::find_if(new_hexidxslist.begin(),
                                                        new_hexidxslist.end(), [&oldh3](const HEXIDX_HGT_ARRAY::value_type&vt) {
                                                            return vt.PARAM_seq_hexidx_element == oldh3;
                                                        });
                if (newhexidxslist_itor != new_hexidxslist.end())
                {
                    _hexIndexslist_accessor->second.m_bHexidxValid = false;
                    //旧的存在 新的不存在 移除
                    BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor build_remove_hexIndexslist_accessor;
                    build_remove_hexIndexslist_.insert(build_remove_hexIndexslist_accessor, _hexIndexslist_accessor->first);
                    build_remove_hexIndexslist_accessor->second = _hexIndexslist_accessor->second.m_HexidxInfo.PARAM_seq_hexidx_hgt;
                    //清理
                    m_hexIndexslist.erase(_hexIndexslist_accessor);
                }
    //            else
    //            {
    //                //旧的存在 新的存在 保留
    //                _hexIndexslist_accessor->second.m_bHexidxValid = true;

    //                BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor build_reserve_hexIndexslist_accessor;
    //                build_reserve_hexIndexslist_.insert(build_reserve_hexIndexslist_accessor, _hexIndexslist_accessor->first);
    //                build_reserve_hexIndexslist_accessor->second = _hexIndexslist_accessor->second.m_HexidxInfo.PARAM_seq_hexidx_hgt;

    //            }
#else
            //检查旧新是否存在
            bool bold_new_exist = false;
            tbb::parallel_for(tbb::blocked_range<size_t>(0, new_hexidxslist.size()),
                              [&](const tbb::blocked_range<size_t>& hexidxslist_r) {
                                  for (size_t hexidxslist_i = hexidxslist_r.begin(); hexidxslist_i != hexidxslist_r.end(); ++hexidxslist_i)
                                  {
                                      const auto& _hexidx = new_hexidxslist[hexidxslist_i].PARAM_seq_hexidx_element;
                                      if (_hexidx == oldh3)
                                      {
                                          //旧的存在 新的存在
                                          bold_new_exist = true;
                                          return;
                                      }
                                  }
                              });

            if(!bold_new_exist)
            {
                _hexIndexslist_accessor->second.m_bHexidxValid = false;
                //旧的存在 新的不存在 移除
                BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor build_remove_hexIndexslist_accessor;
                build_remove_hexIndexslist_.insert(build_remove_hexIndexslist_accessor, _hexIndexslist_accessor->first);
                //清理
                m_hexIndexslist.erase(_hexIndexslist_accessor);
            }
            //            else
            //            {
            //                //旧的存在 新的存在 保留
            //                _hexIndexslist_accessor->second.m_bHexidxValid = true;

//                BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor build_reserve_hexIndexslist_accessor;
//                build_reserve_hexIndexslist_.insert(build_reserve_hexIndexslist_accessor, _hexIndexslist_accessor->first);
//                build_reserve_hexIndexslist_accessor->second = _hexIndexslist_accessor->second.m_HexidxInfo.PARAM_seq_hexidx_hgt;
//            }
#endif
        }
    }
    PARALLEL_END


    tbb::parallel_for(tbb::blocked_range<size_t>(0, new_hexidxslist.size()),
                      [&](const tbb::blocked_range<size_t>& r) {
                          for (size_t i = r.begin(); i != r.end(); ++i)
                          {
                              const auto&_hexidxs_item = new_hexidxslist[i];

                              HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor _hexIndexslist_accessor;
                              if (this->m_hexIndexslist.find(_hexIndexslist_accessor, _hexidxs_item.PARAM_seq_hexidx_element))
                              {
                                  if (!_hexIndexslist_accessor->second.m_bHexidxValid)
                                  {
                                      //新增 无效的重新启用
                                      _hexIndexslist_accessor->second.m_bHexidxValid = true;
                                      _hexIndexslist_accessor->second.m_timestamp = currentTimeStamp;
                                      _hexIndexslist_accessor->second.m_HexidxInfo = _hexidxs_item;

                                      BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor build_append_hexIndexslist_accessor;
                                      build_append_hexIndexslist_.insert(build_append_hexIndexslist_accessor, _hexidxs_item.PARAM_seq_hexidx_element);
                                      build_append_hexIndexslist_accessor->second = _hexidxs_item.PARAM_seq_hexidx_hgt;
                                  }
                                  else
                                  {
                                      //旧的存在 新的存在 保留
                                      _hexIndexslist_accessor->second.m_bHexidxValid = true;
                                      _hexIndexslist_accessor->second.m_timestamp = currentTimeStamp;
                                      _hexIndexslist_accessor->second.m_HexidxInfo = _hexidxs_item;

                                      BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor build_reserve_hexIndexslist_accessor;
                                      build_reserve_hexIndexslist_.insert(build_reserve_hexIndexslist_accessor, _hexIndexslist_accessor->first);
//                                      build_reserve_hexIndexslist_accessor->second = _hexIndexslist_accessor->second.m_HexidxInfo.PARAM_seq_hexidx_hgt;
                                      build_reserve_hexIndexslist_accessor->second = _hexidxs_item.PARAM_seq_hexidx_hgt;
                                  }
                              }
                              else
                              {
                                  BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_Accessor build_append_hexIndexslist_accessor;
                                  build_append_hexIndexslist_.insert(build_append_hexIndexslist_accessor, _hexidxs_item.PARAM_seq_hexidx_element);
                                  build_append_hexIndexslist_accessor->second = _hexidxs_item.PARAM_seq_hexidx_hgt;
                                  //新增
                                  HEXIDX_TIME_STAMP_INFO hex_time_stamp_info;
                                  hex_time_stamp_info.m_bHexidxValid = true;
                                  hex_time_stamp_info.m_timestamp = currentTimeStamp;
                                  hex_time_stamp_info.m_HexidxInfo = _hexidxs_item;

                                  this->m_hexIndexslist.insert(_hexIndexslist_accessor, _hexidxs_item.PARAM_seq_hexidx_element);
                                  _hexIndexslist_accessor->second = std::move(hex_time_stamp_info);
                              }
                          }
                      });


    {
        reserve_hexidxslist.reserve(build_reserve_hexIndexslist_.size());
        BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_ConstIterator _exist_events_map_itor = build_reserve_hexIndexslist_.begin();
        while (_exist_events_map_itor != build_reserve_hexIndexslist_.end())
        {
            reserve_hexidxslist.push_back(transdata_param_seq_hexidx{_exist_events_map_itor->first, _exist_events_map_itor->second});
            _exist_events_map_itor++;
        }
    }
    {
        remove_hexidxslist.reserve(build_remove_hexIndexslist_.size());
        BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_ConstIterator _exist_events_map_itor = build_remove_hexIndexslist_.begin();
        while (_exist_events_map_itor != build_remove_hexIndexslist_.end())
        {
            remove_hexidxslist.push_back(transdata_param_seq_hexidx{_exist_events_map_itor->first, _exist_events_map_itor->second});
            _exist_events_map_itor++;
        }
    }
    {
        append_hexidxslist.reserve(build_append_hexIndexslist_.size());
        BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP_ConstIterator _exist_events_map_itor = build_append_hexIndexslist_.begin();
        while (_exist_events_map_itor != build_append_hexIndexslist_.end())
        {
            append_hexidxslist.push_back(transdata_param_seq_hexidx{_exist_events_map_itor->first, _exist_events_map_itor->second});
            _exist_events_map_itor++;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


tagHexindexSensorInfo::tagHexindexSensorInfo()
{

}

tagHexindexSensorInfo::~tagHexindexSensorInfo()
{
    clear_data();
}

void tagHexindexSensorInfo::append_HexindexlistSensorInfo(const TYPE_ULID &sensingmediaid, tagHexindexlistSensorInfo &&info)
{
    EASY_FUNCTION(profiler::colors::Green)
    SENSOR_HEXIDX_INFO_CONCURRENT_HASHMAP_Accessor _sensor_hexidx_info_map_accessor;
    this->m_sensor_hexidx_info_map.insert(_sensor_hexidx_info_map_accessor, sensingmediaid);
    _sensor_hexidx_info_map_accessor->second = std::move(info);
}

void tagHexindexSensorInfo::clear_data()
{
    std::vector<TYPE_ULID> keys;
    get_keys(this->m_sensor_hexidx_info_map, keys);

    PARALLEL_BEGIN
        TYPE_ULID sensingmediaid;
        HEXIDX_HGT_ARRAY clearhexidxslist;
        BufferInfoElement<transdata_sensorposinfo>* psensorInfoele = nullptr;
    {
        SENSOR_HEXIDX_INFO_CONCURRENT_HASHMAP_Accessor _sensor_hexidx_info_map_accessor;
        if (this->m_sensor_hexidx_info_map.find(_sensor_hexidx_info_map_accessor, key))
        {
            sensingmediaid = _sensor_hexidx_info_map_accessor->first;
            psensorInfoele = _sensor_hexidx_info_map_accessor->second.m_pSensorInfoElement;
            _sensor_hexidx_info_map_accessor->second.deal_clear_all_hexindexs(clearhexidxslist);
            //force_cover_hexindex_sensor(sensorulid,sensingmediaid,clearhexidxslist);
            this->m_sensor_hexidx_info_map.erase(_sensor_hexidx_info_map_accessor);
        }
    }
    force_cover_hexindex_sensor(sensingmediaid, psensorInfoele, clearhexidxslist);
    PARALLEL_END
}

bool tagHexindexSensorInfo::clear_hexindex_sensor(const TYPE_ULID &sensingmediaid)
{
    HEXIDX_HGT_ARRAY clearhexidxslist;
    BufferInfoElement<transdata_sensorposinfo>*psensorInfoele = nullptr;
    {
        SENSOR_HEXIDX_INFO_CONCURRENT_HASHMAP_Accessor _sensor_hexidx_info_map_accessor;
        if(this->m_sensor_hexidx_info_map.find(_sensor_hexidx_info_map_accessor, sensingmediaid))
        {
            psensorInfoele = _sensor_hexidx_info_map_accessor->second.m_pSensorInfoElement;
            _sensor_hexidx_info_map_accessor->second.deal_clear_all_hexindexs(clearhexidxslist);
            m_sensor_hexidx_info_map.erase(_sensor_hexidx_info_map_accessor);
            //force_cover_hexindex_sensor(sensorulid,sensingmediaid,clearhexidxslist);
        }
    }
    force_cover_hexindex_sensor(sensingmediaid, psensorInfoele, clearhexidxslist);
    return this->m_sensor_hexidx_info_map.empty();
}

bool tagHexindexSensorInfo::clear_hexindex_sensor_by_hexidexlist(const TYPE_ULID &sensingmediaid,const HEXIDX_HGT_ARRAY &hexidxslist)
{
    HEXIDX_HGT_ARRAY clearhexidxslist;
    BufferInfoElement<transdata_sensorposinfo>*psensorInfoele = nullptr;
    {
        SENSOR_HEXIDX_INFO_CONCURRENT_HASHMAP_Accessor _sensor_hexidx_info_map_accessor;
        if(this->m_sensor_hexidx_info_map.find(_sensor_hexidx_info_map_accessor, sensingmediaid))
        {
            psensorInfoele = _sensor_hexidx_info_map_accessor->second.m_pSensorInfoElement;
            //新的存在 旧的不存在 / 新的存在 旧的存在但被设置无效的
            _sensor_hexidx_info_map_accessor->second.deal_clear_hexindexs_by_h3list(clearhexidxslist, hexidxslist);
            m_sensor_hexidx_info_map.erase(_sensor_hexidx_info_map_accessor);
            //force_cover_hexindex_sensor(sensorulid,sensingmediaid,clearhexidxslist);
        }
    }
    force_cover_hexindex_sensor(sensingmediaid, psensorInfoele, clearhexidxslist);
    return this->m_sensor_hexidx_info_map.empty();
}

transdata_sensorposinfo tagHexindexSensorInfo::gettransdata_sensorposinfo_by_sensingmedia(const TYPE_ULID &sensingmediaid)
{
    EASY_FUNCTION(profiler::colors::Green)
    SENSOR_HEXIDX_INFO_CONCURRENT_HASHMAP_ConstAccessor _sensor_hexidx_info_map_constaccessor;
    if(this->m_sensor_hexidx_info_map.find(_sensor_hexidx_info_map_constaccessor, sensingmediaid))
    {
        //if(_sensor_hexidx_info_map_constaccessor->second.m_bValid)
        if(_sensor_hexidx_info_map_constaccessor->second.m_pSensorInfoElement && _sensor_hexidx_info_map_constaccessor->second.m_pSensorInfoElement->isValid())
        {
            return _sensor_hexidx_info_map_constaccessor->second.m_pSensorInfoElement->m_bufferinfo;
        }
    }
    return transdata_sensorposinfo();
}

bool tagHexindexSensorInfo::is_contain_sensor(const TYPE_ULID &sensingmediaid)
{
    SENSOR_HEXIDX_INFO_CONCURRENT_HASHMAP_Accessor _sensor_hexidx_info_map_accessor;
    if(this->m_sensor_hexidx_info_map.find(_sensor_hexidx_info_map_accessor, sensingmediaid))
    {
        return true;
    }
    return false;
}

bool tagHexindexSensorInfo::update_hexindex_sensor(const TYPE_ULID &sensingmediaid,
                                                   const HEXIDX_HGT_ARRAY &_hexidxs,
                                                   transdata_sensorposinfo &&_sensorinfo,
                                                   POLYGON_LIST &&_polygon,
                                                   const UINT64 &currentTimeStamp,
                                                   HEXIDX_HGT_ARRAY &remove_hexidxslist,
                                                   HEXIDX_HGT_ARRAY &reserve_hexidxslist,
                                                   HEXIDX_HGT_ARRAY &append_hexidxslist)
{
    UINT8 usage = _sensorinfo.PARAM_wave_usage;
    UINT32 silent_time = _sensorinfo.PARAM_wave_silent_time_gap;
    BufferInfoElement<transdata_sensorposinfo>*psensorInfoele=nullptr;
    auto deal_list=[&](){
        if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_CLEAR_OLD_SENSOR_SENSINGMEDIA_FORCE_COVER_MODE())
        {
            tbb::parallel_for(tbb::blocked_range<size_t>(0, remove_hexidxslist.size()),
                              [&](const tbb::blocked_range<size_t>& r) {
                                  for (size_t i = r.begin(); i != r.end(); ++i)
                                  {
                                      const auto& h3 = remove_hexidxslist[i];
                                      gaeactorenvironment::H3IndexBufferManager::getInstance().deal_hexindex_sensor(SENSOR_KEY{m_sensorulid, sensingmediaid, psensorInfoele}, h3, true);
                                  }
                              });
        }

        tbb::parallel_for(tbb::blocked_range<size_t>(0, append_hexidxslist.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (size_t i = r.begin(); i != r.end(); ++i)
                              {
                                  const auto& h3 = append_hexidxslist[i];
                                  gaeactorenvironment::H3IndexBufferManager::getInstance().deal_hexindex_sensor(SENSOR_KEY{m_sensorulid, sensingmediaid, psensorInfoele}, h3, false);
                              }
                          });

        tbb::parallel_for(tbb::blocked_range<size_t>(0, reserve_hexidxslist.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (size_t i = r.begin(); i != r.end(); ++i)
                              {
                                  const auto& h3 = reserve_hexidxslist[i];
                                  gaeactorenvironment::H3IndexBufferManager::getInstance().deal_hexindex_sensor(SENSOR_KEY{m_sensorulid, sensingmediaid, psensorInfoele}, h3, false);
                              }
                          });
    };
    bool bDeal = false;

    {
        SENSOR_HEXIDX_INFO_CONCURRENT_HASHMAP_Accessor _sensor_hexidx_info_map_accessor;
        if(this->m_sensor_hexidx_info_map.find(_sensor_hexidx_info_map_accessor, sensingmediaid))
        {
            _sensor_hexidx_info_map_accessor->second.deal_hexindex_sensor_remove_old_append_new(remove_hexidxslist, reserve_hexidxslist, append_hexidxslist, _hexidxs, currentTimeStamp);

            psensorInfoele = _sensor_hexidx_info_map_accessor->second.m_pSensorInfoElement;
            if (!_hexidxs.empty())
            {
                if(_sensor_hexidx_info_map_accessor->second.m_pSensorInfoElement)
                {
                    _sensor_hexidx_info_map_accessor->second.m_pSensorInfoElement->update(std::move(_sensorinfo));
                }
                //_sensor_hexidx_info_map_accessor->second.m_polygon = std::move(_polygon);
                _sensor_hexidx_info_map_accessor->second.m_bValid = true;
                _sensor_hexidx_info_map_accessor->second.m_silent_time = silent_time;
                _sensor_hexidx_info_map_accessor->second.m_lastUpdateTimestamp = currentTimeStamp;
            }
            bDeal = true;
        }
    }
    deal_list();


    return bDeal;
}

void tagHexindexSensorInfo::force_cover_hexindex_sensor(const TYPE_ULID &sensingmediaid, BufferInfoElement<transdata_sensorposinfo> *psensorInfoele, const HEXIDX_HGT_ARRAY &hexidxslist)
{
    tbb::parallel_for(tbb::blocked_range<size_t>(0, hexidxslist.size()),
                      [&](const tbb::blocked_range<size_t>& r) {
                          for (size_t i = r.begin(); i != r.end(); ++i)
                          {
                              gaeactorenvironment::H3IndexBufferManager::getInstance().deal_hexindex_sensor(SENSOR_KEY{m_sensorulid, sensingmediaid, psensorInfoele}, hexidxslist[i], true);
                          }
                      });
}

void tagHexindexSensorInfo::force_cover_deal_hexindex_sensor(const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY& hexidxslist)
{
    BufferInfoElement<transdata_sensorposinfo>* psensorInfoele = nullptr;
    HEXIDX_HGT_ARRAY invalidhexidxslist;
    {
        SENSOR_HEXIDX_INFO_CONCURRENT_HASHMAP_Accessor _sensor_hexidx_info_map_accessor;
        if(this->m_sensor_hexidx_info_map.find(_sensor_hexidx_info_map_accessor, sensingmediaid))
        {
            psensorInfoele = _sensor_hexidx_info_map_accessor->second.m_pSensorInfoElement;
            //清理旧的存在 新的不存在 的无效的
            _sensor_hexidx_info_map_accessor->second.deal_invalid_hexindexs_by_h3list(invalidhexidxslist, hexidxslist);
            //force_cover_hexindex_sensor(sensorulid, sensingmediaid, invalidhexidxslist);
        }
    }
    force_cover_hexindex_sensor(sensingmediaid, psensorInfoele, invalidhexidxslist);
}

void tagHexindexSensorInfo::force_cover_update_hexindex_sensor(const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &_hexidxs)
{
    BufferInfoElement<transdata_sensorposinfo>* psensorInfoele = nullptr;
    HEXIDX_HGT_ARRAY invalidhexidxslist;
    {
        SENSOR_HEXIDX_INFO_CONCURRENT_HASHMAP_Accessor _sensor_hexidx_info_map_accessor;
        if(this->m_sensor_hexidx_info_map.find(_sensor_hexidx_info_map_accessor, sensingmediaid))
        {
            psensorInfoele = _sensor_hexidx_info_map_accessor->second.m_pSensorInfoElement;
            //清理旧的存在 新的不存在 的无效的
            _sensor_hexidx_info_map_accessor->second.deal_invalid_hexindexs_by_h3list(invalidhexidxslist, _hexidxs);
            //force_cover_hexindex_sensor(sensorulid, sensingmediaid, invalidhexidxslist);
        }
    }
    force_cover_hexindex_sensor(sensingmediaid, psensorInfoele, invalidhexidxslist);
}


void tagHexindexSensorInfo::refresh_silent_timeout(UINT64 currentTimeStamp)
{
    std::vector<TYPE_ULID> keys;
    get_keys(this->m_sensor_hexidx_info_map, keys);

    PARALLEL_BEGIN
        SENSOR_HEXIDX_INFO_CONCURRENT_HASHMAP_Accessor _sensor_hexidx_info_map_accessor;
    if (this->m_sensor_hexidx_info_map.find(_sensor_hexidx_info_map_accessor, key))
    {
        const TYPE_ULID &sensingmediaid = _sensor_hexidx_info_map_accessor->first;
        if(_sensor_hexidx_info_map_accessor->second.m_bValid)
        {
            if(_sensor_hexidx_info_map_accessor->second.m_silent_time != 0)
            {
                HEXIDX_HGT_ARRAY clearhexidxslist;
                _sensor_hexidx_info_map_accessor->second.deal_silent_time_out(clearhexidxslist, currentTimeStamp);

                force_cover_hexindex_sensor(sensingmediaid,_sensor_hexidx_info_map_accessor->second.m_pSensorInfoElement, clearhexidxslist);
            }
        }
    }
    PARALLEL_END
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SensorInfoManager::SensorInfoManager()
{
    m_sensor_info_buffer_manager.init(SENSOR_INDEX_BUFFER_SIZE);
}


SensorInfoManager::~SensorInfoManager()
{

}


SensorInfoManager &SensorInfoManager::getInstance()
{
    static SensorInfoManager sensorinfomanager;
    return sensorinfomanager;
}


BufferInfoElement<transdata_sensorposinfo> *SensorInfoManager::alloc_sensorinfo_element(transdata_sensorposinfo &&_sensorinfo)
{
    EASY_FUNCTION(profiler::colors::Green)

    return m_sensor_info_buffer_manager.alloc_info_element(std::move(_sensorinfo));
}

void SensorInfoManager::release_sensorinfo_element(BufferInfoElement<transdata_sensorposinfo> *release_ele)
{
    return m_sensor_info_buffer_manager.release_sensorinfo_element(release_ele);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


EntityInfoManager::EntityInfoManager()
{
    m_entity_info_buffer_manager.init(SENSOR_INDEX_BUFFER_SIZE);
}


EntityInfoManager::~EntityInfoManager()
{

}


EntityInfoManager &EntityInfoManager::getInstance()
{
    static EntityInfoManager entityinfomanager;
    return entityinfomanager;
}


BufferInfoElement<tagEntityPosInfo> *EntityInfoManager::alloc_entityinfo_element(tagEntityPosInfo &&_entityinfo)
{
    return m_entity_info_buffer_manager.alloc_info_element(std::move(_entityinfo));
}

void EntityInfoManager::release_entityinfo_element(BufferInfoElement<tagEntityPosInfo> *release_ele)
{
    return m_entity_info_buffer_manager.release_sensorinfo_element(release_ele);
}



}
