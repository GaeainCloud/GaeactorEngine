#include "eventdriver.h"
#include "src/OriginalThread.h"
#include "src/OriginalDateTime.h"

#include "loghelper.h"
#include "../configmanager.h"

#include "settingsconfig.h"

double EventDriver::get_timer_Freq2(UINT64 xtime, double speed)
{
    if(xtime == 0)
    {
        xtime = 100;
    }

    auto lerp = [](double a, double b, double t)->double {
        return a + t * (b - a);
    };

    double one_second_step = (xtime * 10);
    double xhz = one_second_step / (double)(xtime);
    if (speed <= MIN_SPEED_FREQ)
    {
        m_outputFreq = xhz * speed;
    }
    else if (speed >= MAX_SPEED_FREQ)
    {
        m_outputFreq = MAX_TIMER_FREQ;
    }
    else
    {
        // 计算插值比例
        double t = (speed - MIN_SPEED_FREQ) / (MAX_SPEED_FREQ - MIN_SPEED_FREQ);
        m_outputFreq = lerp(xhz, MAX_TIMER_FREQ, t);
    }

    m_step_interval = (one_second_step / m_outputFreq) / (xtime / 100.0);
    m_dt = 1 / m_outputFreq * speed * (xtime / 100.0);
    m_old_heartbeat_one_second_sim_step_second = m_heartbeat_one_second_sim_step_second;
    m_heartbeat_one_second_sim_step_second = m_dt * m_outputFreq;
    std::cout << "Input: " << speed << ", Output Frequency: " << m_outputFreq << " Hz " << m_step_interval << " ms " << " dt " << m_dt << " s " << m_heartbeat_one_second_sim_step_second << " s " << std::endl;

    ConfigManager::getInstance().step_interval = m_step_interval;
    ConfigManager::getInstance().step_dt = m_dt;
    ConfigManager::getInstance().step_freq = m_outputFreq;
    ConfigManager::getInstance().one_second_sim_step_second = m_heartbeat_one_second_sim_step_second;


    DataManager::getInstance().update_SiminfoLog_to_db(SiminfoLog{0,
                                                                  DataManager::getInstance().m_play_min,
                                                                  DataManager::getInstance().m_play_max,
                                                                  ConfigManager::getInstance().step_interval,
                                                                  ConfigManager::getInstance().step_dt,
                                                                  ConfigManager::getInstance().step_freq,
                                                                  ConfigManager::getInstance().one_second_sim_step_second,
                                                                  DataManager::getInstance().m_simname.toStdString(),
                                                                  stdutils::OriDateTime::getCurrentUTCTimeStampMSecs()});

    return m_step_interval;
}

void EventDriver::calc_heartbeat_second()
{
    UINT64 xtime = SettingsConfig::getInstance().base_step_timeinterval();
    this->get_timer_Freq2(xtime, m_speedCoeff);
}

uint64_t EventDriver::get_heartbeat_second()
{
	return m_heartbeat_one_second_sim_step_second;
}

double EventDriver::get_dt()
{
	return m_dt;
}

void timer_callback(uv_timer_t* handle)
{
	tagDealEvent* dealevent = reinterpret_cast<tagDealEvent*>(handle->data);
	if (dealevent)
	{
		//if (dealevent->m_pEventDriver->isExist())
		//{
		//	dealevent->m_pEventDriver->quit();
		//}		

		if (dealevent->m_pEventDriver &&
			((dealevent->m_pEventDriver->running_status() == E_RUNNING_STATUS_TYPE_START) ||
				(dealevent->m_pEventDriver->running_status() == E_RUNNING_STATUS_TYPE_RESUME)))
		{
			dealevent->m_pEventDriver->dealEventCallback(dealevent->m_eventdata);
		}

		uint64_t currentTimeStamp = uv_now(dealevent->m_pEventDriver->getEventLoop());
		uint64_t interval = fabs(currentTimeStamp - dealevent->per_timeout_peroid_start_time);
		//if (dealevent->m_eventdata.bEnableAdjustSpeed)
		//{
		//	std::cout << "-----Timer event callback " 
		//		<< "真实时间间隔:" << interval << " ms "
		//		<<"需要真实等待超时的时间: " << dealevent->cur_timeout <<" ms "

		//		<<"仿真时间间隔 " << interval * dealevent->m_pEventDriver->get_heartbeat_second() << " ms "
		//		<<"需要仿真等待超时的时间: " << dealevent->m_eventdata.timeout << " ms " << std::endl;
		//}
		dealevent->reset_period(currentTimeStamp);

		//		if (dealevent->m_eventdata.flightevent)
		//		{
		//			QDateTime dateTimestamp = QDateTime::fromSecsSinceEpoch(dealevent->m_eventdata.flightevent->m_itime);
		//            //std::cout << "-----Timer event callback " << dateTimestamp.toString("yyyy-MM-dd hh:mm:ss").toStdString() << " real:" << interval<<" except: "<< dealevent->m_eventdata.timeout << " speed coeff " << dealevent->m_pEventDriver->speed_coeff() << " real by speed:" << dealevent->cur_timeout << " ms " << std::endl;
		//		}
		//		else
		//		{
		//            //std::cout << "-----Timer event callback " << " real:" << interval << " except: " << dealevent->m_eventdata.timeout << " speed coeff " << dealevent->m_pEventDriver->speed_coeff() << " real by speed:" << dealevent->cur_timeout << " ms " << std::endl;
		//		}

		switch (dealevent->m_eventdata.e_update_type)
		{
		case E_EVENT_UPDATE_TYPE_ONCE:
		case E_EVENT_UPDATE_TYPE_ONCE_TRIGGER:
		{
			if (dealevent->m_pEventDriver)
			{
				dealevent->m_pEventDriver->do_close_timer_event(dealevent);
				dealevent->m_pEventDriver->dealEvent(dealevent->m_eventdata.flightevent, (dealevent->m_eventdata.e_update_type == E_EVENT_UPDATE_TYPE_ONCE_TRIGGER) ? true : false);
			}
		}break;

		case E_EVENT_UPDATE_TYPE_REPEAT_PERIOD:
		{
			//static int ms = 1;
			//if (dealevent->m_pEventDriver &&
			//	((dealevent->m_pEventDriver->running_status() == E_RUNNING_STATUS_TYPE_START) ||
			//		(dealevent->m_pEventDriver->running_status() == E_RUNNING_STATUS_TYPE_RESUME)))
			//{
			//	ms += dealevent->m_pEventDriver->get_heartbeat_second();
			//	std::cout << "-----Timer event callback " << ms << std::endl;
			//}
		}break;
		case E_EVENT_UPDATE_TYPE_REPEAT_TIMES:
		{
			if (dealevent->cur_repeattimes < dealevent->m_eventdata.repeattimes)
			{
				if (dealevent->m_pEventDriver)
				{
					dealevent->m_pEventDriver->start_event_timer(*dealevent, dealevent->cur_timeout);
				}
				dealevent->cur_repeattimes++;
			}
			else
            {
				dealevent->m_pEventDriver->do_close_timer_event(dealevent);
			}
		}break;
		default:
			break;
		}
	}
}


void timer_close_callback(uv_handle_t* handle)
{
	tagDealEvent* dealevent = reinterpret_cast<tagDealEvent*>(handle->data);
	if (dealevent)
	{
		if (dealevent->m_pEventDriver)
		{
			dealevent->m_pEventDriver->do_clear_timer_event(dealevent);
		}
	}
};
EventDriver::EventDriver(QObject* pParent /*= nullptr*/)
	:QObject(pParent),
	m_pRunningThread(nullptr),
	m_loop(nullptr),
	m_speedCoeff(1.0)
{
    m_end_event_id = FunctionAssistant::generate_random_positive_uint64();
	m_old_heartbeat_one_second_sim_step_second = 1;
	m_heartbeat_one_second_sim_step_second = 1;
	calc_heartbeat_second();
	m_bLoopRnning.store(false);
	m_bLoopExist.store(false);
	m_loop = uv_default_loop();
	m_pRunningThread = new stdutils::OriThread(std::bind(&EventDriver::thread_callback_Loop, this, std::placeholders::_1), this);
	m_pRunningThread->start();
}

EventDriver::~EventDriver()
{
	exit();
	if (m_pRunningThread)
	{
		delete m_pRunningThread;
		m_pRunningThread = nullptr;
	}
	uv_loop_close(m_loop);
}

void EventDriver::start()
{
	m_bLoopRnning.store(true);
}

void EventDriver::stop()
{
	auto itor = m_events.begin();
	while (itor != m_events.end())
    {
		if (itor->second.m_event_status != EVENT_ITEM_STATUS_UNRUNING)
		{
			if (itor->second.m_event_status != EVENT_ITEM_STATUS_CLOSE)
			{
				do_close_timer_event(&itor->second);
			}
		}
		else
		{
			itor = m_events.erase(itor);
			continue;
		}

		itor++;
	}
    //m_events.clear();
	m_bLoopRnning.store(false);
	quit();
}

float EventDriver::speed_coeff()
{
	return m_speedCoeff;
}

void EventDriver::event_pause()
{
	auto itor = m_events.begin();
	while (itor != m_events.end())
	{
		tagDealEvent& dealevent = itor->second;
		if (dealevent.m_eventdata.bEnablePause &&
            dealevent.m_event_status == EVENT_ITEM_STATUS_RUNNING)
		{
			add_event_timer_to_stop(&dealevent);

			updatePauseTimeout(dealevent);

			dealevent.m_event_status = EVENT_ITEM_STATUS_PAUSE;

			start_event_timer(dealevent, PAUSE_TIME_OUT_MS);
		}
		itor++;
	}

	DataManager::getInstance().m_play_pause = DataManager::getInstance().m_play_cur;
	std::stringstream ss;
	ss << "---------------------------------------暂停仿真---------------------------------------";
	DataManager::getInstance().trans_log("操作： ", ss, std::stringstream());
}


void EventDriver::event_resume()
{
	//当前时间戳
	auto itor = m_events.begin();
	while (itor != m_events.end())
	{
		tagDealEvent& dealevent = itor->second;
		if (dealevent.m_eventdata.bEnablePause &&
            dealevent.m_event_status == EVENT_ITEM_STATUS_PAUSE)
		{
			add_event_timer_to_stop(&dealevent);

			//根据当前时间戳 以及 事件已经等待的超时时间 ，计算一个该事件超时周期内的起始时间戳
//			dealevent.per_timeout_peroid_start_time = pause_end - dealevent.trigger_pause_already_wait_timeout_sim;
			dealevent.per_timeout_peroid_start_time = uv_now(m_loop);
			dealevent.m_event_status = EVENT_ITEM_STATUS_RUNNING;

			start_event_timer(dealevent, dealevent.trigger_resume_need_wait_timeout_sim);
					
		}
		itor++;
	}

	std::stringstream ss;
	ss << "---------------------------------------继续仿真--------------------------------------- ";
	DataManager::getInstance().trans_log("操作： ", ss, std::stringstream());
}

void EventDriver::event_start()
{
    {
        auto _events_itor = std::find_if(m_events.begin(),
                                         m_events.end(), [&](const std::unordered_map<uint64_t, tagDealEvent>::value_type& vt) {
                                             return vt.first == m_end_event_id;
                                         });
        if (_events_itor != m_events.end())
        {
            tagDealEvent& dealevent = m_events[m_end_event_id];
            add_event_timer_to_stop(&dealevent);
        }
    }
	auto start_event_trigger = [&](tagFlightEventTime* pflightevent)
	{
		if (pflightevent)
		{
			tagEventInfo eventinfo;
			eventinfo.e_update_type = E_EVENT_UPDATE_TYPE_ONCE_TRIGGER;
			eventinfo.m_eventId = pflightevent->m_eventid;
			eventinfo.timeout = 100;// pflightevent->m_day_senscod_offset_ms;
			eventinfo.repeattimes = 0;
			eventinfo.m_eventtype = E_EVENT_TYPE_ID_RUNTIME;
			eventinfo.flightevent = pflightevent;
			eventinfo.bEnableAdjustSpeed = true;
			this->addevent(eventinfo);
		}
	};


	std::map<uint64_t, tagFlightEventTime>& day_flight_events = DataManager::getInstance().total_flightEventTimedata;
	auto ff = day_flight_events.begin();
	if (ff != day_flight_events.end())
	{
		DataManager::getInstance().m_play_min = ff->first;
	}
	auto rff = day_flight_events.rbegin();
	if (rff != day_flight_events.rend())
	{
		DataManager::getInstance().m_play_max = rff->first;
	}

	tagFlightEventTime* pflightevent = DataManager::getInstance().find_target_event(DataManager::getInstance().m_play_min);

	if (pflightevent)
	{
		DataManager::getInstance().m_play_range = DataManager::getInstance().m_play_max - DataManager::getInstance().m_play_min;
		DataManager::getInstance().m_play_cur_d = 0.0;
		DataManager::getInstance().m_play_cur = DataManager::getInstance().m_play_min;

		start_event_trigger(pflightevent);

		std::stringstream ss;
		ss << "---------------------------------------开始仿真---------------------------------------";
		DataManager::getInstance().trans_log("操作： ", ss, std::stringstream());
	}
}

void EventDriver::event_stop()
{
	auto itor = m_events.begin();
	while (itor != m_events.end())
	{
		tagDealEvent& dealevent = itor->second;
        if (dealevent.m_eventdata.bEnablePause)
		{
			if (itor->second.m_event_status != EVENT_ITEM_STATUS_UNRUNING)
			{
				if (itor->second.m_event_status != EVENT_ITEM_STATUS_CLOSE)
				{
					do_close_timer_event(&itor->second);
				}
			}
			else
			{
				itor = m_events.erase(itor);
				continue;
			}
		}
		itor++;
	}
	std::stringstream ss;
	ss << "---------------------------------------终止仿真---------------------------------------";
	DataManager::getInstance().trans_log("操作： ", ss, std::stringstream());
}



void EventDriver::updatePauseTimeout(tagDealEvent& dealevent)
{
	UINT64 currentTimeStamp = uv_now(m_loop);
	UINT64 _heartbeat_one_second_sim_step_second = 1;
	if (dealevent.m_eventdata.bEnableAdjustSpeed)
	{
		_heartbeat_one_second_sim_step_second = m_heartbeat_one_second_sim_step_second;
	}

	UINT64 sim_plan_wait_timeout = fabs(currentTimeStamp - dealevent.per_timeout_peroid_start_time) * _heartbeat_one_second_sim_step_second;

	dealevent.updatePauseTimeout(sim_plan_wait_timeout, _heartbeat_one_second_sim_step_second);
}


void EventDriver::updateTimeout(tagDealEvent& dealevent, UINT64 new_timeout)
{
	//调整之后的 定时时间间隔
	dealevent.cur_timeout = new_timeout <= MIN_TIMEOUT ? MIN_TIMEOUT : new_timeout;

	if (dealevent.m_event_status == EVENT_ITEM_STATUS_PAUSE)
	{
		UINT64 _heartbeat_one_second_sim_step_second = 1;
		if (dealevent.m_eventdata.bEnableAdjustSpeed)
		{
			_heartbeat_one_second_sim_step_second = m_heartbeat_one_second_sim_step_second;
		}

		dealevent.updatePauseTimeout_spd(_heartbeat_one_second_sim_step_second);
	}
}


void EventDriver::add_event_timer_to_stop(tagDealEvent* event)
{
	QWriteLocker locker(&m_stop_timer_queue_mutex);

	if (m_stop_timer_queue.find(event) != m_stop_timer_queue.end())
	{
		m_stop_timer_queue.at(event) = true;
	}
	else
	{
		m_stop_timer_queue.insert(std::make_pair(event, true));
	}
}

void EventDriver::deal_event_timer_list_to_stop()
{
	QWriteLocker locker(&m_stop_timer_queue_mutex);
	auto itor = m_stop_timer_queue.begin();
	while (itor != m_stop_timer_queue.end())
	{
		tagDealEvent& dealevent = *(itor->first);
		uv_timer_stop(&dealevent.m_timer);
		itor++;
	}
	m_stop_timer_queue.clear();
}

void EventDriver::add_event_timer_to_close(tagDealEvent* event)
{
	QWriteLocker locker(&m_close_timer_queue_mutex);

	if (m_close_timer_queue.find(event) != m_close_timer_queue.end())
	{
		m_close_timer_queue.at(event) = true;
	}
	else
	{
		m_close_timer_queue.insert(std::make_pair(event, true));
	}
}

void EventDriver::deal_event_timer_list_to_close()
{
	QWriteLocker locker(&m_close_timer_queue_mutex);
	auto itor = m_close_timer_queue.begin();
	while (itor != m_close_timer_queue.end())
	{
		tagDealEvent& dealevent = *(itor->first);
		uv_close((uv_handle_t*)&dealevent.m_timer, timer_close_callback);
		itor++;
	}
	m_close_timer_queue.clear();
}

void EventDriver::add_event_timer_to_start(tagDealEvent* event, const std::tuple<uint64_t, uint64_t>& timeout)
{
	QWriteLocker locker(&m_start_timer_queue_mutex);

	if (m_start_timer_queue.find(event) != m_start_timer_queue.end())
	{
		m_start_timer_queue.at(event) = timeout;
	}
	else
	{
		m_start_timer_queue.insert(std::make_pair(event, timeout));
	}
}

void EventDriver::deal_event_timer_list_to_start()
{
	QWriteLocker locker(&m_start_timer_queue_mutex);
	auto itor = m_start_timer_queue.begin();
	while (itor != m_start_timer_queue.end())
	{
		tagDealEvent& dealevent = *(itor->first);
		uv_timer_start(&dealevent.m_timer, timer_callback, std::get<0>(itor->second), std::get<1>(itor->second)); // 注册定时器事件处理函数
		itor++;
	}
	m_start_timer_queue.clear();
}

void EventDriver::restart_event_timer(tagDealEvent& dealevent, UINT32 newtimeout)
{
	add_event_timer_to_stop(&dealevent);
	UINT64 currentTimeStamp = uv_now(m_loop);
	//此时是在旧的倍速下获取的时间间隔
	UINT64 _heartbeat_one_second_sim_step_second = 1;
	if (dealevent.m_eventdata.bEnableAdjustSpeed)
	{
		_heartbeat_one_second_sim_step_second = m_heartbeat_one_second_sim_step_second;
	}
	//需要应用旧的倍速得到 已经仿真的时间
	UINT64 sim_plan_wait_timeout = fabs(currentTimeStamp - dealevent.per_timeout_peroid_start_time) * m_old_heartbeat_one_second_sim_step_second;

	dealevent.updatePauseTimeout(sim_plan_wait_timeout, _heartbeat_one_second_sim_step_second);

	dealevent.per_timeout_peroid_start_time = currentTimeStamp;
	dealevent.m_event_status = EVENT_ITEM_STATUS_RUNNING;
	start_event_timer(dealevent, dealevent.trigger_resume_need_wait_timeout_sim);
}

void EventDriver::do_close_timer_event(tagDealEvent* dealevent)
{
//	std::cout << "-----libuv close Timer event trigger " << dealevent->m_eventdata.m_eventId << std::endl;
	dealevent->m_event_status = EVENT_ITEM_STATUS_CLOSE;
	//    uv_close((uv_handle_t*)&dealevent->m_timer,dealevent->m_timer.close_cb);
	//    uv_close((uv_handle_t*)&dealevent->m_timer,timer_close_callback);

	add_event_timer_to_close(dealevent);
}

bool EventDriver::do_clear_timer_event(tagDealEvent* dealevent)
{
	return clearevent(dealevent->m_eventdata.m_eventId);
}


void EventDriver::start_event_timer(tagDealEvent& dealevent, uint64_t timeout)
{
	UINT64 repeat_timeout = 0;
	switch (dealevent.m_eventdata.e_update_type)
	{
	case E_EVENT_UPDATE_TYPE_ONCE:
	case E_EVENT_UPDATE_TYPE_ONCE_TRIGGER:
	case E_EVENT_UPDATE_TYPE_REPEAT_TIMES:
	{
		repeat_timeout = (timeout == PAUSE_TIME_OUT_MS) ? PAUSE_TIME_OUT_MS : 0;
	}break;
	case E_EVENT_UPDATE_TYPE_REPEAT_PERIOD:
	{
		repeat_timeout = (timeout == PAUSE_TIME_OUT_MS) ? PAUSE_TIME_OUT_MS : dealevent.cur_timeout;
	}break;
	default:
		break;
	}
//	uv_timer_start(&dealevent.m_timer, timer_callback, timeout, repeat_timeout); // 注册定时器事件处理函数
	add_event_timer_to_start(&dealevent, std::make_tuple(timeout, repeat_timeout));
}

E_RUNNING_STATUS_TYPE EventDriver::running_status() const
{
	return m_running_status;
}

void EventDriver::setRunning_status(E_RUNNING_STATUS_TYPE newRunning_status,bool bSend)
{
	switch (newRunning_status) {
	case E_RUNNING_STATUS_TYPE_START:
	{
        if(bSend)
        {
            QJsonObject jsobj;
            jsobj.insert("speed", 0.0);
            jsobj.insert("ctrlparam", "start");
            jsobj.insert("ctrltype", "processctrl");
            if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->execute_running_speed(jsobj))
            {
                std::cout << "http error" << std::endl;
            }
        }
		event_start();
	}break;
	case E_RUNNING_STATUS_TYPE_PAUSE:
	{
        if(bSend)
        {
            QJsonObject jsobj;
            jsobj.insert("speed", 0.0);
            jsobj.insert("ctrlparam", "pause");
            jsobj.insert("ctrltype", "processctrl");
            if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->execute_running_speed(jsobj))
            {
                std::cout << "http error" << std::endl;
            }
        }
		if (m_running_status == E_RUNNING_STATUS_TYPE_START ||
			m_running_status == E_RUNNING_STATUS_TYPE_RESUME)
		{
			event_pause();
		}
	}break;

	case E_RUNNING_STATUS_TYPE_RESUME:
	{
        if(bSend)
        {
            QJsonObject jsobj;
            jsobj.insert("speed", 0.0);
            jsobj.insert("ctrlparam", "resume");
            jsobj.insert("ctrltype", "processctrl");
            if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->execute_running_speed(jsobj))
            {
                std::cout << "http error" << std::endl;
            }
        }
		if (m_running_status == E_RUNNING_STATUS_TYPE_PAUSE)
		{
			event_resume();
		}
	}break;

	case E_RUNNING_STATUS_TYPE_STOP:
	{
        if(bSend)
        {
            QJsonObject jsobj;
            jsobj.insert("speed", 0.0);
            jsobj.insert("ctrlparam", "stop");
            jsobj.insert("ctrltype", "processctrl");
            if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->execute_running_speed(jsobj))
            {
                std::cout << "http error" << std::endl;
            }
        }
		event_stop();
	}break;

	default:
		break;
	}
	m_running_status = newRunning_status;
}


void EventDriver::set_speed_coeff(float speedCoeff)
{
	m_speedCoeff = speedCoeff;

	QJsonObject jsobj;
	jsobj.insert("speed", m_speedCoeff);
    jsobj.insert("ctrlparam", "prgctrl");
    jsobj.insert("ctrltype", "prgctrl");
	if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->execute_running_speed(jsobj))
	{
		std::cout << "http error" << std::endl;
	}

	calc_heartbeat_second();
	std::cout << "cur speed coeff " << m_speedCoeff << std::endl;

	auto itor = m_events.begin();
	while (itor != m_events.end())
	{
		tagDealEvent& dealevent = itor->second;

		if (dealevent.m_eventdata.bEnableAdjustSpeed)
		{
			UINT64 new_timeout = dealevent.m_eventdata.timeout / speedCoeff;
			if (dealevent.m_eventdata.bEnableAdjustFreq)
			{
				//				new_timeout = get_timer_Freq2(dealevent.m_eventdata.timeout, speedCoeff);
				//根据物理1s  推进仿真的秒数 计算出 需要多少物理1s 来 延迟这个timeout
				double real_need_timeout_ms = dealevent.m_eventdata.timeout / m_heartbeat_one_second_sim_step_second;
				new_timeout = real_need_timeout_ms;
			}
			if (new_timeout != dealevent.cur_timeout)
			{
				updateTimeout(dealevent, new_timeout);
				if (dealevent.m_event_status == EVENT_ITEM_STATUS_RUNNING)
				{
					restart_event_timer(dealevent, new_timeout);
				}
			}
		}
		itor++;
	}
}

void EventDriver::exit()
{
	m_bLoopExist.store(true);
	stop();
}

bool EventDriver::isExist()
{
	return m_bLoopExist.load();
}

bool EventDriver::addevent(const tagEventInfo& eventinfo)
{
	if (m_loop)
	{
		auto _events_itor = std::find_if(m_events.begin(),
			m_events.end(), [&](const std::unordered_map<uint64_t, tagDealEvent>::value_type& vt) {
			return vt.first == eventinfo.m_eventId;
		});
		if (_events_itor == m_events.end())
		{
			tagDealEvent dealeventinfo;
			dealeventinfo.m_eventdata = eventinfo;
			dealeventinfo.m_pEventDriver = this;

			dealeventinfo.cur_repeattimes = 0;
			if (dealeventinfo.m_eventdata.bEnableAdjustSpeed)
			{
				updateTimeout(dealeventinfo, dealeventinfo.m_eventdata.timeout / m_speedCoeff);
				if (dealeventinfo.m_eventdata.bEnableAdjustFreq)
				{
					//根据物理1s  推进仿真的秒数 计算出 需要多少物理1s 来 延迟这个timeout
					double real_need_timeout_ms = dealeventinfo.m_eventdata.timeout / m_heartbeat_one_second_sim_step_second;
					updateTimeout(dealeventinfo, real_need_timeout_ms);
				}
			}
			else
			{
				updateTimeout(dealeventinfo, dealeventinfo.m_eventdata.timeout);
			}
			dealeventinfo.reset_period(uv_now(m_loop));
			dealeventinfo.m_event_status = EVENT_ITEM_STATUS_RUNNING;

			m_events.insert(std::make_pair(eventinfo.m_eventId, std::move(dealeventinfo)));
			tagDealEvent& dealevent = m_events[eventinfo.m_eventId];
			dealevent.m_timer.data = &(dealevent);
			uv_timer_init(m_loop, &dealevent.m_timer);

			dealevent.trigger_resume_need_wait_timeout_sim = dealevent.cur_timeout;
			start_event_timer(dealevent, dealevent.trigger_resume_need_wait_timeout_sim);
			return true;
		}
	}
	return false;
}

bool EventDriver::removeevent(const tagEventInfo& eventinfo)
{
	auto _events_itor = std::find_if(m_events.begin(),
		m_events.end(), [&](const std::unordered_map<uint64_t, tagDealEvent>::value_type& vt) {
		return vt.first == eventinfo.m_eventId;
	});
	if (_events_itor != m_events.end())
	{
		tagDealEvent& dealevent = m_events[eventinfo.m_eventId];
		if (dealevent.m_event_status != EVENT_ITEM_STATUS_UNRUNING)
		{
			if (dealevent.m_event_status != EVENT_ITEM_STATUS_CLOSE)
			{
				do_close_timer_event(&dealevent);
			}
		}
		else
		{
			m_events.erase(_events_itor);
		}
		return true;
	}
	return false;
}

bool EventDriver::dealEvent(tagFlightEventTime* flightevent, bool bTrigger)
{
	if (flightevent)
	{
		emit trigger_event_to_deal_sig(flightevent->m_itime, flightevent->m_eventid);
		if (bTrigger)
		{
			triggerNextEvent(flightevent);
		}
		return true;
	}
	return false;
}

bool EventDriver::dealEventCallback(const tagEventInfo& m_eventdata)
{
	if (m_event_callback)
	{
		m_event_callback(m_eventdata.m_eventId, m_eventdata.m_eventtype);
	}
	return true;
}

bool EventDriver::clearevent(const uint64_t& eventid)
{
	auto _events_itor = std::find_if(m_events.begin(),
		m_events.end(), [&](const std::unordered_map<uint64_t, tagDealEvent>::value_type& vt) {
		return vt.first == eventid;
	});
	if (_events_itor != m_events.end())
	{
		m_events.erase(_events_itor);
		return true;
	}
	return false;
}

void EventDriver::quit()
{
	if (uv_loop_alive(m_loop))
	{
		std::cout << "uv event loop stoping" << std::endl;
		uv_stop(m_loop);
	}
}

uv_loop_t* EventDriver::getEventLoop()
{
	return m_loop;
}

void EventDriver::set_event_callback(event_callback newEvent_callback)
{
	m_event_callback = std::move(newEvent_callback);
}

void EventDriver::thread_callback_Loop(void* param)
{
	deal_event_timer_list_to_stop();
	deal_event_timer_list_to_start();
	deal_event_timer_list_to_close();
	if (m_bLoopRnning.load())
	{
		//        std::cout << "uv event loop running" << std::endl;
		uv_run(m_loop, UV_RUN_NOWAIT);
	}
	else
	{
		stdutils::OriDateTime::sleep(1);
	}
	//    std::cout << "uv event loop running exit" << std::endl;

	if (m_bLoopExist.load())
	{
		return;
	}
	stdutils::OriDateTime::sleep(1);
}

bool EventDriver::triggerNextEvent(tagFlightEventTime* flightevent)
{
	if (flightevent->m_next_)
	{
		tagFlightEventTime* _next_ = flightevent->m_next_;
		tagEventInfo eventinfo;
		eventinfo.e_update_type = E_EVENT_UPDATE_TYPE_ONCE_TRIGGER;
		eventinfo.m_eventId = _next_->m_eventid;
		eventinfo.timeout = flightevent->m_next_with_offset_ms;
		eventinfo.repeattimes = 0;
		eventinfo.flightevent = _next_;
		eventinfo.bEnableAdjustSpeed = true;
		this->addevent(eventinfo);
		return true;
	}
    else
    {
        //增加结束 5 s
        tagEventInfo timereventinfo;
        timereventinfo.e_update_type = E_EVENT_UPDATE_TYPE_ONCE;
        timereventinfo.m_eventId = m_end_event_id;
        timereventinfo.timeout = 5 * 1000;// pflightevent->m_day_senscod_offset_ms;
        timereventinfo.repeattimes = 0;
        timereventinfo.m_eventtype = E_EVENT_TYPE_ID_SIM_END;
        timereventinfo.flightevent = nullptr;
        timereventinfo.bEnableAdjustSpeed = false;
        this->addevent(timereventinfo);
//        emit trigger_event_end_sig();
    }
	return false;
}
