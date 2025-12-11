/****************************************************************************
**
** Copyright (C) 2015-2016 Dinu SV.
** (contact: mail@dinusv.com)
** This file is part of QML Gantt library.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
****************************************************************************/
#pragma execution_character_set("utf-8")
#include "ganttwidget.h"
#include <QDateTime>
#include <ActiveQt/QAxObject>
#include <ActiveQt/QAxWidget>
#include <ActiveQt/QAxBase>
#include <QDebug>
#include <unordered_map>
#include <QQmlContext>
#include <QWheelEvent>

#include <QtXlsx/QtXlsx>

#include "qrangeview.h"
#include "qganttmodel.h"
#include "qganttmodelitem.h"

#include "qganttmodellist.h"
#include "qganttdata.h"
#include "components/function.h"

#include "../components/global_variables.h"
#include "loghelper.h"
#define SCALE_DIFF (10)
static int CONFIGURATION_MODEL_SIZE = 60 * 60 * 24*3 / SCALE_DIFF;


int GanttWidget::randBetween(int min, int max) {
	return rand() % (max - min + 1) + min;
}

QVariant GanttWidget::createModelData() {
    return QVariant::fromValue(new QGanttData);
}

void GanttWidget::setAirportInfos(const QString &airport_code, const QStringList &allowRunway)
{
    m_airport_code = airport_code;
    m_allowRunway = allowRunway;
}

void GanttWidget::keyPressEvent(QKeyEvent* event)
{
	if ((event->key() == Qt::Key_Plus))
	{
		m_zoomscale += 0.01;
		if (m_zoomscale > 1.0)
		{
			m_zoomscale = 1.0;
		}
	}
	if ((event->key() == Qt::Key_Minus))
	{
		m_zoomscale -= 0.01;
		if (m_zoomscale < 0.1)
		{
			m_zoomscale = 0.1;
		}

	}
	QObject* pRoot = (QObject*)m_qmlpanelWidget->rootObject();
	QMetaObject::invokeMethod(pRoot, "updateZoom",
		Q_ARG(QVariant, QVariant::fromValue(m_zoomscale)));

	//m_modelList->setContentWidth(CONFIGURATION_MODEL_SIZE*m_zoomscale);
	QWidget::keyPressEvent(event);
}

void GanttWidget::keyReleaseEvent(QKeyEvent* event)
{
	QWidget::keyReleaseEvent(event);
}

void GanttWidget::wheelEvent(QWheelEvent *event)
{
	QWidget::wheelEvent(event);
}

void GanttWidget::resizeEvent(QResizeEvent *event)
{
	m_qmlpanelWidget->setGeometry(this->geometry());
	m_qmlpanelWidget->move(QPoint(0, 0));
	QWidget::resizeEvent(event);
}

void GanttWidget::showEvent(QShowEvent *event)
{
	m_qmlpanelWidget->setVisible(true);
	QWidget::showEvent(event);
}

void GanttWidget::hideEvent(QHideEvent *event)
{
	m_qmlpanelWidget->setVisible(false);
	QWidget::hideEvent(event);
}


GanttWidget::GanttWidget(QWidget *parent)
	:QWidget(parent),
	m_qmlpanelWidget(nullptr),
	m_zoomscale(1.0f)
{
	qmlRegisterUncreatableType<QAbstractRangeModel>("Gantt", 1, 0, "AbstractRangeModel", "AbstractRangeModel is of abstract type.");
	qmlRegisterType<QRangeView>("Gantt", 1, 0, "RangeView");
	qmlRegisterType<QGanttModel>("Gantt", 1, 0, "GanttModel");
	qmlRegisterType<QGanttModelItem>("Gantt", 1, 0, "GanttModelItem");
	qmlRegisterType<QGanttData>("Gantt", 1, 0, "GanttData");
	qmlRegisterType<QGanttModelList>("Gantt", 1, 0, "GanttModelList");

	m_modelList = new QGanttModelList(CONFIGURATION_MODEL_SIZE);

	m_qmlpanelWidget = new QQuickWidget(this);
	m_qmlpanelWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);


	m_qmlpanelWidget->rootContext()->setContextProperty("ganttModelList", getModel());
	m_qmlpanelWidget->rootContext()->setContextProperty("ganttWidget", this);
	QMLGlobalVariableHelper::setWidgetGlobalVariable(m_qmlpanelWidget);

	m_qmlpanelWidget->setSource(QUrl(QStringLiteral("qrc:/qml/gantt/gantt.qml")));

	m_qmlpanelWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
	m_qmlpanelWidget->setAttribute(Qt::WA_TranslucentBackground);
	m_qmlpanelWidget->setClearColor(Qt::transparent);  // 设置 QML 视图的背景为透明

	QObject* pRoot = (QObject*)m_qmlpanelWidget->rootObject();
	//QObject *pRoot = (QObject*)pWidget->rootObject();
	if (pRoot != NULL) {
		connect(pRoot, SIGNAL(qml_quit_agent_edit_panel_sig()), this, SIGNAL(qml_quit_agent_edit_panel_sig()));
	}
	//    m_qmlpanelWidget->hide();

}

GanttWidget::~GanttWidget()
{

}

int GanttWidget::getDateTimePos(const QString & dateTimeTakeOff)
{
	QDateTime dtTakeOff = QDateTime::fromString(dateTimeTakeOff, "yyyy-MM-dd hh:mm:ss");
	return m_start_date_time_datetime.secsTo(dtTakeOff) / SCALE_DIFF;
}

int GanttWidget::getDateTimeLen(const QString & dateTimeTakeOff, const QString & dateTimeLanding)
{
	QDateTime dtTakeOff = QDateTime::fromString(dateTimeTakeOff, "yyyy-MM-dd hh:mm:ss");
	QDateTime dtLanding = QDateTime::fromString(dateTimeLanding, "yyyy-MM-dd hh:mm:ss");
	return dtTakeOff.secsTo(dtLanding) / SCALE_DIFF;
}

QVariant GanttWidget::getDateTimeStr(const QVariant &val)
{
	return QVariant::fromValue(m_start_date_time_datetime.addSecs(val.toUInt()*SCALE_DIFF).toString("yyyy-MM-dd hh:mm:ss"));
}

void GanttWidget::openFlightFile(const QVariant &dt)
{
	importexcel(dt.toString());
}

QObject *GanttWidget::getModel()
{
	return m_modelList;
}

void getSheetVal(QAxObject *worksheet, const QString& title, QString& val)
{
	QAxObject *range = worksheet->querySubObject("Range(QString)", title);
	val = range->property("Value").toString();
}

void getXlsxDocumentVal(const QXlsx::Document &worksheet, const QString& title, QString& val)
{
	val = worksheet.read(title).toString();
}

void GanttWidget::importexcel(const QString &fileName)
{
	QXlsx::Document _Xlsx(fileName);
	int lastColumn = _Xlsx.dimension().columnCount();
	int lastRow = _Xlsx.dimension().rowCount();

	std::unordered_map<QString, std::unordered_map<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>, FlightPlanConf> >& flightdata = DataManager::getInstance().m_flightdata;
    std::map<uint64_t, tagFlightEventTime> &total_flightEventTimedata = DataManager::getInstance().total_flightEventTimedata;
    std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *> &total_flightdata = DataManager::getInstance().total_flightdata;
	std::map<uint64_t, std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *> > flighttimedata;

	std::map<uint64_t, std::map<uint64_t, tagFlightEventTime>> flightEventTimedata;


    total_flightEventTimedata.clear();
    total_flightdata.clear();
	flightdata.clear();
	flighttimedata.clear();
	flightEventTimedata.clear();
	for (int i = 1; i < lastRow; i++)
	{
		FlightPlanConf fightplanConf;

		getXlsxDocumentVal(_Xlsx, "A" + QString::number(i + 1), fightplanConf.m_Date);
		if (!fightplanConf.m_Date.isEmpty())
		{
			getXlsxDocumentVal(_Xlsx, "B" + QString::number(i + 1), fightplanConf.m_FilghtNumber);
			getXlsxDocumentVal(_Xlsx, "C" + QString::number(i + 1), fightplanConf.m_DepArrFlag);
			getXlsxDocumentVal(_Xlsx, "D" + QString::number(i + 1), fightplanConf.m_PlaneNum);
			getXlsxDocumentVal(_Xlsx, "E" + QString::number(i + 1), fightplanConf.m_PlaneType);
			getXlsxDocumentVal(_Xlsx, "F" + QString::number(i + 1), fightplanConf.m_FlightClass);
			getXlsxDocumentVal(_Xlsx, "G" + QString::number(i + 1), fightplanConf.m_FlightLeg);
			getXlsxDocumentVal(_Xlsx, "H" + QString::number(i + 1), fightplanConf.m_FlightStartPlace);
			getXlsxDocumentVal(_Xlsx, "I" + QString::number(i + 1), fightplanConf.m_FlightEndPlace);
			getXlsxDocumentVal(_Xlsx, "J" + QString::number(i + 1), fightplanConf.m_PlanDateTimeTakeOff);
			getXlsxDocumentVal(_Xlsx, "K" + QString::number(i + 1), fightplanConf.m_ExpectedDateTimeTakeOff);
			getXlsxDocumentVal(_Xlsx, "L" + QString::number(i + 1), fightplanConf.m_RealityDateTimeTakeOff);

			getXlsxDocumentVal(_Xlsx, "M" + QString::number(i + 1), fightplanConf.m_PlanDateTimeLanding);
			getXlsxDocumentVal(_Xlsx, "N" + QString::number(i + 1), fightplanConf.m_ExpectedDateTimeLanding);
			getXlsxDocumentVal(_Xlsx, "O" + QString::number(i + 1), fightplanConf.m_RealityDateTimeLanding);
			getXlsxDocumentVal(_Xlsx, "P" + QString::number(i + 1), fightplanConf.m_Delay);

			getXlsxDocumentVal(_Xlsx, "Q" + QString::number(i + 1), fightplanConf.m_Seat);
			getXlsxDocumentVal(_Xlsx, "R" + QString::number(i + 1), fightplanConf.m_Terminal);
            getXlsxDocumentVal(_Xlsx, "S" + QString::number(i + 1), fightplanConf.m_Runway);
            QString flightid;
            getXlsxDocumentVal(_Xlsx, "T" + QString::number(i + 1), flightid);
            if(flightid.isEmpty())
            {
                fightplanConf.flightid = QString::number(i);
            }
            else
            {
                fightplanConf.flightid = flightid;
            }
            QString wps;
            getXlsxDocumentVal(_Xlsx, "U" + QString::number(i + 1), wps);
            if(!wps.isEmpty())
            {
                fightplanConf.wps = wps.remove(QRegExp("\\s+")).remove("\\");
            }
            if (fightplanConf.m_DepArrFlag == "到达")
			{
				fightplanConf.m_flight_dep_arr_type = E_FLIGHT_DEP_ARR_TYPE_ARR;
			}
			else
			{
				fightplanConf.m_flight_dep_arr_type = E_FLIGHT_DEP_ARR_TYPE_DEP;
			}

            if(fightplanConf.flightid.isEmpty())
            {
                fightplanConf.flightid = QString::number(FunctionAssistant::generate_random_positive_uint64());
            }

			if (fightplanConf.m_PlaneNum.isEmpty())
			{
				std::stringstream ss;
				ss << fightplanConf.printf();
				DataManager::getInstance().trans_log("读取航班时刻表：错误 ", ss, std::stringstream()<< " 航班号 为空 ");
			}

			QDateTime dateTime;
			QString date_time = fightplanConf.m_Date;
			QString _start_date_time = fightplanConf.m_Date + " 00:00:00";
			QDateTime _start_date_time_datetime = QDateTime::fromString(_start_date_time, "yyyyMMdd hh:mm:ss");
			uint64_t _start_date_time_timestamp = _start_date_time_datetime.toSecsSinceEpoch();

			if (i == 1)
			{
				m_start_date_time = _start_date_time;
				m_start_date_time_datetime = _start_date_time_datetime;
				m_start_date_time_timestamp = _start_date_time_timestamp;
			}
			
            dateTime = QDateTime::fromString(fightplanConf.m_Date, "yyyyMMdd");

			FlightPlanConf *pData = nullptr;
			auto flightdata_itor = std::find_if(flightdata.begin(),
				flightdata.end(), [&](const std::unordered_map<QString, std::unordered_map<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>, FlightPlanConf>>::value_type & vt) {
				return vt.first == fightplanConf.m_Date;
			});
			if (flightdata_itor != flightdata.end())
			{
				std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString> _key = std::make_tuple(fightplanConf.m_FilghtNumber, fightplanConf.m_flight_dep_arr_type, fightplanConf.m_Runway);
				std::unordered_map<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>, FlightPlanConf>& day_flights = flightdata_itor->second;
				day_flights.insert(std::make_pair(_key, std::move(fightplanConf)));
				pData = &day_flights.at(_key);
			}
			else
			{
				std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString> _key = std::make_tuple(fightplanConf.m_FilghtNumber, fightplanConf.m_flight_dep_arr_type, fightplanConf.m_Runway);
				std::unordered_map<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>, FlightPlanConf> day_flights;
				day_flights.insert(std::make_pair(_key, std::move(fightplanConf)));
				flightdata.insert(std::make_pair(date_time, std::move(day_flights)));
				pData = &(flightdata.at(date_time).at(_key));
			}

			if (pData)
			{
				uint64_t timedate = dateTime.toSecsSinceEpoch();
				uint64_t timestamp = 0;
				switch (fightplanConf.m_flight_dep_arr_type)
				{
				case E_FLIGHT_DEP_ARR_TYPE_ARR:
				{
					QDateTime dateTimestamp;
					dateTimestamp = QDateTime::fromString(pData->m_PlanDateTimeLanding, "yyyy-MM-dd hh:mm:ss");
					pData->updatePlanDateTimeLanding_timestamp(dateTimestamp.toSecsSinceEpoch());
					//dateTimestamp.addSecs(-pData->m_aheadtimelen);
					timestamp = pData->m_PlanDateTimeLanding_ahead_timestamp;
					//pData->m_PlanDateTimeLanding_ahead_timestamp = timestamp;

				}
				break;
				case E_FLIGHT_DEP_ARR_TYPE_DEP:
				{
					QDateTime dateTimestamp;
					dateTimestamp = QDateTime::fromString(pData->m_PlanDateTimeTakeOff, "yyyy-MM-dd hh:mm:ss");
					pData->updatePlanDateTimeTakeOff_timestamp(dateTimestamp.toSecsSinceEpoch());
					//dateTimestamp.addSecs(-pData->m_aheadtimelen);
					timestamp = pData->m_PlanDateTimeTakeOff_ahead_timestamp;
					//pData->m_PlanDateTimeTakeOff_ahead_timestamp = timestamp;
				}
				break;
				default:
					break;
				}

				auto flighttimedata_itor = std::find_if(flighttimedata.begin(),
					flighttimedata.end(), [&](const std::map<uint64_t, std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *> >::value_type & vt) {
					return vt.first == timedate;
				});
				if (flighttimedata_itor != flighttimedata.end())
				{
					std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf*>& day_flights = flighttimedata_itor->second;
					if (day_flights.find(std::make_tuple(timestamp, pData->m_flight_dep_arr_type, pData->m_Runway, pData->m_FilghtNumber)) != day_flights.end())
					{
						auto &existval = day_flights.at(std::make_tuple(timestamp, pData->m_flight_dep_arr_type, pData->m_Runway, pData->m_FilghtNumber));
						std::stringstream ss;
						ss  << existval->printf() << " 与 " << pData->printf();
						DataManager::getInstance().trans_log("读取航班时刻表：错误 ", ss, std::stringstream() << " 航班号重复 ");
					}
					else
					{
						day_flights.insert(std::make_pair(std::make_tuple(timestamp, pData->m_flight_dep_arr_type, pData->m_Runway, pData->m_FilghtNumber), /*std::move*/(pData)));

						std::map<uint64_t, tagFlightEventTime> &day_flight_events = flightEventTimedata.at(timedate);

						auto day_flight_events_itor = std::find_if(day_flight_events.begin(),
							day_flight_events.end(), [&](const std::map<uint64_t, tagFlightEventTime>::value_type & vt) {
							return vt.first == timestamp;
						});
						if (day_flight_events_itor != day_flight_events.end())
						{
							tagFlightEventTime &flightevent = day_flight_events_itor->second;
							flightevent.appendFlightCfg(pData);
						}
						else
						{
							tagFlightEventTime flightevent(timestamp, FunctionAssistant::generate_random_positive_uint64(), timestamp - timedate);
							flightevent.appendFlightCfg(pData);
							day_flight_events.insert(std::make_pair(timestamp, std::move(flightevent)));
						}
					}
				}
				else
				{
					std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf*> day_flights;
					day_flights.insert(std::make_pair(std::make_tuple(timestamp, pData->m_flight_dep_arr_type, pData->m_Runway, pData->m_FilghtNumber), /*std::move*/(pData)));
					flighttimedata.insert(std::make_pair(timedate, std::move(day_flights)));

					////////////////////////////////////////////////////////////////////////
					std::map<uint64_t, tagFlightEventTime> day_flight_events;
					tagFlightEventTime flightevent(timestamp, FunctionAssistant::generate_random_positive_uint64(), timestamp - timedate);
					flightevent.appendFlightCfg(pData);

					day_flight_events.insert(std::make_pair(timestamp, std::move(flightevent)));
					flightEventTimedata.insert(std::make_pair(timedate, std::move(day_flight_events)));
					////////////////////////////////////////////////////////////////////////
				}
			}
		}
	}

	auto flightEventTimedata_itor = flightEventTimedata.begin();
	while (flightEventTimedata_itor != flightEventTimedata.end())
	{
		std::map<uint64_t, tagFlightEventTime> &day_flight_events = flightEventTimedata_itor->second;
		auto day_flight_events_itor = day_flight_events.begin();
		while (day_flight_events_itor != day_flight_events.end())
		{
			auto day_flight_events_itor_tmp = day_flight_events_itor;
			auto day_flight_events_itor_prev = --day_flight_events_itor_tmp;
			day_flight_events_itor_tmp = day_flight_events_itor;
			auto day_flight_events_itor_next = ++day_flight_events_itor_tmp;
			tagFlightEventTime &flightevent = day_flight_events_itor->second;
			if (day_flight_events_itor == day_flight_events.begin())
			{
				if (day_flight_events_itor_next != day_flight_events.end())
				{
					flightevent.setNext(&day_flight_events_itor_next->second);
				}
			}
			else if (day_flight_events_itor_next == day_flight_events.end())
			{
				flightevent.setPrev(&day_flight_events_itor_prev->second);
			}
			else
			{
				flightevent.setPrev(&day_flight_events_itor_prev->second);
				if (day_flight_events_itor_next != day_flight_events.end())
				{
					flightevent.setNext(&day_flight_events_itor_next->second);
				}
			}
			day_flight_events_itor++;
		}
		flightEventTimedata_itor++;
	}

    //////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	auto tmp_flightEventTimedata_itor = flightEventTimedata.begin();
	while (tmp_flightEventTimedata_itor != flightEventTimedata.end())
	{
		std::map<uint64_t, tagFlightEventTime> &day_flight_events = tmp_flightEventTimedata_itor->second;
		auto day_flight_events_itor = day_flight_events.begin();
		while (day_flight_events_itor != day_flight_events.end())
		{
			auto tmpitor = total_flightEventTimedata.find(day_flight_events_itor->first);
			if (tmpitor == total_flightEventTimedata.end())
			{
				total_flightEventTimedata.insert(std::make_pair(day_flight_events_itor->first, day_flight_events_itor->second));
			}
			else
			{
				tmpitor->second.m_flightCfgs.insert(tmpitor->second.m_flightCfgs.end(), day_flight_events_itor->second.m_flightCfgs.begin(), day_flight_events_itor->second.m_flightCfgs.end());
			}
			day_flight_events_itor++;
		}
		tmp_flightEventTimedata_itor++;
	}



	auto total_flightEventTimedata_itor = total_flightEventTimedata.begin();
	while (total_flightEventTimedata_itor != total_flightEventTimedata.end())
	{
		auto total_flightEventTimedata_itor_tmp = total_flightEventTimedata_itor;
		auto total_flightEventTimedata_itor_prev = --total_flightEventTimedata_itor_tmp;
		total_flightEventTimedata_itor_tmp = total_flightEventTimedata_itor;
		auto total_flightEventTimedata_itor_next = ++total_flightEventTimedata_itor_tmp;
		tagFlightEventTime &flightevent = total_flightEventTimedata_itor->second;
		if (total_flightEventTimedata_itor == total_flightEventTimedata.begin())
		{
			if (total_flightEventTimedata_itor_next != total_flightEventTimedata.end())
			{
				flightevent.setNext(&total_flightEventTimedata_itor_next->second);
			}
		}
		else if (total_flightEventTimedata_itor_next == total_flightEventTimedata.end())
		{
			flightevent.setPrev(&total_flightEventTimedata_itor_prev->second);
		}
		else
		{
			flightevent.setPrev(&total_flightEventTimedata_itor_prev->second);
			if (total_flightEventTimedata_itor_next != total_flightEventTimedata.end())
			{
				flightevent.setNext(&total_flightEventTimedata_itor_next->second);
			}
		}
		total_flightEventTimedata_itor++;
	}


	std::stringstream ss;
	ss << "************* 时间驱动事件个数: " << total_flightEventTimedata.size();
	DataManager::getInstance().trans_log("读取航班时刻表： ", ss, std::stringstream(),false);

	/////////////////////////////////////////////////////////////////////////////
	auto flighttimedata_itor = flighttimedata.begin();
	while (flighttimedata_itor != flighttimedata.end())
	{
		std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *>& day_flights = flighttimedata_itor->second;

		if (flighttimedata_itor == flighttimedata.begin() && !day_flights.empty())
		{
			auto fightplanConfitor = day_flights.begin();
			if (fightplanConfitor != day_flights.end())
			{
				m_start_date_time = fightplanConfitor->second->m_Date + " 00:00:00";
				m_start_date_time_datetime = QDateTime::fromString(m_start_date_time, "yyyyMMdd hh:mm:ss");
				m_start_date_time_timestamp = m_start_date_time_datetime.toSecsSinceEpoch();
			}
		}

		total_flightdata.insert(day_flights.begin(), day_flights.end());
		flighttimedata_itor++;
	}

    dealTotalFlightData_ex(total_flightdata);
	/////////////////////////////////////////////////////////////////////////////

	//dealFlightData();
}

void GanttWidget::dealTotalFlightData(std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *>& total_flightdata)
{
    std::unordered_map<QString, std::unordered_map<QString, QString> >&  _InstagentInstance = DataManager::getInstance().m_InstagentInstance;
    std::unordered_map<QString, AgentInstanceInfo>& agentInstances = DataManager::getInstance().agentInstances();

    auto _InstagentInstance_itor = _InstagentInstance.begin();
    while (_InstagentInstance_itor != _InstagentInstance.end())
    {
        const std::unordered_map<QString, QString>& instance = _InstagentInstance_itor->second;
        auto _instance_itor = instance.begin();
        while (_instance_itor != instance.end())
        {
            auto agentInstances_itor = std::find_if(agentInstances.begin(),
                                                    agentInstances.end(), [&](const std::unordered_map<QString, AgentInstanceInfo>::value_type& vt) {
                                                        return vt.first == _instance_itor->second;
                                                    });
            if (agentInstances_itor != agentInstances.end())
            {
                agentInstances.erase(agentInstances_itor);
            }
            _instance_itor++;
        }
        _InstagentInstance_itor++;
    }

    _InstagentInstance.clear();

	m_modelList->clearAll();
	int icount = 0;
	auto day_flights_itor = total_flightdata.begin();
	while (day_flights_itor != total_flightdata.end())
	{

		const FlightPlanConf * pflighlt = day_flights_itor->second;
		if (!pflighlt)
		{
			day_flights_itor++;
			continue;
		}
		QGanttModel* m = new QGanttModel;
		m->setItemDataFactoryFunction(&createModelData);
		m->setContentWidth(CONFIGURATION_MODEL_SIZE);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		auto insertitem = [&](int pos, int length, const QString &context, const QColor& cl) {
			QGanttModelItem* item = new QGanttModelItem(pos, length);
			QGanttData * pQGanttData = new QGanttData();
			pQGanttData->setColor(cl);
			pQGanttData->setLabel(context);
			item->setData(QVariant::fromValue(pQGanttData));
			m->insertItem(item);
		};
		QString context = QString("起降标识：%1 航班号：%2 机型：%3 机号：%4 始发地：%5 到达地：%6 计划起飞时间：%7 计划落地时间：%8 航站楼：%9 跑道：%10")
			.arg(pflighlt->m_DepArrFlag)
			.arg(pflighlt->m_FilghtNumber)
			.arg(pflighlt->m_PlaneType)
			.arg(pflighlt->m_PlaneNum)
			.arg(pflighlt->m_FlightStartPlace)
			.arg(pflighlt->m_FlightEndPlace)
			.arg(pflighlt->m_PlanDateTimeTakeOff)
			.arg(pflighlt->m_PlanDateTimeLanding)
			.arg(pflighlt->m_Terminal)
			.arg(pflighlt->m_Runway);

		QColor itemcl;
		QColor itemaheadcl;
		uint64_t aheadpos;
		uint64_t iaheadlen = 0;
		switch (pflighlt->m_flight_dep_arr_type)
		{
		case E_FLIGHT_DEP_ARR_TYPE_ARR:
		{

			itemcl = QColor(227, 149, 65, 255);
			itemaheadcl = QColor(227, 149, 65, 64);
			QDateTime dtTakeOff = QDateTime::fromSecsSinceEpoch(pflighlt->m_PlanDateTimeLanding_ahead_timestamp);
			aheadpos = m_start_date_time_datetime.secsTo(dtTakeOff) / SCALE_DIFF;
			iaheadlen = pflighlt->m_aheadtimelen / SCALE_DIFF;


			insertitem(getDateTimePos(pflighlt->m_PlanDateTimeTakeOff),
				getDateTimeLen(pflighlt->m_PlanDateTimeTakeOff, pflighlt->m_PlanDateTimeLanding) - iaheadlen,
				context,
				itemaheadcl);

			insertitem(aheadpos,
				iaheadlen,
				"appear",
				itemcl);
		}break;
		case E_FLIGHT_DEP_ARR_TYPE_DEP:
		{
			itemcl = QColor(33, 140, 141, 255);
			itemaheadcl = QColor(33, 140, 141, 64);
			QDateTime dtTakeOff = QDateTime::fromSecsSinceEpoch(pflighlt->m_PlanDateTimeTakeOff_ahead_timestamp);
			aheadpos = m_start_date_time_datetime.secsTo(dtTakeOff) / SCALE_DIFF;
			iaheadlen = pflighlt->m_aheadtimelen / SCALE_DIFF;


			insertitem(getDateTimePos(pflighlt->m_PlanDateTimeTakeOff),
				getDateTimeLen(pflighlt->m_PlanDateTimeTakeOff, pflighlt->m_PlanDateTimeLanding),
				context,
				itemcl);

			insertitem(aheadpos,
				iaheadlen,
				"appear",
				itemaheadcl);
		}
		break;
		}
		icount++;
		QString numstr = QString("%1 . %2").arg(QString::number(icount)).arg(pflighlt->m_FilghtNumber);
		m_modelList->appendModel(m, numstr);
		/////////////////////////////////////////////////////////////////////////////////////////////////////////

		auto _InstagentInstanceitor = std::find_if(_InstagentInstance.begin(),
			_InstagentInstance.end(), [&](const std::unordered_map<QString, std::unordered_map<QString, QString>>::value_type &vt) {
			return vt.first == pflighlt->m_PlaneType;
		});
		if (_InstagentInstanceitor != _InstagentInstance.end())
		{
			std::unordered_map<QString, QString>& planeinstancelist = _InstagentInstanceitor->second;

			auto _planeinstancelistitor = std::find_if(planeinstancelist.begin(),
				planeinstancelist.end(), [&](const std::unordered_map<QString, QString>::value_type& vt) {
				return vt.first == pflighlt->m_PlaneNum;
			});
			if (_planeinstancelistitor == planeinstancelist.end())
			{
				planeinstancelist.insert(std::make_pair(pflighlt->m_PlaneNum, QString::number(FunctionAssistant::generate_random_positive_uint64())));
			}
		}
		else
		{
			std::unordered_map<QString, QString> planeinstancelist;
			planeinstancelist.insert(std::make_pair(pflighlt->m_PlaneNum, QString::number(FunctionAssistant::generate_random_positive_uint64())));
			_InstagentInstance.insert(std::make_pair(pflighlt->m_PlaneType, std::move(planeinstancelist)));
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		day_flights_itor++;
	}

	emit m_modelList->layoutChanged();

	QObject* pRoot = (QObject*)m_qmlpanelWidget->rootObject();
	QMetaObject::invokeMethod(pRoot, "updateHeader");


    tagAirPortInfo * ptagAirPortInfo = DataManager::getInstance().getAirportInfo(m_airport_code);
	if (nullptr == ptagAirPortInfo)
	{
		return;
    }

    /////////////////////////////////////////////////////////////////////////////
    emit deal_instagentData_sig(m_airport_code, m_allowRunway);
}

void GanttWidget::dealTotalFlightData_ex(std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *> &total_flightdata)
{
    std::unordered_map<QString, std::unordered_map<QString, QString> >&  _InstagentInstance = DataManager::getInstance().m_InstagentInstance;
    std::unordered_map<QString, AgentInstanceInfo>& agentInstances = DataManager::getInstance().agentInstances();

    auto _InstagentInstance_itor = _InstagentInstance.begin();
    while (_InstagentInstance_itor != _InstagentInstance.end())
    {
        const std::unordered_map<QString, QString>& instance = _InstagentInstance_itor->second;
        auto _instance_itor = instance.begin();
        while (_instance_itor != instance.end())
        {
            auto agentInstances_itor = std::find_if(agentInstances.begin(),
                                                    agentInstances.end(), [&](const std::unordered_map<QString, AgentInstanceInfo>::value_type& vt) {
                                                        return vt.first == _instance_itor->second;
                                                    });
            if (agentInstances_itor != agentInstances.end())
            {
                agentInstances.erase(agentInstances_itor);
            }
            _instance_itor++;
        }
        _InstagentInstance_itor++;
    }

    _InstagentInstance.clear();
    auto day_flights_itor = total_flightdata.begin();
    while (day_flights_itor != total_flightdata.end())
    {
        const FlightPlanConf * pflighlt = day_flights_itor->second;
        if (!pflighlt)
        {
            day_flights_itor++;
            continue;
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////////////

        auto _InstagentInstanceitor = std::find_if(_InstagentInstance.begin(),
                                                   _InstagentInstance.end(), [&](const std::unordered_map<QString, std::unordered_map<QString, QString>>::value_type &vt) {
                                                       return vt.first == pflighlt->m_PlaneType;
                                                   });
        if (_InstagentInstanceitor != _InstagentInstance.end())
        {
            std::unordered_map<QString, QString>& planeinstancelist = _InstagentInstanceitor->second;

            auto _planeinstancelistitor = std::find_if(planeinstancelist.begin(),
                                                       planeinstancelist.end(), [&](const std::unordered_map<QString, QString>::value_type& vt) {
                                                           return vt.first == pflighlt->m_PlaneNum;
                                                       });
            if (_planeinstancelistitor == planeinstancelist.end())
            {
                planeinstancelist.insert(std::make_pair(pflighlt->m_PlaneNum, QString::number(FunctionAssistant::generate_random_positive_uint64())));
            }
        }
        else
        {
            std::unordered_map<QString, QString> planeinstancelist;
            planeinstancelist.insert(std::make_pair(pflighlt->m_PlaneNum, QString::number(FunctionAssistant::generate_random_positive_uint64())));
            _InstagentInstance.insert(std::make_pair(pflighlt->m_PlaneType, std::move(planeinstancelist)));
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        day_flights_itor++;
    }

    tagAirPortInfo * ptagAirPortInfo = DataManager::getInstance().getAirportInfo(m_airport_code);
    if (nullptr == ptagAirPortInfo)
    {
        return;
    }

    /////////////////////////////////////////////////////////////////////////////
    emit deal_instagentData_sig(m_airport_code, m_allowRunway);
}



