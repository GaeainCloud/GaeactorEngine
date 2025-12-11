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

#include "pieitem.h"

#include "ProjectionEPSG3857.h"
#include <QBrush>
#include <QPainter>
#include <QPen>
#include "src/OriginalDateTime.h"
#define M_PI 3.14159265358979323846
PieItem::PieItem(TYPE_ULID _ulid, const QGV::GeoPos& startpos, double fspeed, qreal radius, int startAngle, int spanAngle, QColor color, bool bSelectable)
    : BaseItem(color,_ulid)
    , m_startAngle(startAngle)
    , m_spanAngle(spanAngle)
    , m_radius(radius)
{
    m_center = startpos;
    setSpeed(fspeed);
    setSelectable(bSelectable);
    for(int i = 0; i < 361; i++)
    {
        int angle = (i+180)%361;
        _verts.push_back(QPointF(sin(angle * M_PI/ 180.0f),cos(angle * M_PI/ 180.0f)));
    }

    setSelectable(true);
}

void PieItem::updateRadius(qreal radius, bool bFill)
{
    m_radius = radius;
    m_bFill = bFill;
    updateDrawInfo();
    update();
}

void PieItem::updateStartAngle(int angle)
{
    m_startAngle = angle;
    updateDrawInfo();
    update();
}

void PieItem::updateEndAngle(int angle)
{
    m_spanAngle = angle;
    updateDrawInfo();
    update();
}

void PieItem::setTransfer(bool bTransfer)
{

    if(geousingMap() && geousingMap()->getProjection())
    {
        QGV::GeoPos centertmp = QGV::GeoPos(m_center.latitude(), m_center.longitude());
        if(!bTransfer)
        {
            auto cettmp = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(centertmp.latitude(), centertmp.longitude());
            centertmp = QGV::GeoPos(cettmp.lat,cettmp.lng);
        }
        m_centerpt = geousingMap()->getProjection()->geoToProj(centertmp);
        mProjRect = getPiePoints(m_centerpt, m_radius,m_startAngle,m_spanAngle);

        update();
    }
}

void PieItem::updateTrackingCenter(const QGV::GeoPos &trackingpos)
{
    m_center = trackingpos;
    updateDrawInfo();
    update();
}


void PieItem::onProjection(QGVMap* geoMap)
{
    setGeousingMap(geoMap);
    QGVDrawItem::onProjection(geoMap);
    updateDrawInfo();
    setFirstPos(m_center);
}

QPainterPath PieItem::projShape() const
{
    QPainterPath path;
    path.addPolygon(mProjRect);
    addPainterPath(path);

    return path;
}

void PieItem::projPaint(QPainter* painter)
{
    preparePainter(painter);

    painter->drawPolygon(mProjRect);

#ifndef SHOW_UNFILL
    painter->save();
    painter->setPen(QPen(QBrush(QColor(mColor.red(),mColor.green(),mColor.blue())), m_penwidth*2));
    painter->drawPolygon(mProjRect);
    painter->restore();
#endif


    if(isSelectable())
    {
        drawIcon(painter);
    }

    drawTrackingLine(painter);
}

QString PieItem::projTooltip(const QPointF& projPos) const
{
    return "ulid: " + m_ulidstr;
    //    auto geo = getMap()->getProjection()->projToGeo(projPos);
    //    return "Rectangle with color " + mColor.name() + "\nPosition " + geo.latToString() + " " + geo.lonToString();
}

QPolygonF PieItem::getPiePoints(QPointF center, qreal radius, int startAngle, int spanAngle)
{
    QPolygonF points;
    points.push_back(center);
    for(int i = startAngle; i < startAngle + spanAngle+1;i++)
    {
        const QPointF& item = _verts.at(i % 361);
        points.push_back(QPointF(item.x()*radius+center.x(), item.y()*radius+center.y()));
    }
    points.push_back(center);
    return points;
}


void PieItem::updateDrawInfo()
{
    m_centerpt = geousingMap()->getProjection()->geoToProj(m_center);
    mProjRect = getPiePoints(m_centerpt, m_radius,m_startAngle,m_spanAngle);

    m_Polygons.clear();

    for(auto item:mProjRect)
    {
        m_Polygons.push_back(geousingMap()->getProjection()->projToGeo(item));
    }
}
