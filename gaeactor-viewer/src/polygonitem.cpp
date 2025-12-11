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

#include "polygonitem.h"
#include "ProjectionEPSG3857.h"
#include <QBrush>
#include <QPainter>
#include <QPen>
#include "src/OriginalDateTime.h"
PolygonItem::PolygonItem(TYPE_ULID _ulid, const QVector<QGV::GeoPos>& geoRect, double fspeed, QColor color, bool bSelectable /*= true*/)
    : BaseItem(color,_ulid)
{
    setSpeed(fspeed);
    m_Polygons = geoRect;
    setSelectable(bSelectable);
}

void PolygonItem::addPointMove(const QGV::GeoPos &geoPos)
{
    m_bFill = false;
    updateDrawInfo();

    mProjRect.push_back(geousingMap()->getProjection()->geoToProj(geoPos));
    update();
}

void PolygonItem::appendPoint(const QGV::GeoPos &geoPos)
{
    m_Polygons.push_back(geoPos);
    m_bFill = false;
    updateDrawInfo();
    update();
}

void PolygonItem::appendFinish(const QGV::GeoPos &geoPos)
{
    m_Polygons.push_back(geoPos);
    m_bFill = true;
    updateDrawInfo();
    update();
}

void PolygonItem::onProjection(QGVMap* geoMap)
{
    setGeousingMap(geoMap);
    QGVDrawItem::onProjection(geoMap);
    updateDrawInfo();

    if(!m_Polygons.empty())
    {
        setFirstPos(m_center);
    }
}

QPainterPath PolygonItem::projShape() const
{
    QPainterPath path;
    path.addPolygon(mProjRect);
    addPainterPath(path);
    return path;
}

void PolygonItem::projPaint(QPainter* painter)
{
    preparePainter(painter);

    if(mProjRect.size()==1)
    {
        painter->setPen(QPen(QBrush(QColor(mColor.red(),mColor.green(),mColor.blue()) ), m_penwidth));
        painter->setBrush(QBrush(QColor(mColor.red(),mColor.green(),mColor.blue()) ));
        painter->drawPoint(mProjRect.at(0));
    }
    else if(mProjRect.size()==2)
    {
        painter->setPen(QPen(QBrush(QColor(mColor.red(),mColor.green(),mColor.blue()) ), m_penwidth/2));
        painter->setBrush(QBrush(QColor(mColor.red(),mColor.green(),mColor.blue()) ));
        painter->drawLine(mProjRect.at(0), mProjRect.at(1));
    }
    else
    {
        painter->drawPolygon(mProjRect);

#ifndef SHOW_UNFILL
        painter->save();
        painter->setPen(QPen(QBrush(QColor(mColor.red(),mColor.green(),mColor.blue())), m_penwidth*2));
        painter->drawPolygon(mProjRect);
        painter->restore();
#endif

    }

    if(isSelectable())
    {
        drawIcon(painter);
    }

    drawTrackingLine(painter);

}

QString PolygonItem::projTooltip(const QPointF& projPos) const
{
    return "ulid: " + m_ulidstr;
    //    auto geo = getMap()->getProjection()->projToGeo(projPos);
    //    return "Rectangle with color " + mColor.name() + "\nPosition " + geo.latToString() + " " + geo.lonToString();

}

void PolygonItem::updateDrawInfo()
{
    if(geousingMap() && geousingMap()->getProjection())
    {
        mProjRect.clear();
        for(auto item : m_Polygons)
        {
            LAT_LNG geopos{item.latitude(), item.longitude()};
            if(m_bTransfer)
            {
                geopos = projectionmercator::ProjectionEPSG3857::wgs84_to_gcj02(geopos.lat,geopos.lng);
            }
            mProjRect.push_back(geousingMap()->getProjection()->geoToProj(QGV::GeoPos(geopos.lat, geopos.lng)));
        }

        if(!m_Polygons.empty())
        {
            LAT_LNG geopos{m_Polygons.at(0).latitude(), m_Polygons.at(0).longitude()};
            if(m_bTransfer)
            {
                geopos = projectionmercator::ProjectionEPSG3857::wgs84_to_gcj02(geopos.lat,geopos.lng);
            }
            m_center = QGV::GeoPos(geopos.lat,geopos.lng);
            m_centerpt = geousingMap()->getProjection()->geoToProj(m_center);
        }
    }
}

const QPair<TYPE_ULID, TYPE_ULID>& PolygonItem::getIntersectionUlidPair() const
{
    return m_IntersectionUlidPair;
}

void PolygonItem::setTransfers(bool bTransfer)
{
    if(geousingMap() && geousingMap()->getProjection())
    {
        mProjRect.clear();
        for(auto item : m_Polygons)
        {
            LAT_LNG geopos{item.latitude(), item.longitude()};
            if(bTransfer)
            {
                geopos = projectionmercator::ProjectionEPSG3857::wgs84_to_gcj02(geopos.lat,geopos.lng);
            }
            mProjRect.push_back(geousingMap()->getProjection()->geoToProj(QGV::GeoPos(geopos.lat,geopos.lng)));
        }
        update();
    }
}

void PolygonItem::setTransfertmp(bool bTransfer)
{
    if(geousingMap() && geousingMap()->getProjection())
    {
        mProjRect.clear();
        for(auto item : m_Polygons)
        {
            LAT_LNG geopos{item.latitude(), item.longitude()};
            if(!bTransfer)
            {
                geopos = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(geopos.lat,geopos.lng);
            }

            mProjRect.push_back(geousingMap()->getProjection()->geoToProj(QGV::GeoPos(geopos.lat,geopos.lng)));
        }
        update();
    }
}

void PolygonItem::updateTrackingCenter(const QGV::GeoPos &trackingpos)
{
    double steplat = trackingpos.latitude() - m_center.latitude();
    double steplng = trackingpos.longitude() - m_center.longitude();

    for(auto& item:m_Polygons)
    {
        item.setLat(item.latitude()+ steplat);
        item.setLon(item.longitude()+ steplng);
    }

    updateDrawInfo();
    update();
}

void PolygonItem::setIntersectionUlidPair(const QPair<TYPE_ULID, TYPE_ULID> &newIntersectionUlidPair)
{
    m_IntersectionUlidPair = newIntersectionUlidPair;
}

H3INDEX PolygonItem::h3index() const
{
    return m_h3index;
}

void PolygonItem::setH3index(H3INDEX newH3index)
{
    m_h3index = newH3index;
}

void PolygonItem::updatePts(const QVector<QGV::GeoPos> &geoRect)
{
    m_Polygons = geoRect;
    updateDrawInfo();
    update();
}
