#ifndef GAEACTOR_ENVIRONMENT_TBB_DATAHELPER_H
#define GAEACTOR_ENVIRONMENT_TBB_DATAHELPER_H
#include "tbb/concurrent_hash_map.h"
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"

#include "head_define.h"

#define DECLARE_TYPEDEF_TBB_HASH_MAP(MAPINFO_KEY,MAPINFO_VAL,HASHMAP_ALIASNAME)  \
    typedef tbb::concurrent_hash_map<MAPINFO_KEY,MAPINFO_VAL> HASHMAP_ALIASNAME;\
    typedef typename HASHMAP_ALIASNAME::const_accessor HASHMAP_ALIASNAME##_ConstAccessor;\
    typedef typename HASHMAP_ALIASNAME::accessor HASHMAP_ALIASNAME##_Accessor;\
    typedef typename HASHMAP_ALIASNAME::iterator HASHMAP_ALIASNAME##_Iterator;\
    typedef typename HASHMAP_ALIASNAME::const_iterator  HASHMAP_ALIASNAME##_ConstIterator;\
    typedef HASHMAP_ALIASNAME::value_type HASHMAP_ALIASNAME##_ValuePair;


namespace gaeactorenvironment
{
template <typename T>
class BufferInfoElement;
}

#define INVALID_SENSORINFO_INDEX (-1)

typedef uint32_t BUFFERINFO_INDEX_TYPE;

typedef struct sensor_sensingmedia_index_key_type
{
    TYPE_ULID sensorid;
    TYPE_ULID sensingmediaid;
    gaeactorenvironment::BufferInfoElement<transdata_sensorposinfo>* _sensorinfo_ptr;
}SENSOR_KEY;


namespace std {
template <>
struct hash<SENSOR_KEY> {
    size_t operator()(const SENSOR_KEY& p)const {
        return std::hash<TYPE_ULID>()(p.sensorid)
               ^ std::hash<TYPE_ULID>()(p.sensingmediaid)
               ^ std::hash<gaeactorenvironment::BufferInfoElement<transdata_sensorposinfo>*>()(p._sensorinfo_ptr);
    }
};

template <>
struct equal_to<SENSOR_KEY> {
    bool operator()(const SENSOR_KEY& left, const SENSOR_KEY& right) const {
        return std::tie(left.sensorid, left.sensingmediaid, left._sensorinfo_ptr) == std::tie(right.sensorid, right.sensingmediaid, right._sensorinfo_ptr);
    }
};
};


typedef QPair<EVENT_INFO, bool> EVENT_VALID_STATUS;
DECLARE_TYPEDEF_TBB_HASH_MAP(EVENT_KEY_TYPE,EVENT_VALID_STATUS, EXIST_EVENTS_CONCURRENT_HASHMAP);

DECLARE_TYPEDEF_TBB_HASH_MAP(SENSOR_KEY, transdata_param_seq_hexidx_hgt, SENESOR_LIST_CONCURRENT_HASHMAP);


struct tagEntityPosInfo
{
    transdata_entityposinfo m_entityInfo;
    bool m_isSensor;
};

typedef struct entity_index_key_type
{
    TYPE_ULID entityid;
    gaeactorenvironment::BufferInfoElement<tagEntityPosInfo>* _entityinfo_ptr;
}ENTITY_KEY;


namespace std {
template <>
struct hash<ENTITY_KEY> {
    size_t operator()(const ENTITY_KEY& p)const {
        return std::hash<TYPE_ULID>()(p.entityid)
               ^ std::hash<gaeactorenvironment::BufferInfoElement<tagEntityPosInfo>*>()(p._entityinfo_ptr);
    }
};

template <>
struct equal_to<ENTITY_KEY> {
    bool operator()(const ENTITY_KEY& left, const ENTITY_KEY& right) const {
        return std::tie(left.entityid, left._entityinfo_ptr) == std::tie(right.entityid, right._entityinfo_ptr);
    }
};
};

DECLARE_TYPEDEF_TBB_HASH_MAP(ENTITY_KEY, FLOAT64, ENTITY_LIST_CONCURRENT_HASHMAP);


DECLARE_TYPEDEF_TBB_HASH_MAP(EVENT_KEY_TYPE, EVENT_INFO, EVENTS_CONCURRENT_HASHMAP)

struct tagConcurrentIntersectInfo
{
    SENESOR_LIST_CONCURRENT_HASHMAP sensorlist;
    ENTITY_LIST_CONCURRENT_HASHMAP entitylist;
};

DECLARE_TYPEDEF_TBB_HASH_MAP(H3INDEX, tagConcurrentIntersectInfo, INTERSECTINFO_CONCURRENT_HASHMAP)

#endif // GAEACTOR_ENVIRONMENT_TBB_DATAHELPER_H
