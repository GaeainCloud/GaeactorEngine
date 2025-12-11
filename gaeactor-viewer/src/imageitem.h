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
class ImageItem : public BaseItem
{
    Q_OBJECT

public:
    explicit ImageItem(TYPE_ULID _ulid, const QGV::GeoPos& geoRect, QColor color, bool bSelectable = true);

    void updatePts(const QGV::GeoPos& geoRect, float pitch, float roll, float yaw);

    void loadImage(const QString &imagename);

    H3INDEX h3index() const;
    void setH3index(H3INDEX newH3index);

    virtual void setTransfer(bool bTransfer) override;
    void setTransferTmp(bool bTransfer);
protected:
    virtual void updateTracking() override { update();};
    virtual void updateVisiable(bool bVisiable) override {setVisible(bVisiable);};
    virtual void updateTrackingCenter(const QGV::GeoPos &trackingpos) override;
private:
    void onProjection(QGVMap* geoMap) override;
    QPainterPath projShape() const override;
    void projPaint(QPainter* painter) override;
    QString projTooltip(const QPointF& projPos) const override;
    void updateDrawRect();
private:
    QRectF mProjRect;
    QRectF mProjRectText;
    H3INDEX m_h3index;
    bool m_bShowImage;

    bool m_bSetShowImage;

    float m_pitch;
    float m_roll;
    float m_yaw;

};
