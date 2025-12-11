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

#include "baseitem.h"

#include <QPainter>
#include "ProjectionEPSG3857.h"

#include "src/OriginalDateTime.h"
#define SPEED (300.0f)
BaseItem::BaseItem(QColor color, TYPE_ULID _ulid)
    :m_geousingMap(nullptr)
    , mColor(color)
    , m_bFill(true)
    , m_trackingIndex(0)
    , m_showTrackingLine(false)
    , m_eTrackingEndType(E_TRACKING_END_TYPE_END_CYCLE)
    , m_bAutoGenreateCycle(false)
    , m_bEditFinished(true)
    , m_end_cycle_radius(5*1000)
    , m_speedcoeff(1.0)
    , m_penwidth(10)
    , m_bTransfer(true)
{
    m_ulid = _ulid;
    std::string sensorulidstr = QString::number(m_ulid).toStdString();
    m_ulidstr = QString::fromStdString(sensorulidstr);

    mImage.load("./res/sensor-error_40_40.png");
    m_speed = SPEED;
}

BaseItem::~BaseItem()
{

}

void BaseItem::updateFirstPos(const QGV::GeoPos &geoPos)
{
    if(mTrackingGeo.size()>0)
    {
        mTrackingGeo[0] = geoPos;
    }
}

void BaseItem::setFirstPos(const QGV::GeoPos &geoPos)
{
    mTrackingGeo.push_back(geoPos);
}

void BaseItem::addPointMove(const QGV::GeoPos &geoPos)
{
    mTrackingPoints.clear();
    for(auto item : mTrackingGeo)
    {
        mTrackingPoints.push_back(m_geousingMap->getProjection()->geoToProj(item));
    }
    mTrackingPoints.push_back(m_geousingMap->getProjection()->geoToProj(geoPos));
    updateTracking();
}

void BaseItem::setGeousingMap(QGVMap *newGeousingMap)
{
    m_geousingMap = newGeousingMap;

    m_penwidth = 2/1/m_geousingMap->getCamera().scale();
}

QGVMap *BaseItem::geousingMap() const
{
    return m_geousingMap;
}

bool BaseItem::isTrackingable() const
{
    return mTrackingGeo.size() > 1;
}

void BaseItem::addPainterPath(QPainterPath &path) const
{
    if(isTrackingable() && m_showTrackingLine)
    {
        path.addPolygon(mTrackingPoints);
    }
}

void BaseItem::drawUlid(QPainter *painter)
{
    painter->save();
    QFont font;
    font.setPointSize(48);
    font.setFamily("Microsoft YaHei");
    font.setLetterSpacing(QFont::AbsoluteSpacing,0);
    painter->setFont(font);
    QPen penpen;
    penpen = QPen(QBrush(Qt::black), 5);
    penpen.setCosmetic(true);
    painter->setPen(penpen);
    painter->drawText(QPointF(m_centerpt.x() - mImage.width()/2, m_centerpt.y()- mImage.height()),m_ulidstr);
    painter->restore();
}

#define SCALE_DIFF (100)

void BaseItem::drawIcon(QPainter *painter)
{
    return;
    drawUlid(painter);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    //QPointF pt = geousingMap()->getProjection()->geoToProj(m_Polygons.at(0));
    QRectF _image_rect(m_centerpt.x() - mImage.width()*SCALE_DIFF, m_centerpt.y() - mImage.height()*SCALE_DIFF, mImage.width()*SCALE_DIFF*2, mImage.height()*SCALE_DIFF*2);
//    if (!isFlag(QGV::ItemFlag::IgnoreScale))
//    {
//        const double pixelFactor = 1.0 / geousingMap()->getCamera().scale();
//        _image_rect.setSize(_image_rect.size() + QSizeF(pixelFactor, pixelFactor));
//    }
//    painter->drawImage(QPointF(m_centerpt.x() - mImage.width()*20/2, m_centerpt.y()- mImage.height()*20/2), mImage);
    painter->drawImage(_image_rect, mImage);
}

void BaseItem::drawTrackingLine(QPainter *painter)
{
    if(isTrackingable() && m_showTrackingLine)
    {

        painter->save();
        painter->setPen(QPen(QBrush(QColor(mColor.red(),mColor.green(),mColor.blue(),255)), m_penwidth));

        for(int i = 0;i < mTrackingPoints.size()-1;i++)
        {
            painter->drawLine(mTrackingPoints.at(i),mTrackingPoints.at(i+1));
        }
        painter->restore();
    }
}

void BaseItem::preparePainter(QPainter *painter)
{
#ifdef SHOW_UNFILL
    painter->setPen(QPen(QBrush(QColor(mColor.red(),mColor.green(),mColor.blue())), m_penwidth*2));
#else
    if(m_bFill)
    {
        //        painter->setPen(QPen(QBrush(Qt::black), m_penwidth));
        painter->setPen(QPen(QBrush(color()), m_penwidth/10));
        painter->setBrush(QBrush(color()));
    }
    else
    {
        painter->setPen(QPen(QBrush(QColor(mColor.red(),mColor.green(),mColor.blue())), m_penwidth*2));
    }
#endif
}

void BaseItem::setBFill(bool newBFill)
{
    m_bFill = newBFill;
}

void BaseItem::setPenwidth(int newPenwidth)
{
    m_penwidth = newPenwidth;
    updateTracking();
}

void BaseItem::setTransfer(bool bTransfer)
{
    if(m_bTransfer != bTransfer)
    {
        m_bTransfer = bTransfer;
        if(m_geousingMap && m_geousingMap->getProjection())
        {
            mTrackingPoints.clear();
            for(int index = 0;index < mTrackingGeo.size(); index++)
            {
                LAT_LNG geopos{mTrackingGeo.at(index).latitude(), mTrackingGeo.at(index).longitude()};
                if(m_bTransfer)
                {
                    geopos = projectionmercator::ProjectionEPSG3857::wgs84_to_gcj02(geopos.lat,geopos.lng);
                }
                mTrackingPoints.push_back(m_geousingMap->getProjection()->geoToProj(QGV::GeoPos(geopos.lat,geopos.lng)));
            }
        }
        updateTracking();
    }
}

bool BaseItem::showLine() const
{
    return m_showTrackingLine;
}

void BaseItem::setShowTrackingLine(bool newShowLine)
{
    m_showTrackingLine = newShowLine;
}

QColor BaseItem::color() const
{
    return QColor(mColor.red(),mColor.green(),mColor.blue(),96) ;
}

double BaseItem::end_cycle_radius() const
{
    return m_end_cycle_radius;
}

void BaseItem::setEnd_cycle_radius(double newEnd_cycle_radius)
{
    m_end_cycle_radius = newEnd_cycle_radius;
}

BaseItem::E_TRACKING_END_TYPE BaseItem::eTrackingEndType() const
{
    return m_eTrackingEndType;
}

void BaseItem::setETrackingEndType(E_TRACKING_END_TYPE newETrackingEndType)
{
    m_eTrackingEndType = newETrackingEndType;
}

void BaseItem::needgenerateEndCycle()
{
    if(m_eTrackingEndType == E_TRACKING_END_TYPE_END_CYCLE && m_trackingIndex == 0 && !m_bAutoGenreateCycle)
    {
        double m_end_cycle_radius = 50;
        LAT_LNG center{mTrackingGeo.back().latitude(), mTrackingGeo.back().longitude()};
        QVector<LAT_LNG>  trackinggeopt = projectionmercator::ProjectionEPSG3857::generateCirclePoint(center, m_end_cycle_radius);
        mTrackingGeo.clear();
        for(int index = 0; index < trackinggeopt.size(); index++)
        {
            if(index < trackinggeopt.size() - 1)
            {
                mTrackingGeo.push_back(QGV::GeoPos(trackinggeopt[index].lat, trackinggeopt[index].lng));
            }
            else
            {
                appendFinish(QGV::GeoPos(trackinggeopt[index].lat, trackinggeopt[index].lng));
                generateTrackingPoint();
            }
        }
        if(!m_trackingpts.empty())
        {
            mTrackingGeo.push_back(m_trackingpts.back());
        }
        m_bAutoGenreateCycle = true;
    }
}

double BaseItem::speed() const
{
    return m_speed;
}

void BaseItem::setSpeed(double newSpeed)
{
    m_speed = newSpeed;
    generateTrackingPoint();
}

QPair<TYPE_ULID,TYPE_ULID> BaseItem::entityulid() const
{
    return m_entityulid;
}

void BaseItem::setEntityulid(const QPair<TYPE_ULID,TYPE_ULID> &newEntityulid)
{
    m_entityulid = newEntityulid;
}

QString BaseItem::entityname() const
{
    return m_entityname;
}

void BaseItem::setEntityname(const QString &newEntityname)
{
    m_entityname = newEntityname;
}


void BaseItem::appendPoint(const QGV::GeoPos &geoPos)
{
    if(m_eTrackingEndType == E_TRACKING_END_TYPE_END_CYCLE && m_bAutoGenreateCycle && m_bEditFinished)
    {
        m_bEditFinished = false;
        if(mTrackingGeo.size() == 361)
        {
            QGV::GeoPos sgeoPos = mTrackingGeo.back();
            mTrackingGeo.clear();
            mTrackingGeo.push_back(sgeoPos);
        }
    }
    mTrackingGeo.push_back(geoPos);

    mTrackingPoints.clear();
    for(int index = 0;index < mTrackingGeo.size(); index++)
    {
        LAT_LNG geopos{mTrackingGeo.at(index).latitude(), mTrackingGeo.at(index).longitude()};
        if(m_bTransfer)
        {
            geopos = projectionmercator::ProjectionEPSG3857::wgs84_to_gcj02(geopos.lat,geopos.lng);
        }
        mTrackingPoints.push_back(m_geousingMap->getProjection()->geoToProj(QGV::GeoPos(geopos.lat,geopos.lng)));
    }

    updateTracking();
}

void BaseItem::appendFinish(const QGV::GeoPos &geoPos)
{
    mTrackingGeo.push_back(geoPos);
    appendFinished();
}

void BaseItem::appendFinished()
{
    mTrackingPoints.clear();
    for(auto item : mTrackingGeo)
    {
        mTrackingPoints.push_back(m_geousingMap->getProjection()->geoToProj(QGV::GeoPos(item.latitude(), item.longitude())));
    }
    m_bEditFinished = true;
    m_bAutoGenreateCycle = false;
    updateTracking();
}

QGV::GeoPos BaseItem::center() const
{
    return m_center;
}

void BaseItem::updateSpeedCoeff(double fspeedcoeff)
{
    m_speedcoeff = fspeedcoeff;
    generateTrackingPoint();
}

const QVector<QGV::GeoPos> &BaseItem::geoRect() const
{
    return m_Polygons;
}

TYPE_ULID BaseItem::ulid() const
{
    return m_ulid;
}


void BaseItem::generateTrackingPoint()
{
    m_trackingpts.clear();
    for(int j = 0;j < mTrackingGeo.size()-1;/*j++*/)
    {
        QGV::GeoPos start = mTrackingGeo.at(j);
        QGV::GeoPos end = mTrackingGeo.at(j+1);

        double distance =  m_geousingMap->getProjection()->geodesicMeters(start, end);
        double fspeed = m_speed/(1000/TIMER_INTERVAL)*m_speedcoeff;
        if(distance < fspeed)
        {
            j = j+1;
            while(distance < fspeed && j+1 < mTrackingGeo.size()-1)
            {
                start = end;
                j = j+1;
                end = mTrackingGeo.at(j);
                distance +=  m_geousingMap->getProjection()->geodesicMeters(start, end);
            }
            m_trackingpts.push_back(mTrackingGeo.at(j-1));
        }
        else
        {
            j++;
            double speedNum = distance/fspeed;
            if(speedNum > 0)
            {
                double steplat = fabs(end.latitude() - start.latitude()) / speedNum;
                double steplng = fabs(end.longitude() - start.longitude()) / speedNum;
                for(int i = 0; i < speedNum; i++)
                {
                    QGV::GeoPos tmp;
                    if(end.latitude() > start.latitude())
                    {
                        tmp.setLat(start.latitude() + steplat*i);
                    }
                    else
                    {
                        tmp.setLat(start.latitude() - steplat*i);
                    }

                    if(end.longitude() > start.longitude())
                    {
                        tmp.setLon(start.longitude() + steplng*i);
                    }
                    else
                    {
                        tmp.setLon(start.longitude() - steplng*i);
                    }
                    //if(!m_trackingpts.contains(tmp))
                    {
                        m_trackingpts.push_back(tmp);
                    }
                }
            }
        }
    }
    updateTracking();
}

bool BaseItem::updateTrackingPoint()
{
    bool bEnd = false;
    if(m_trackingpts.size() > 0 )
    {
        m_trackingIndex++;
        m_trackingIndex = m_trackingIndex % m_trackingpts.size();
        needgenerateEndCycle();
        updateTrackingCenter(m_trackingpts.at(m_trackingIndex));
        if(m_trackingIndex == 0)
        {
            bEnd = true;
        }
    }
    else
    {
        needgenerateEndCycle();
    }
    return bEnd;
}
