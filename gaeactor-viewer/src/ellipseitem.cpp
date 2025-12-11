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

#include "ellipseitem.h"

#include "ProjectionEPSG3857.h"
#include <QBrush>
#include <QPainter>
#include <QPen>
#include "src/OriginalDateTime.h"
#define M_PI 3.14159265358979323846
EllipseItem::EllipseItem(TYPE_ULID _ulid, const QGV::GeoPos &startpos,double fspeed, qreal radius, QColor color, bool bSelectable)
    : BaseItem(color,_ulid)
    , m_radius(radius)
{
    m_center = startpos;
    setSpeed(fspeed);
    setEnd_cycle_radius(100*1000);
    setSelectable(true);

    for(int i = 0; i < 361; i++)
    {
        int angle = (i+180)%361;
        _verts.push_back(QPointF(sin(angle * M_PI/ 180.0f),cos(angle * M_PI/ 180.0f)));
    }
}

void EllipseItem::updateRadius(qreal radius, bool bFill)
{
    m_radius = radius;
    m_bFill = bFill;
    updateDrawInfo();
    update();
}

void EllipseItem::setTransfer(bool bTransfer)
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
        mProjRect = getEllipsePoints(m_centerpt, m_radius);

        update();
    }
}

void EllipseItem::updateTrackingCenter(const QGV::GeoPos &trackingpos)
{
    m_center = trackingpos;
    updateDrawInfo();
    update();
}


void EllipseItem::onProjection(QGVMap* geoMap)
{
    setGeousingMap(geoMap);
    QGVDrawItem::onProjection(geoMap);
    updateDrawInfo();
    setFirstPos(m_center);
}

QPainterPath EllipseItem::projShape() const
{
    QPainterPath path;
    path.addPolygon(mProjRect);

    addPainterPath(path);
    return path;
}

void EllipseItem::projPaint(QPainter* painter)
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

QString EllipseItem::projTooltip(const QPointF& projPos) const
{
    return "ulid: " + m_ulidstr;
    //    auto geo = getMap()->getProjection()->projToGeo(projPos);
    //    return "Rectangle with color " + mColor.name() + "\nPosition " + geo.latToString() + " " + geo.lonToString();
}

QPolygonF EllipseItem::getEllipsePoints(QPointF center, qreal radius)
{
    QPolygonF points;
    for (auto item:_verts)
    {
        points.push_back(QPointF(item.x()*radius+center.x(), item.y()*radius+center.y()));
    }
    return points;
}

void EllipseItem::updateDrawInfo()
{
    QGV::GeoPos centertmp = QGV::GeoPos(m_center.latitude(), m_center.longitude());
    m_centerpt = geousingMap()->getProjection()->geoToProj(centertmp);
    mProjRect = getEllipsePoints(m_centerpt, m_radius);

    m_Polygons.clear();

    for(auto item:mProjRect)
    {
        m_Polygons.push_back(geousingMap()->getProjection()->projToGeo(item));
    }
}

