/***************************************************************************
 * QGeoView is a Qt / C ++ widget for visualizing geographic data.
 * Copyright (C) 2018-2020 Andrey Yaroshenko.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see https://www.gnu.org/licenses.
 ****************************************************************************/

#pragma once

#include <QGeoView/QGVDrawItem.h>
#include "head_define.h"

//#define DEBUG_HEX

//#define SHOW_UNFILL

#define TIMER_INTERVAL (100)
class BaseItem: public QGVDrawItem
{
    Q_OBJECT
public:

    enum E_TRACKING_END_TYPE
    {
        E_TRACKING_END_TYPE_REPEAT,
        E_TRACKING_END_TYPE_END_CYCLE,
    };
    BaseItem(QColor color, TYPE_ULID _ulid);
    ~BaseItem();
    void updateFirstPos(const QGV::GeoPos &geoPos);
    void setFirstPos(const QGV::GeoPos &geoPos);
    void addPointMove(const QGV::GeoPos& geoPos);
    void appendPoint(const QGV::GeoPos& geoPos);
    void appendFinish(const QGV::GeoPos& geoPos);
    void appendFinished();
    void generateTrackingPoint();
    bool updateTrackingPoint();
    TYPE_ULID ulid() const;
    QGV::GeoPos center() const;

    void updateSpeedCoeff(double fspeedcoeff);

    bool isTrackingable() const;
    const QVector<QGV::GeoPos> &geoRect() const;
    double speed() const;
    void setSpeed(double newSpeed);

    QString entityname() const;
    void setEntityname(const QString &newEntityname);

    QPair<TYPE_ULID, TYPE_ULID> entityulid() const;
    void setEntityulid(const QPair<TYPE_ULID,TYPE_ULID> &newEntityulid);

    E_TRACKING_END_TYPE eTrackingEndType() const;
    void setETrackingEndType(E_TRACKING_END_TYPE newETrackingEndType);

    void needgenerateEndCycle();
    double end_cycle_radius() const;
    void setEnd_cycle_radius(double newEnd_cycle_radius);

    QColor color() const;

    bool showLine() const;
    void setShowTrackingLine(bool newShowLine);

    void setPenwidth(int newPenwidth);

    virtual void setTransfer(bool bTransfer);

    void setBFill(bool newBFill);

protected:
    void setGeousingMap(QGVMap *newGeousingMap);
    QGVMap *geousingMap() const;


    void addPainterPath(QPainterPath &path) const;

    void drawUlid(QPainter* painter);
    void drawIcon(QPainter* painter);
    void drawTrackingLine(QPainter* painter);
    void preparePainter(QPainter* painter);
protected:
    virtual void updateTracking() = 0;
    virtual void updateTrackingCenter(const QGV::GeoPos &trackingpos) = 0;
    virtual void updateVisiable(bool bVisiable) = 0;
protected:
    QGVMap* m_geousingMap;
    QVector<QGV::GeoPos> mTrackingGeo;
    QPolygonF mTrackingPoints;
    TYPE_ULID m_ulid;

    QColor mColor;
    bool m_bFill;
    QGV::GeoPos m_center;
    QPointF m_centerpt;
    QImage mImage;

    QString m_ulidstr;
    QVector<QGV::GeoPos> m_trackingpts;
    int m_trackingIndex;
    QVector<QGV::GeoPos> m_Polygons;

    double m_speed;

    bool m_showTrackingLine;


    QString m_entityname;
    QPair<TYPE_ULID,TYPE_ULID> m_entityulid;

    E_TRACKING_END_TYPE m_eTrackingEndType;


    bool m_bAutoGenreateCycle;

    bool m_bEditFinished;

    double m_end_cycle_radius;

    double m_speedcoeff;

    int m_penwidth;

    bool m_bTransfer;

};
