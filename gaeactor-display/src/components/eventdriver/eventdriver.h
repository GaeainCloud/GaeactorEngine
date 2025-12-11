#ifndef EVENTDRIVER_H
#define EVENTDRIVER_H
#include <QObject>
#include <uv.h>
#include "../../datamanager/datamanager.hpp"

namespace stdutils
{
	class OriThread;
};

#define PAUSE_TIME_OUT_MS   (std::numeric_limits<uint64_t>::max())

enum E_EVENT_UPDATE_TYPE
{
	E_EVENT_UPDATE_TYPE_ONCE,
	E_EVENT_UPDATE_TYPE_ONCE_TRIGGER,
	E_EVENT_UPDATE_TYPE_REPEAT_PERIOD,
	E_EVENT_UPDATE_TYPE_REPEAT_TIMES
};

enum E_EVENT_TYPE_ID :UINT32
{
	E_EVENT_TYPE_ID_UNKOWN = 0x00,
	E_EVENT_TYPE_ID_HEARTBEAT,
	E_EVENT_TYPE_ID_SIM_TRACKING,
	E_EVENT_TYPE_ID_RUNTIME,
	E_EVENT_TYPE_ID_PARKINGPOINT_RELEASE,
    E_EVENT_TYPE_ID_RUNWAY_RELEASE,
    E_EVENT_TYPE_ID_SIM_END
};

struct tagEventInfo
{
	tagFlightEventTime* flightevent;
	E_EVENT_TYPE_ID m_eventtype;
	uint64_t m_eventId;
	uint64_t timeout;
	bool bEnableAdjustFreq;
    bool bEnableAdjustSpeed;
    bool bEnablePause;
    E_EVENT_UPDATE_TYPE e_update_type;
	uint64_t repeattimes;
	tagEventInfo()
		:flightevent(nullptr),
		m_eventtype(E_EVENT_TYPE_ID_UNKOWN),
		m_eventId(0),
		timeout(0),
		bEnableAdjustFreq(true),
        bEnableAdjustSpeed(false),
        bEnablePause(true),
        e_update_type(E_EVENT_UPDATE_TYPE_ONCE),
		repeattimes(0)
	{
	}
};
class EventDriver;
enum EVENT_ITEM_STATUS
{
    EVENT_ITEM_STATUS_UNRUNING,
    EVENT_ITEM_STATUS_RUNNING,
    EVENT_ITEM_STATUS_PAUSE,
	EVENT_ITEM_STATUS_CLOSE
};

#define MIN_TIMEOUT (10)

struct tagDealEvent
{
	tagEventInfo m_eventdata;
	uv_timer_t m_timer;

    uint64_t per_timeout_peroid_start_time;
	uint64_t cur_timeout;
	uint64_t cur_repeattimes;


    uint64_t trigger_pause_already_wait_timeout_sim_no_speed;
    uint64_t trigger_resume_need_wait_timeout_sim_no_speed;
    uint64_t trigger_pause_already_wait_timeout_sim;
    uint64_t trigger_resume_need_wait_timeout_sim;
    EventDriver* m_pEventDriver;

    EVENT_ITEM_STATUS m_event_status;
    tagDealEvent()
        :
		per_timeout_peroid_start_time(0),
		cur_timeout(0),
		cur_repeattimes(0),
		trigger_pause_already_wait_timeout_sim_no_speed(0),
		trigger_resume_need_wait_timeout_sim_no_speed(0),
		trigger_pause_already_wait_timeout_sim(0),
		trigger_resume_need_wait_timeout_sim(0),
		m_pEventDriver(nullptr),
        m_event_status(EVENT_ITEM_STATUS_UNRUNING)
    {

    }
	void reset_period(const TIMESTAMP_TYPE& currentTimeStamp)
	{
		this->per_timeout_peroid_start_time = currentTimeStamp;
		this->trigger_pause_already_wait_timeout_sim_no_speed = 0;
	}

    void updatePauseTimeout(UINT64 sim_plan_wait_timeout, UINT64 _heartbeat_one_second_sim_step_second)
    {
		//计算在当前仿真的速度下 物理发生的描述 推演了多少 仿真的秒数
		//获取事件已经等待的超时时间，该次事件等待超时启动时间戳 - 当前时间戳 [仿真时间]
		trigger_pause_already_wait_timeout_sim_no_speed += sim_plan_wait_timeout;

		trigger_pause_already_wait_timeout_sim_no_speed = trigger_pause_already_wait_timeout_sim_no_speed < m_eventdata.timeout ? trigger_pause_already_wait_timeout_sim_no_speed : m_eventdata.timeout;

		//获取事件还需要等待的超时时间，超时时间-已经等待的时间   [仿真时间]
		trigger_resume_need_wait_timeout_sim_no_speed = fabs(m_eventdata.timeout - trigger_pause_already_wait_timeout_sim_no_speed);

		trigger_pause_already_wait_timeout_sim = trigger_pause_already_wait_timeout_sim_no_speed / _heartbeat_one_second_sim_step_second;
		trigger_resume_need_wait_timeout_sim = trigger_resume_need_wait_timeout_sim_no_speed / _heartbeat_one_second_sim_step_second;
		/*
		std::stringstream ss;
		ss << "---------等待--------- 仿真时间已经等待 " << trigger_pause_already_wait_timeout_sim_no_speed << " 真实时间已经等待 " << trigger_pause_already_wait_timeout_sim << " 仿真时间需要等待 " << trigger_resume_need_wait_timeout_sim_no_speed << " 真实时间需要等待 " << trigger_resume_need_wait_timeout_sim;
		std::cout << ss.str() << std::endl;
		*/
    }

	void updatePauseTimeout_spd(UINT64 _heartbeat_one_second_sim_step_second)
	{
		trigger_pause_already_wait_timeout_sim = trigger_pause_already_wait_timeout_sim_no_speed / _heartbeat_one_second_sim_step_second;
		trigger_resume_need_wait_timeout_sim = (trigger_resume_need_wait_timeout_sim_no_speed / _heartbeat_one_second_sim_step_second) ;
		/*
		std::stringstream ss;
		ss << "---------等待--------- 仿真时间已经等待 " << trigger_pause_already_wait_timeout_sim_no_speed << " 真实时间已经等待 " << trigger_pause_already_wait_timeout_sim << " 仿真时间需要等待 " << trigger_resume_need_wait_timeout_sim_no_speed << " 真实时间需要等待 " << trigger_resume_need_wait_timeout_sim;
		std::cout << ss.str() << std::endl;
		*/
	}
};

typedef std::function<void(const UINT64&, const E_EVENT_TYPE_ID&)> event_callback;

enum E_RUNNING_STATUS_TYPE
{
    E_RUNNING_STATUS_TYPE_FREE,
    E_RUNNING_STATUS_TYPE_START,
    E_RUNNING_STATUS_TYPE_PAUSE,
    E_RUNNING_STATUS_TYPE_RESUME,
    E_RUNNING_STATUS_TYPE_STOP

};
//struct uv_loop_t;
class EventDriver : public QObject
{
	Q_OBJECT
public:
	EventDriver(QObject *pParent = nullptr);
	~EventDriver();
	void start();
	void stop();
	float speed_coeff();
	void set_speed_coeff(float speedCoeff);
	void exit();
	bool isExist();
	bool addevent(const tagEventInfo& eventinfo);
	bool removeevent(const tagEventInfo& eventinfo);

	bool dealEvent(tagFlightEventTime* flightevent, bool bTrigger = false);
	bool dealEventCallback(const tagEventInfo &m_eventdata);
	bool clearevent(const uint64_t & eventid);
	void quit();
	uv_loop_t* getEventLoop();

	void set_event_callback(event_callback newEvent_callback);
    double get_timer_Freq2(UINT64 xtime, double speed);

	void calc_heartbeat_second();
    uint64_t get_heartbeat_second();
    double get_dt();
    E_RUNNING_STATUS_TYPE running_status() const;
    void setRunning_status(E_RUNNING_STATUS_TYPE newRunning_status, bool bSend = true);

	void event_pause();
	void event_resume();
	void event_start();
	void event_stop();

    void start_event_timer(tagDealEvent &dealevent, uint64_t timeout);

	void do_close_timer_event(tagDealEvent* dealevent);
	bool do_clear_timer_event(tagDealEvent* dealevent);
signals:
	void trigger_event_to_deal_sig(uint64_t triggertimestamp, uint64_t trigger_event_id);

    void trigger_event_end_sig();
private:
	void thread_callback_Loop(void* param);

	bool triggerNextEvent(tagFlightEventTime* flightevent);
    void restart_event_timer(tagDealEvent &dealevent, UINT32 newtimeout);

    void updatePauseTimeout(tagDealEvent &dealevent);
    void updateTimeout(tagDealEvent &dealevent, UINT64 new_timeout);


	void add_event_timer_to_stop(tagDealEvent* event);
	void deal_event_timer_list_to_stop();

	void add_event_timer_to_close(tagDealEvent* event);
	void deal_event_timer_list_to_close();

	void add_event_timer_to_start(tagDealEvent* event, const std::tuple<uint64_t, uint64_t>& timeout);
	void deal_event_timer_list_to_start();

private:
	stdutils::OriThread *m_pRunningThread;
	uv_loop_t* m_loop;
	//	uv_timer_t m_timer;

	std::unordered_map<uint64_t, tagDealEvent> m_events;

	std::atomic<bool> m_bLoopRnning;
	std::atomic<bool> m_bLoopExist;
	float m_speedCoeff;


	event_callback m_event_callback;

    //物理1s  推进仿真的秒数
	uint64_t m_old_heartbeat_one_second_sim_step_second;
	uint64_t m_heartbeat_one_second_sim_step_second;
	double m_dt;

    uint64_t m_step_interval;
    double m_outputFreq; // 输出频率
    E_RUNNING_STATUS_TYPE m_running_status;


	QReadWriteLock m_start_timer_queue_mutex;
	std::unordered_map<tagDealEvent*, std::tuple<uint64_t, uint64_t>> m_start_timer_queue;

	QReadWriteLock m_stop_timer_queue_mutex;
	std::unordered_map<tagDealEvent*, bool> m_stop_timer_queue;

	QReadWriteLock m_close_timer_queue_mutex;
	std::unordered_map<tagDealEvent*, bool> m_close_timer_queue;

    uint64_t m_end_event_id;
};
#endif // EVENTDRIVER_H
