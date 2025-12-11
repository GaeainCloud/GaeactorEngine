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

#include "QGVLayerTiles.h"

#include <QNetworkReply>

class QGV_LIB_DECL QGVLayerTilesOnline : public QGVLayerTiles
{
    Q_OBJECT

public:
    ~QGVLayerTilesOnline();

    void setDoladMaxZoom(int zoommax);
    virtual void downloadAreaData(const QGV::GeoRect& areaGeoRect);
    int zoommax() const;

signals:

    void updateDownloadProgresssig(int total,int reply,int zoom, int zoommax);

protected:
    virtual QString tilePosToUrl(const QGV::GeoTilePos& tilePos) const = 0;

protected:
    void onProjection(QGVMap* geoMap) override;
    void onClean() override;
    virtual void request(const QGV::GeoTilePos& tilePos) override;
    void cancel(const QGV::GeoTilePos& tilePos) override;
    void request2(const QGV::GeoTilePos& tilePos);
    void onReplyFinished(QNetworkReply* reply);
    void onReplyFinished2(QNetworkReply* reply);
    void removeReply(const QGV::GeoTilePos& tilePos);
    void saveImage(const QGV::GeoTilePos& tilePos, const QByteArray & by, bool bCacheArea);

    virtual QString getMapDirDesc();
    QString getCacheFilename(const QGV::GeoTilePos& tilePos);
    bool isCacheExist(const QGV::GeoTilePos& tilePos);
    bool loadLocalCache(const QGV::GeoTilePos& tilePos);

    QList<QGV::GeoTilePos> getTitlePos(int mZoombegin,int mZoomend,const QGV::GeoRect& areaGeoRect);
    void downloadcache(const QList<QGV::GeoTilePos>& titleposlist);

private:
    QMap<QGV::GeoTilePos, QNetworkReply*> mRequest;
    int m_itotalCount;
    int m_ireplyCount;
    QMap<QGV::GeoTilePos, QNetworkReply*> m_retlist;
    int m_downloadzoommax;
};
