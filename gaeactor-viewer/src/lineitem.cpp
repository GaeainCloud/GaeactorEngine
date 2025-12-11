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

#include "lineitem.h"

#include <QBrush>
#include <QPainter>
#include <QPen>
#include "src/OriginalDateTime.h"
#include <QDebug>
#include "ProjectionEPSG3857.h"
#include "LocationHelper.h"
#define MAX_LINE_POINT (200)

LineItem::LineItem(TYPE_ULID _ulid, const QGV::GeoPos& startpos, double fspeed, QColor color, bool bSelectable)
    : BaseItem(color,_ulid)
    , m_bGuide(false)
{
//    setETrackingEndType(E_TRACKING_END_TYPE_END_CYCLE);
    mTrackingGeo.push_back(startpos);
    setSpeed(fspeed);
    setSelectable(bSelectable);
}



void LineItem::appendPoint(const QGV::GeoPos &geoPos,bool appendmouse, bool bautoremovefront)
{
    if(appendmouse)
    {
//        std::cout<<"{"<<geoPos.longitude()<<","<<geoPos.latitude()<<"}"<<std::endl;
//        qDebug()<<"{"<<geoPos.lonToString()<<","<<geoPos.latToString()<<"},";
    }
    if(bautoremovefront)
    {
        removefrontpt();
    }
    BaseItem::appendPoint(geoPos);
}

void LineItem::appendguidept(const QGV::GeoPos &geoPos)
{
    m_bGuide = true;
    m_guidePt = geoPos;
}

const QVector<QGV::GeoPos>& LineItem::geoLine() const
{
    return m_trackingpts;
}

void LineItem::dealGuidePt()
{
    if(m_bGuide)
    {
        QGV::GeoPos startpos;
        if(m_trackingpts.size() > 0)
        {
            m_trackingIndex++;
            m_trackingIndex = m_trackingIndex % m_trackingpts.size();
            startpos = m_trackingpts.at(m_trackingIndex);
        }
        mTrackingGeo.clear();
        mTrackingGeo.push_back(startpos);
        appendFinish(m_guidePt);
        generateTrackingPoint();
        m_trackingIndex = 0;
        m_bGuide = false;
        m_bAutoGenreateCycle = false;
    }
}

void LineItem::setTransfertmp(bool bTransfer)
{
    if(geousingMap() && geousingMap()->getProjection())
    {
        mTrackingPoints.clear();
        for(auto item : mTrackingGeo)
        {
            LAT_LNG geopos{item.latitude(), item.longitude()};
            if(!bTransfer)
            {
                geopos = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(geopos.lat,geopos.lng);
            }
            mTrackingPoints.push_back(geousingMap()->getProjection()->geoToProj(QGV::GeoPos(geopos.lat,geopos.lng)));
        }
        update();
    }
}

bool LineItem::updateTrackingPoints( QGV::GeoPos& ret)
{
    bool bget = false;
    if(m_trackingpts.size() > 0)
    {
        m_trackingIndex++;
        m_trackingIndex = m_trackingIndex % m_trackingpts.size();
        needgenerateEndCycle();
        ret = m_trackingpts.at(m_trackingIndex);
        bget = true;
    }
    else
    {
        needgenerateEndCycle();
    }
    return bget;
}

bool LineItem::updateTrackingLastPoints(QGV::GeoPos &ret)
{
    bool bget = false;
    if(m_trackingpts.size() > 0 && m_trackingIndex > 0)
    {
        m_trackingIndex = m_trackingIndex % m_trackingpts.size();
        ret = m_trackingpts.at(m_trackingIndex);
        bget = true;
    }
    return bget;
}

void LineItem::popfrontpt()
{
    if(mTrackingGeo.size() > 0)
    {
        mTrackingGeo.pop_front();
    }
}

void LineItem::removefrontpt()
{
    if(mTrackingGeo.size() > MAX_LINE_POINT)
    {
        mTrackingGeo.pop_front();
    }
}

void LineItem::onProjection(QGVMap* geoMap)
{
    setGeousingMap(geoMap);
    QGVDrawItem::onProjection(geoMap);
    mTrackingPoints.clear();
    for(auto item : mTrackingGeo)
    {
        LAT_LNG geopos{item.latitude(), item.longitude()};
        if(!m_bTransfer)
        {
            geopos = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(geopos.lat,geopos.lng);
        }
        mTrackingPoints.push_back(geousingMap()->getProjection()->geoToProj(QGV::GeoPos(geopos.lat,geopos.lng)));
    }
}

QPainterPath LineItem::projShape() const
{
    QPainterPath path;
    path.addPolygon(mTrackingPoints);

    return path;
}

void LineItem::projPaint(QPainter* painter)
{
    if(m_showTrackingLine)
    {
        painter->setPen(QPen(QBrush(QColor(mColor.red(),mColor.green(),mColor.blue())), m_penwidth));
        for(int i = 0;i < mTrackingPoints.size()-1;i++)
        {
            painter->drawLine(mTrackingPoints.at(i),mTrackingPoints.at(i+1));
        }
    }
}

QString LineItem::projTooltip(const QPointF& projPos) const
{
    return "ulid: " + m_ulidstr;
    //    auto geo = getMap()->getProjection()->projToGeo(projPos);
    //    return "Rectangle with color " + mColor.name() + "\nPosition " + geo.latToString() + " " + geo.lonToString();
}

