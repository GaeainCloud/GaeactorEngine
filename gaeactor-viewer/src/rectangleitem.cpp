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

#include "rectangleitem.h"

#include "ProjectionEPSG3857.h"
#include <QBrush>
#include <QPainter>
#include <QPen>
#include "src/OriginalDateTime.h"
RectangleItem::RectangleItem(TYPE_ULID _ulid, const QGV::GeoRect& geoRect, QGV::ItemFlags flags, double fspeed, QColor color, bool bSelectable)
    : BaseItem(color,_ulid)
    , mGeoRect(geoRect)
{
    setSpeed(fspeed);
    setETrackingEndType(E_TRACKING_END_TYPE_REPEAT);
    setFlags(flags);
    setSelectable(bSelectable);

}

void RectangleItem::updateRect(const QGV::GeoRect &geoRect, bool bFill)
{
    mGeoRect = geoRect;
    m_bFill = bFill;
    updateDrawInfo();
    updateFirstPos(m_center);
    update();
}

void RectangleItem::setTransfer(bool bTransfer)
{

    if(geousingMap() && geousingMap()->getProjection())
    {
        QGV::GeoRect mGeoRecttmp(mGeoRect);
        QGV::GeoPos centertmp = QGV::GeoPos(m_center.latitude(), m_center.longitude());
        if(!bTransfer)
        {
            auto topLeft = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(mGeoRect.topLeft().latitude(), mGeoRect.topLeft().longitude());
            auto bottomRight = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(mGeoRect.bottomRight().latitude(), mGeoRect.bottomRight().longitude());
            mGeoRecttmp = QGV::GeoRect(QGV::GeoPos(topLeft.lat,topLeft.lng) , QGV::GeoPos(bottomRight.lat,bottomRight.lng));
        }
        mProjRect = geousingMap()->getProjection()->geoToProj(mGeoRecttmp);

        m_center = QGV::GeoPos((mGeoRecttmp.topLeft().latitude() + mGeoRecttmp.bottomRight().latitude())/2,
                               (mGeoRecttmp.topLeft().longitude() + mGeoRecttmp.bottomRight().longitude())/2);
        m_centerpt = geousingMap()->getProjection()->geoToProj(m_center);

        update();
    }
}

void RectangleItem::updateTrackingCenter(const QGV::GeoPos &trackingpos)
{
    double steplat = trackingpos.latitude() - m_center.latitude();
    double steplng = trackingpos.longitude() - m_center.longitude();

    mGeoRect = QGV::GeoRect(mGeoRect.topLeft().latitude() + steplat,
                            mGeoRect.topLeft().longitude() + steplng,
                            mGeoRect.bottomRight().latitude() + steplat,
                            mGeoRect.bottomRight().longitude() + steplng);

    updateDrawInfo();
    update();
}


void RectangleItem::onProjection(QGVMap* geoMap)
{
    setGeousingMap(geoMap);
    QGVDrawItem::onProjection(geoMap);
    updateDrawInfo();
    setFirstPos(m_center);
}

void RectangleItem::onUpdate()
{
    QGVDrawItem::onUpdate();
}

QPainterPath RectangleItem::projShape() const
{
    QPainterPath path;
    path.addRect(mProjRect);
    addPainterPath(path);
    return path;
}

void RectangleItem::projPaint(QPainter* painter)
{
    preparePainter(painter);

    painter->drawRect(mProjRect);

#ifndef SHOW_UNFILL
    painter->save();
    painter->setPen(QPen(QBrush(QColor(mColor.red(),mColor.green(),mColor.blue())), m_penwidth*2));
    painter->drawRect(mProjRect);
    painter->restore();
#endif


    if(isSelectable())
    {
        drawIcon(painter);
    }

    drawTrackingLine(painter);
}

QPointF RectangleItem::projAnchor() const
{
    return mProjRect.center();
}

QTransform RectangleItem::projTransform() const
{
    return QGV::createTransfromAzimuth(projAnchor(), 45);
}

QString RectangleItem::projTooltip(const QPointF& projPos) const
{
    return "ulid: " + m_ulidstr;
    //    auto geo = getMap()->getProjection()->projToGeo(projPos);
    //    return "Rectangle with color " + mColor.name() + "\nPosition " + geo.latToString() + " " + geo.lonToString();
}

void RectangleItem::updateDrawInfo()
{
    mProjRect = geousingMap()->getProjection()->geoToProj(mGeoRect);

    m_Polygons.clear();
    m_Polygons.push_back(mGeoRect.topLeft());
    m_Polygons.push_back(mGeoRect.topRight());
    m_Polygons.push_back(mGeoRect.bottomRight());
    m_Polygons.push_back(mGeoRect.bottomLeft());
    m_Polygons.push_back(mGeoRect.topLeft());

    m_center = QGV::GeoPos((mGeoRect.topLeft().latitude() + mGeoRect.bottomRight().latitude())/2,
                           (mGeoRect.topLeft().longitude() + mGeoRect.bottomRight().longitude())/2);
    m_centerpt = geousingMap()->getProjection()->geoToProj(m_center);
}

