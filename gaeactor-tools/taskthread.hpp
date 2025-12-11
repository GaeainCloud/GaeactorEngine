#ifndef TASKTHREAD_H
#define TASKTHREAD_H
#include <QJsonDocument>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <unordered_map>
#include <QReadWriteLock>

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <QRunnable>

#include <glm/vec3.hpp>

#include "function.h"

#include "transformdata_define.h"

#define CALC_PATH

enum E_GEOTYPE
{
    E_GEOTYPE_POINT,
    E_GEOTYPE_LINE,
    E_GEOTYPE_POLYGON,
    E_GEOTYPE_MULITPOLYGON
};

struct  tagGeoJsonInfo
{
    QString name;
    QColor m_color;
    E_GEOTYPE type;
    QJsonObject properties;
    std::vector<LATLNGS_VECTOR> coordinates;

    ////////////////////////////////////////////////////////
    // lines info
    QMap<QString,QString> m_tags;
    int z_order;
    std::vector<std::tuple<LATLNGS_VECTOR, QColor>> coordinatesExtend;
    ////////////////////////////////////////////////////////
    /// \brief dealPolygonInfo
    ///
    void dealInfo()
    {
        if (type == E_GEOTYPE_LINE)
        {
            dealLineInfo();
        }
        else if (type == E_GEOTYPE_MULITPOLYGON || type == E_GEOTYPE_POLYGON)
        {
            dealPolygonInfo();
        }

        else if (type == E_GEOTYPE_POINT)
        {
            dealPointInfo();
        }
    }

    void dealPolygonInfo()
    {
        QColor cl = FunctionAssistant::randColor(64);
        if(!properties.value("aeroway").toString().isEmpty())
        {
            if( properties.value("aeroway").toString() == "taxiway")
            {
//                cl = QColor(64, 64, 64, 128);
				cl = QColor(255, 255, 0, 128);
            }
            else if( properties.value("aeroway").toString() == "aerodrome")
            {
                cl = QColor(64, 64, 64, 64);
            }
            else if( properties.value("aeroway").toString() == "apron")
            {
                cl = QColor(64, 64, 64, 32);
            }
        }

        else if(!properties.value("landuse").toString().isEmpty())
        {
            if( properties.value("landuse").toString() == "grass")
            {
                cl = QColor(0, 192, 0, 192);
            }
            else if( properties.value("landuse").toString() == "farmland")
            {
                cl = QColor(128, 128, 255, 192);
            }
            else if( properties.value("landuse").toString() == "construction")
            {
                cl = QColor(128, 128, 0, 192);
            }
        }

        else if(!properties.value("natural").toString().isEmpty())
        {
            if( properties.value("natural").toString() == "water")
            {
                cl = QColor(0, 192, 192, 192);
            }
        }

        else if(!properties.value("amenity").toString().isEmpty())
        {
            if( properties.value("amenity").toString() == "fuel")
            {
                cl = QColor(255, 255, 0, 192);
            }
            else if( properties.value("amenity").toString() == "parking")
            {
                cl = QColor(255, 255, 0, 128);
            }
        }
        m_color = cl;
    }

    void dealLineInfo()
    {
        double width = 4.0f;
        width = m_tags.value("width","4.0").toDouble();
        width = width/2;

        QColor cl = cl = QColor(0, 0, 255, 255);

        if(!properties.value("highway").toString().isEmpty())
        {
            if( m_tags.value("service") == "parking_aisle")
            {
                cl = QColor(0, 0, 255, 128);
                width = 3;
            }
            else if(m_tags.value("service") == "private")
            {
                cl = QColor(255, 0, 0, 128);
                width = 3;
            }
            else if(m_tags.value("service") == "driveway")
            {
                cl = QColor(255, 255, 0, 128);
                width = 3;
            }

            if(m_tags.value("access") == "private")
            {
                cl = QColor(255, 0, 0, 192);
                width = 3;
            }

            QString highway = properties.value("highway").toString();

            if(highway == "footway")
            {
                cl = QColor(0, 64, 64, 128);
                width = 2;
            }
            else if(highway == "tertiary" || highway == "tertiary_link")
            {
                cl = QColor(0, 128, 128, 128);
                width = 3;
            }
            else if(highway == "secondary" || highway == "secondary_link")
            {
                cl = QColor(0, 192, 192, 128);
                width = 4;
            }
            else if(highway == "primary")
            {
                cl = QColor(0, 255, 255, 128);
                width = 5;
            }
            else if(highway == "trunk" || highway == "trunk_link")
            {
                cl = QColor(0, 255, 255, 192);
                width = 6;
            }
            else if(highway == "construction")
            {
                cl = QColor(0, 255, 255, 64);
                width = 5;
            }
            else if(highway == "residential")
            {
                cl = QColor(0, 255, 255, 32);
                width = 2;
            }
            else if(highway == "steps")
            {
                cl = QColor(0, 128, 0, 32);
                width = 2;
            }
            else if(highway == "unclassified")
            {
                cl = QColor(255, 255, 0, 32);
                width = 4;
            }
        }
        else if(m_tags.contains("aeroway"))
        {
            if(m_tags.value("aeroway") == "runway")
            {
				cl = QColor(255, 255, 0, 255);
                width = 30;
            }
            else if(m_tags.value("aeroway") == "taxiway")
            {
				cl = QColor(255, 255, 0, 192);
                width = 20;
            }
            else if(m_tags.value("aeroway") == "parking_position")
            {
                cl = QColor(255, 255, 0, 128);
                width = 10;
			}
			else if (m_tags.value("aeroway") == "stopbar")
			{
				cl = QColor(0, 255, 255, 128);
				width = 15;
			}
			else if (m_tags.value("aeroway") == "landingpoint")
			{
				cl = QColor(0, 255, 0, 128);
				width = 15;
			}
			else if (m_tags.value("aeroway") == "holdingpoint")
			{
				cl = QColor(255, 255, 0, 128);
				width = 15;
			}
			else if (m_tags.value("aeroway") == "transrunway")
			{
				cl = QColor(255, 0, 0, 128);
				width = 15;
			}
			else if (m_tags.value("aeroway") == "tr_checkbar_stopbar")
			{
				cl = QColor(255, 0, 255, 128);
				width = 15;
			}
			else if (m_tags.value("aeroway") == "tr_checkbar")
			{
				cl = QColor(255, 255, 0, 128);
				width = 15;
			}
			else if (m_tags.value("aeroway") == "vacaterunway")
			{
				cl = QColor(0, 192, 0, 128);
				width = 5;
			}
			else if (m_tags.value("aeroway") == "tr_stopbar")
			{
				cl = QColor(0, 192, 192, 128);
				width = 15;
			}
        }
        else if(!properties.value("waterway").toString().isEmpty())
        {
            auto waterway = properties.value("waterway").toString();
            if(waterway == "drain")
            {
                cl = QColor(0, 255, 255, 128);
                width = 4.0f;
            }
            else if(waterway == "canal")
            {
                cl = QColor(0, 255, 255, 192);
                width = 6.0f;
            }
            else if(waterway == "river")
            {
                cl = QColor(0, 0, 255, 192);
                width = 10.0f;
            }
        }
        else if(!properties.value("barrier").toString().isEmpty())
        {
            cl = QColor(0, 255, 0, 192);
            width = 2.0f;
        }
        else if(!properties.value("man_made").toString().isEmpty())
        {
            cl = QColor(255, 0, 0, 192);
            width = 4.0f;
        }
        else if(!properties.value("railway").toString().isEmpty())
        {
            auto railway = properties.value("railway").toString();
            if(railway == "rail")
            {
                cl = QColor(0xe9, 0x99, 0x4a, 128);
                width = 4.0f;
            }
            else if(railway == "subway")
            {
                cl = QColor(0xe9, 0x99, 0x4a, 192);
                width = 4.0f;
            }
        }

        m_color = cl;
        coordinatesExtend.clear();
        for(auto latlnglist:coordinates)
        {
            LATLNGS_VECTOR extentmp = FunctionAssistant::extendLineToPolygonPlane(latlnglist, width);
            coordinatesExtend.emplace_back(std::make_tuple(std::move(extentmp), cl));
        }
    }

    void dealPointInfo()
    {
        QColor cl = FunctionAssistant::randColor(128);
        m_color = cl;
    }
};


#define PARSE_STRING_FROM_JSON(DST,SRC,ELE) DST.ELE = SRC.value(#ELE).toString().toStdString();

#define PARSE_DOUBLE_FROM_JSON(DST,SRC,ELE) DST.ELE = SRC.value(#ELE).toDouble();

#define PARSE_LATLNG_FROME_JSON(DST, SRC ) {\
auto array = SRC.toArray();\
    LAT_LNG latlng;\
    latlng.lng = array.at(0).toDouble();\
    latlng.lat = array.at(1).toDouble();\
    DST.emplace_back(std::move(latlng));\
}



struct GeoJsonInfos
{
    QString name;
    E_GEOTYPE type;
    QHash <QString , tagGeoJsonInfo> subItem;
    GeoJsonInfos()
    {
    }
};

#include <QJsonArray>

struct AgentKeyItemInfo
{
    QString agentKey;
    QString agentId;
    QString agentKeyword;
    QString agentName;
    QString agentNameI18n;
    QString agentType;
    QString modelUrlSlim;
    QString modelUrlFat;
    QString modelUrlMedium;
    QJsonArray modelUrlSymbols;

	void toJson(QJsonObject& jsitem)
	{
        jsitem.insert("agentKey", agentKey);
        jsitem.insert("agentId", agentId);
        jsitem.insert("agentKeyword", agentKeyword);
        jsitem.insert("agentName", agentName);
        jsitem.insert("agentNameI18n", agentNameI18n);
        jsitem.insert("agentType", agentType);
        jsitem.insert("modelUrlSlim", modelUrlSlim);
        jsitem.insert("modelUrlFat", modelUrlFat);
        jsitem.insert("modelUrlMedium", modelUrlMedium);
        jsitem.insert("modelUrlSymbols", modelUrlSymbols);
	}

	void fromJson(const QJsonObject& jsitem)
	{
		agentKey = jsitem.value("agentKey").toString();
        agentId = jsitem.value("agentId").toString();
        agentKeyword = jsitem.value("agentKeyword").toString();
        agentName = jsitem.value("agentName").toString();
        agentNameI18n = jsitem.value("agentNameI18n").toString();
        agentType = jsitem.value("agentType").toString();
        modelUrlSlim = jsitem.value("modelUrlSlim").toString();
        modelUrlFat = jsitem.value("modelUrlFat").toString();
        modelUrlMedium = jsitem.value("modelUrlMedium").toString();
        modelUrlSymbols = jsitem.value("modelUrlSymbols").toArray();
	}
};


struct AgentInstanceItemInfo
{
    QString agentInstId;
    QString agentOffsetKey;
    QString asmKey;

    AgentKeyItemInfo agentKeyItem;

    void toJson(QJsonObject& jsitem)
    {
        jsitem.insert("agentInstId", agentInstId);
        jsitem.insert("agentOffsetKey", agentOffsetKey);
        jsitem.insert("asmKey", asmKey);
        agentKeyItem.toJson(jsitem);
    }

    void fromJson(const QJsonObject& jsitem)
    {
        agentInstId = jsitem.value("agentInstId").toString();
        agentOffsetKey = jsitem.value("agentOffsetKey").toString();
        asmKey = jsitem.value("asmKey").toString();
        agentKeyItem.fromJson(jsitem);
    }
};

struct AgentInstanceInfo
{
    AgentInstanceItemInfo m_agentinfo;
    std::vector<AgentInstanceItemInfo> m_subagentinfo;

    void toJson(QJsonObject &jsitem)
    {
        m_agentinfo.toJson(jsitem);
    }

    void fromJson(const QJsonObject& jsitem) 
    {
		m_agentinfo.fromJson(jsitem);
    }
};


enum E_FLIGHT_DEP_ARR_TYPE:UINT32
{
	E_FLIGHT_DEP_ARR_TYPE_DEP,
	E_FLIGHT_DEP_ARR_TYPE_ARR
};
#define TAKEOFF_AHEADTIME_MIN (30)
#define LANDING_AHEADTIME_MIN (10)
#define LANDING_BEHINDTIME_MIN (10)

struct FlightPlanConf
{
    QString m_Date;
    QString m_FilghtNumber;
    QString m_DepArrFlag;
	E_FLIGHT_DEP_ARR_TYPE m_flight_dep_arr_type;
    QString m_PlaneNum;
    QString m_PlaneType;
    QString m_FlightClass;
    QString m_FlightLeg;
    QString m_FlightStartPlace;
    QString m_FlightEndPlace;
    uint64_t m_aheadtimelen;

	uint64_t m_PlanDateTimeTakeOff_ahead_timestamp;
    QString m_PlanDateTimeTakeOff;
	uint64_t m_PlanDateTimeTakeOff_timestamp;
	QString m_ExpectedDateTimeTakeOff;
    QString m_RealityDateTimeTakeOff;

	uint64_t m_PlanDateTimeLanding_ahead_timestamp;
	QString m_PlanDateTimeLanding;
	uint64_t m_PlanDateTimeLanding_timestamp;
	QString m_ExpectedDateTimeLanding;
    QString m_RealityDateTimeLanding;
	uint64_t m_PlanDateTimeLanding_behind_timestamp;
	uint64_t m_behindtimelen;

    QString m_Delay;
    QString m_Seat;
    QString m_Terminal;
    QString m_Runway;
    FlightPlanConf()
    {

    }

	void updatePlanDateTimeTakeOff_timestamp(uint64_t _PlanDateTimeTakeOff_timestamp)
	{
		this->m_PlanDateTimeTakeOff_timestamp = _PlanDateTimeTakeOff_timestamp;
		this->m_aheadtimelen = TAKEOFF_AHEADTIME_MIN * 60;
		this->m_PlanDateTimeTakeOff_ahead_timestamp = this->m_PlanDateTimeTakeOff_timestamp - this->m_aheadtimelen;
	}
	void updatePlanDateTimeLanding_timestamp(uint64_t _PlanDateTimeLanding_timestamp)
	{
		this->m_PlanDateTimeLanding_timestamp = _PlanDateTimeLanding_timestamp;
		this->m_aheadtimelen = LANDING_AHEADTIME_MIN * 60;
		this->m_PlanDateTimeLanding_ahead_timestamp = this->m_PlanDateTimeLanding_timestamp - this->m_aheadtimelen;
		this->m_behindtimelen = LANDING_BEHINDTIME_MIN * 60;
		this->m_PlanDateTimeLanding_behind_timestamp = this->m_PlanDateTimeLanding_timestamp + this->m_behindtimelen;		
	}
};


struct tagFlightEventTime
{
	uint64_t m_itime;
	uint64_t m_eventid;
	uint64_t m_day_senscod_offset_s;
	uint64_t m_prev_with_offset_s;
	uint64_t m_next_with_offset_s;

	uint64_t m_day_senscod_offset_ms;
	uint64_t m_prev_with_offset_ms;
	uint64_t m_next_with_offset_ms;


	std::list<FlightPlanConf*> m_flightCfgs;
	tagFlightEventTime * m_prev_;
	tagFlightEventTime * m_next_;
	tagFlightEventTime()
	{
		m_itime = 0;
		m_eventid = 0;
		m_day_senscod_offset_s = 0;
		m_prev_with_offset_s = 0;
		m_next_with_offset_s = 0;
		m_day_senscod_offset_ms = 0;
		m_prev_with_offset_ms = 0;
		m_next_with_offset_ms = 0;
		m_prev_ = nullptr;
		m_next_ = nullptr;
	}

	tagFlightEventTime(uint64_t itime, uint64_t eventid, uint64_t idayoffset)
	{
		m_itime = itime;
		m_eventid = eventid;
		m_day_senscod_offset_s = idayoffset;
		m_prev_with_offset_s = 0;
		m_next_with_offset_s = 0;
		m_day_senscod_offset_ms = idayoffset * 1000;
		m_prev_with_offset_ms = 0;
		m_next_with_offset_ms = 0;
		m_prev_ = nullptr;
		m_next_ = nullptr;
	}

	void setPrev(tagFlightEventTime * prev_)
	{
		m_prev_ = prev_;
		if (prev_)
		{
			m_prev_with_offset_s = m_itime - prev_->m_itime;
			m_prev_with_offset_ms = m_prev_with_offset_s * 1000;
		}
	}

	void setNext(tagFlightEventTime * next_)
	{
		m_next_ = next_;
		if (next_)
		{
			m_next_with_offset_s = next_->m_itime - m_itime;
			m_next_with_offset_ms = m_next_with_offset_s * 1000;

		}
	}

	void appendFlightCfg(FlightPlanConf* flightcfg)
	{
		m_flightCfgs.push_back(flightcfg);
	}
};

namespace std {
	template<> struct hash<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>> {
		size_t operator()(const std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>& p)const {
			return hash<QString>()(std::get<0>(p)) ^ \
				hash<uint32_t>()(std::get<1>(p)) ^ \
				hash<QString>()(std::get<2>(p));
		}
	};
	template <>
	struct equal_to<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>> {
		bool operator()(const std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>& left, const std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>& right) const {
			return left == right;
		}
	};
};

struct tagPoiItem
{
	QString poiKey;
	QString poiKeyword;
	UINT32 poiFrame;
	LAT_LNG poipoint;
	double alt;
	UINT32 poiDirection;
	QString poiName;
	QString poiNameI18n;
	std::tuple<UINT64,LAT_LNG> m_calibrate_osm_path_info;
	tagPoiItem()
	{
		m_calibrate_osm_path_info = std::make_tuple(0, LAT_LNG{0,0});
	}
};

struct tagStandard_Taxiing_Path
{
	QString m_title;
	QString m_path;
	QString m_runway;
	QString m_parkingarea;
	QString m_pathdetail;
	QVector<QString> m_pathPoints;
	E_FLIGHT_DEP_ARR_TYPE m_flight_dep_arr_type;
	void analysis();
};

class Dijkstra;
struct tagPath_Plan
{
	QString m_runway;
	QString m_parkingpoint;
	E_FLIGHT_DEP_ARR_TYPE m_flight_dep_arr_type;
	QString m_airportcode;
	QString m_path;
    struct ptinfo{
        QString m_pt;
        bool bvalid;
        int index;
    };
    QVector<ptinfo> m_pathPoints;
	LATLNGS_VECTOR m_trackinglatlng;
	std::vector<std::tuple< UINT64,LAT_LNG>> m_tracking_osm_path_info;
	std::vector<std::tuple< UINT64, LAT_LNG>> m_tracking_osm_path_info_calibrate;

    LATLNGS_VECTOR m_extendwpslatlng;
	LATLNGS_VECTOR m_runwayextendwpslatlng;


    LATLNGS_VECTOR m_runway_total;


    LATLNGS_VECTOR m_extendwpslatlng_start_simple;
    LATLNGS_VECTOR m_extendwpslatlng_simple;
    LATLNGS_VECTOR m_runway_total_simple;

	QColor m_trackingcl;

	bool m_bValid;
	tagPath_Plan()
	{
		m_bValid = false;
	}

	void resize(int wpssize);
	void analysis(const std::unordered_map<QString, tagStandard_Taxiing_Path>& _Standard_Taxiing_Paths,
		const std::unordered_map<QString, tagPoiItem>& _poiitemsmap);

	QJsonObject toJson() const;
	void fromJson(const QJsonObject& jsonobj);


	QJsonObject outputgeojson();
};


Q_DECLARE_METATYPE(tagPath_Plan)

struct tagPathPlanInfo
{
	tagPath_Plan pathindex;
};

typedef std::unordered_map < QString, tagPathPlanInfo> RUNWAY_PATH;

typedef std::unordered_map < E_FLIGHT_DEP_ARR_TYPE, RUNWAY_PATH> ARR_DEP_RUNWAY_PATH;

struct PathPlanValidInfo
{
	enum E_PATH_TYPE
	{
		E_PATH_TYPE_UNKOWN = 0x00,
		E_PATH_TYPE_PLAN_PARKINGPOINT = 0x01,
		E_PATH_TYPE_PLAN_RUNWAY = 0x02,

		E_PATH_TYPE_CONFLICT_PARKINGPOINT = 0x04,
		E_PATH_TYPE_CONFLICT_RUNWAY = 0x08,

		E_PATH_TYPE_REALLOC_PARKINGPOINT = 0x10,
		E_PATH_TYPE_REALLOC_RUNWAY = 0x20,

		E_PATH_TYPE_REALLOC_CONFLICT_PARKINGPOINT = 0x40,
		E_PATH_TYPE_REALLOC_CONFLICT_RUNWAY = 0x80,
	};
	E_PATH_TYPE eStatus;

	QString target_parkingpoint;
	QString target_runway;
	QString alloc_parkingpoint;
	QString alloc_runway;
	PathPlanValidInfo()
	{
		eStatus = E_PATH_TYPE_UNKOWN;
	}
};

typedef std::tuple<tagPath_Plan*, FlightPlanConf*, PathPlanValidInfo> FLIGHTPLAN_PATH_INFO;



typedef std::function<void(tagPath_Plan *path_plan)> deal_pathplan_func_callback;

class PathPlanExtendProcessor : public QRunnable
{
public:
	PathPlanExtendProcessor(deal_pathplan_func_callback _pCallbackfunc, tagPath_Plan *path_plan);
	virtual ~PathPlanExtendProcessor();
	virtual void run() override;
private:
	deal_pathplan_func_callback m_pCallbackfunc;
	tagPath_Plan* m_path_plan;
};


class PathPlanExtendTaskManager :public QObject
{
	Q_OBJECT
public:
	PathPlanExtendTaskManager(QObject* parent = nullptr);
	virtual ~PathPlanExtendTaskManager();

	void appendProcessor(deal_pathplan_func_callback _pCallbackfunc, tagPath_Plan *path_plan);

	int getActiveThreadCount() const;

private:
	QThreadPool* m_threadPool;
};



typedef std::function<void(tagPath_Plan *)> deal_data_func_callback;


template<typename T>
class ThreadTaskProcessor
{
public:
	ThreadTaskProcessor(int id,
		deal_data_func_callback _pCallbackfunc,
		const T& param1)
		:m_id(id)
		, m_pCallbackfunc(std::move(_pCallbackfunc))
		, m_param1(std::move(param1))
	{
		//m_processorNum++;
	}
	virtual ~ThreadTaskProcessor() {}
	void operator()()
	{
		if (m_pCallbackfunc)
		{
			m_pCallbackfunc(m_param1);
		}
	}

private:
	int m_id;
	deal_data_func_callback m_pCallbackfunc;
	T m_param1;
};




namespace TaskThreadPool
{

	class ThreadPool {
	public:
		ThreadPool(size_t);
		template<class F, class... Args>
		auto enqueue(F&& f, Args&&... args)
			->std::future<typename std::result_of<F(Args...)>::type>;
		~ThreadPool();
	private:
		// need to keep track of threads so we can join them
		std::vector< std::thread > workers;
		// the task queue
		std::queue< std::function<void()> > tasks;

		// synchronization
		std::mutex queue_mutex;
		std::condition_variable condition;
		bool stop;
	};


#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

	// the constructor just launches some amount of workers
	inline ThreadPool::ThreadPool(size_t threads)
		: stop(false)
	{
		for (size_t i = 0; i < threads; ++i)
		{
			workers.emplace_back([this] {
#ifdef WIN32
				SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#else
				// �����߳����ȼ�Ϊ���
				struct sched_param params;
				params.sched_priority = sched_get_priority_max(SCHED_FIFO);
				pthread_setschedparam(pthread_self(), SCHED_FIFO, &params);
#endif
				for (;;)
				{
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						this->condition.wait(lock,
							[this] { return this->stop || !this->tasks.empty(); });
						if (this->stop && this->tasks.empty())
							return;
						task = std::move(this->tasks.front());
						this->tasks.pop();
					}

					task();
				}
			});
		}
	}

	// add new work item to the pool
	template<class F, class... Args>
	auto ThreadPool::enqueue(F&& f, Args&&... args)
		-> std::future<typename std::result_of<F(Args...)>::type>
	{
		using return_type = typename std::result_of<F(Args...)>::type;

		auto task = std::make_shared< std::packaged_task<return_type()> >(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			// don't allow enqueueing after stopping the pool
			if (stop)
				throw std::runtime_error("enqueue on stopped ThreadPool");

			tasks.emplace([task]() { (*task)(); });
		}
		condition.notify_one();
		return res;
	}

	// the destructor joins all threads
	inline ThreadPool::~ThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread &worker : workers)
		{
			worker.join();
		}
	}
}


#endif
