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

#include "imageitem.h"
#include "ProjectionEPSG3857.h"

#include <QBrush>
#include <QPainter>
#include <QPen>

#define SCALE_DIFF (1.0f/4)
ImageItem::ImageItem(TYPE_ULID _ulid, const QGV::GeoPos& geoRect, QColor color, bool bSelectable /*= true*/)
    : BaseItem(color,_ulid)
    , m_bShowImage(true)
    , m_bSetShowImage(false)
    , m_pitch(0.0)
    , m_roll(0.0)
    , m_yaw(0.0)
{
    m_center = geoRect;
    setFlag(QGV::ItemFlag::IgnoreScale);
    setSelectable(bSelectable);
    mImage.load("./res/radar_6.png");
}

void ImageItem::onProjection(QGVMap* geoMap)
{
    setGeousingMap(geoMap);
    QGVDrawItem::onProjection(geoMap);
    updateDrawRect();
}

QPainterPath ImageItem::projShape() const
{
    QPainterPath path;
    path.addPolygon(mProjRect);
    path.addPolygon(mProjRectText);


    addPainterPath(path);

    return path;
}

void ImageItem::projPaint(QPainter* painter)
{
    preparePainter(painter);
    if(m_bShowImage)
    {
        painter->save();
        painter->translate(mProjRect.center().x(),mProjRect.center().y());
        painter->rotate(m_yaw);
        painter->translate(-mProjRect.center().x(),-mProjRect.center().y());
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->drawImage(mProjRect, mImage);
        painter->restore();
    }
    if(isSelectable())
    {
        drawIcon(painter);
    }
    painter->save();
    QFont font;
    font.setPointSize(9);
    font.setFamily("Microsoft YaHei");
    font.setLetterSpacing(QFont::AbsoluteSpacing,0);
    painter->setFont(font);
    QPen penpen;
    penpen = QPen(QBrush(QColor(0,255,255,255)), 5);
    penpen.setCosmetic(true);
    painter->setPen(penpen);
    painter->drawText(mProjRectText,Qt::AlignHCenter | Qt::AlignTop,m_entityname);
    painter->restore();


    drawUlid(painter);
    drawTrackingLine(painter);
}

QString ImageItem::projTooltip(const QPointF& projPos) const
{
    return "ulid: " + m_ulidstr;
    //    auto geo = getMap()->getProjection()->projToGeo(projPos);
    //    return "Rectangle with color " + mColor.name() + "\nPosition " + geo.latToString() + " " + geo.lonToString();

}


void ImageItem::updateDrawRect()
{
    if(geousingMap() && geousingMap()->getProjection())
    {
        LAT_LNG geopos{m_center.latitude(), m_center.longitude()};
        if(m_bTransfer)
        {
            geopos = projectionmercator::ProjectionEPSG3857::wgs84_to_gcj02(geopos.lat,geopos.lng);
        }

        m_centerpt = geousingMap()->getProjection()->geoToProj(QGV::GeoPos(geopos.lat, geopos.lng));
        mProjRect.setRect(m_centerpt.x() - mImage.width()*SCALE_DIFF, m_centerpt.y() - mImage.height()*SCALE_DIFF, mImage.width()*SCALE_DIFF*2, mImage.height()*SCALE_DIFF*2);
        mProjRectText.setRect(m_centerpt.x() - mImage.width()*SCALE_DIFF*2, m_centerpt.y() - mImage.height()*SCALE_DIFF, mImage.width()*SCALE_DIFF*4, mImage.height()*SCALE_DIFF*2);
    }
}

void ImageItem::updateTrackingCenter(const QGV::GeoPos &trackingpos)
{

}

void ImageItem::setH3index(H3INDEX newH3index)
{
    m_h3index = newH3index;
}

void ImageItem::setTransfer(bool bTransfer)
{
    if(m_bTransfer != bTransfer)
    {
        m_bTransfer = bTransfer;
        updateDrawRect();
        update();
    }
}

void ImageItem::setTransferTmp(bool bTransfer)
{
    if(geousingMap() && geousingMap()->getProjection())
    {
        LAT_LNG geopos{m_center.latitude(), m_center.longitude()};
        if(!bTransfer)
        {
            geopos = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(geopos.lat,geopos.lng);
        }

        m_centerpt = geousingMap()->getProjection()->geoToProj(QGV::GeoPos(geopos.lat,geopos.lng));
        mProjRect.setRect(m_centerpt.x() - mImage.width()*SCALE_DIFF, m_centerpt.y() - mImage.height()*SCALE_DIFF, mImage.width()*SCALE_DIFF*2, mImage.height()*SCALE_DIFF*2);
        mProjRectText.setRect(m_centerpt.x() - mImage.width()*SCALE_DIFF*2, m_centerpt.y() - mImage.height()*SCALE_DIFF, mImage.width()*SCALE_DIFF*4, mImage.height()*SCALE_DIFF*2);
        update();
    }
}


void ImageItem::updatePts(const QGV::GeoPos &geoRect, float pitch,  float roll,  float yaw)
{
    if(!m_bSetShowImage)
    {
        m_bSetShowImage = true;
        mImage.load("./res/uav.png");
    }
    m_pitch = pitch;
    m_roll = roll;
    m_yaw = yaw;
    m_center = geoRect;
    updateDrawRect();
    update();
}

void ImageItem::loadImage(const QString &imagename)
{
    m_bSetShowImage = true;
    if(imagename.isEmpty())
    {
        m_bShowImage = false;
    }
    else
    {
        mImage.load(imagename);
    }
}
