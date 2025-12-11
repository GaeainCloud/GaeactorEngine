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
#include "baseitem.h"
#include <QGeoView/QGVMapQGView.h>
class EllipseItem : public BaseItem
{
    Q_OBJECT

public:
    explicit EllipseItem(TYPE_ULID _ulid, const QGV::GeoPos& geoRect, double fspeed, qreal radius, QColor color, bool bSelectable = true);

    void updateRadius(qreal radius, bool bFill);
    virtual void setTransfer(bool bTransfer) override;
protected:
    virtual void updateTracking() override { update();};
    virtual void updateVisiable(bool bVisiable) override {setVisible(bVisiable);geousingMap()->geoView()->scene()->update();};
    virtual void updateTrackingCenter(const QGV::GeoPos &trackingpos) override;
private:
    void onProjection(QGVMap* geoMap) override;
    QPainterPath projShape() const override;
    void projPaint(QPainter* painter) override;
    QString projTooltip(const QPointF& projPos) const override;

    QPolygonF getEllipsePoints(QPointF center, qreal radius);
    void updateDrawInfo();
private:
    QVector<QPointF> _verts;
    qreal m_radius;
    QPolygonF mProjRect;

};
