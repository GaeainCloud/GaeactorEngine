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

#ifndef GANTTWIDGET_H_
#define GANTTWIDGET_H_
#include <QApplication>

#include <unordered_map>
#include <QDateTime>
#include <QString>
#include <QWidget>
#include <QQuickWidget>

#include "../datamanager/datamanager.hpp"

class QGanttModelList;

class GanttWidget: public QWidget
{
    Q_OBJECT
public:
    GanttWidget(QWidget *parent = nullptr);
    ~GanttWidget();
    Q_INVOKABLE QVariant getDateTimeStr(const QVariant &dt);

    Q_INVOKABLE void openFlightFile(const QVariant &dt);

    void importexcel(const QString &fileName);
    QObject* getModel();

    static int randBetween(int min, int max);
    static QVariant createModelData();

    void setAirportInfos(const QString& airport_code, const QStringList& allowRunway);
signals:
    void deal_instagentData_sig(const QString& airport_code, const QStringList& allowRunway);
    void qml_quit_agent_edit_panel_sig();
protected:
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;
private:

    void dealTotalFlightData(std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *>& total_flightdata);
    void dealTotalFlightData_ex(std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *>& total_flightdata);
    int getDateTimePos(const QString & dateTimeTakeOff);
    int getDateTimeLen(const QString & dateTimeTakeOff,const QString & dateTimeLanding);

private:
    QQuickWidget *m_qmlpanelWidget;
    QGanttModelList* m_modelList;
    QString m_start_date_time;
	QDateTime m_start_date_time_datetime;
	uint64_t m_start_date_time_timestamp;
    float m_zoomscale;


    QString m_airport_code;
    QStringList m_allowRunway;
};

#endif
