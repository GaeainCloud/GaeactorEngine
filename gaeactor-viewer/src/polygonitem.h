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
#include "LocationHelper.h"
#include "baseitem.h"
class PolygonItem : public BaseItem
{
    Q_OBJECT

public:
    explicit PolygonItem(TYPE_ULID _ulid, const QVector<QGV::GeoPos>& geoRect,double fspeed, QColor color, bool bSelectable = true);

    void addPointMove(const QGV::GeoPos& geoPos);
    void appendPoint(const QGV::GeoPos& geoPos);
    void appendFinish(const QGV::GeoPos& geoPos);

    void updatePts(const QVector<QGV::GeoPos>& geoRect);

    H3INDEX h3index() const;
    void setH3index(H3INDEX newH3index);

    void setIntersectionUlidPair(const QPair<TYPE_ULID, TYPE_ULID> &newIntersectionUlidPair);

    const QPair<TYPE_ULID, TYPE_ULID>& getIntersectionUlidPair() const;
    void setTransfers(bool bTransfer);

    void setTransfertmp(bool bTransfer);
protected:
    virtual void updateTracking() override { update();};
    virtual void updateVisiable(bool bVisiable) override {setVisible(bVisiable);};
    virtual void updateTrackingCenter(const QGV::GeoPos &trackingpos) override;
private:
    void onProjection(QGVMap* geoMap) override;
    QPainterPath projShape() const override;
    void projPaint(QPainter* painter) override;
    QString projTooltip(const QPointF& projPos) const override;
    void updateDrawInfo();
private:
    QPolygonF mProjRect;

    H3INDEX m_h3index;

    QPair<TYPE_ULID,TYPE_ULID> m_IntersectionUlidPair;
};
