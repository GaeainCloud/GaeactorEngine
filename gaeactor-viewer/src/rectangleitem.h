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

#include <QBrush>
#include <QPen>
#include "head_define.h"
#include "baseitem.h"
class RectangleItem : public BaseItem
{
    Q_OBJECT

public:
    explicit RectangleItem(TYPE_ULID _ulid, const QGV::GeoRect& geoRect, QGV::ItemFlags flags,double fspeed, QColor color, bool bSelectable = true);
    void updateRect(const QGV::GeoRect& geoRect, bool bFill);
    virtual void setTransfer(bool bTransfer) override;
protected:
    virtual void updateTracking() override { update();};
    virtual void updateVisiable(bool bVisiable) override {setVisible(bVisiable);};
    virtual void updateTrackingCenter(const QGV::GeoPos &trackingpos) override;
private:
    void onProjection(QGVMap* geoMap) override;
    void onUpdate() override;
    QPainterPath projShape() const override;
    void projPaint(QPainter* painter) override;
    QPointF projAnchor() const override;
    QTransform projTransform() const override;
    QString projTooltip(const QPointF& projPos) const override;
    void updateDrawInfo();
private:
    QGV::GeoRect mGeoRect;
    QRectF mProjRect;

};
