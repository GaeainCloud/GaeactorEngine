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

#include "QGVLayerTilesOnline.h"
#include "QGVImage.h"

#include <QDir>
#include <iostream>

#include <QtMath>

//#define DIR_DIR_TYPE
const char * LOCAL_CACHE_DIR =  "E:/Cache/SaveMapTiles";
//const char * LOCAL_CACHE_DIR = "./SaveMapTiles";

QGVLayerTilesOnline::~QGVLayerTilesOnline()
{
    qDeleteAll(mRequest);
}

void QGVLayerTilesOnline::setDoladMaxZoom(int zoommax)
{
    if(zoommax == -1)
    {
        m_downloadzoommax = maxZoomlevel();
    }
    else
    {
        m_downloadzoommax = zoommax;
    }
}

void QGVLayerTilesOnline::downloadAreaData(const QGV::GeoRect &areaGeoRect)
{
    QList<QGV::GeoTilePos> titleposlist = getTitlePos(minZoomlevel(),m_downloadzoommax,areaGeoRect);
    downloadcache(titleposlist);
}

QList<QGV::GeoTilePos> QGVLayerTilesOnline::getTitlePos(int mZoombegin, int mZoomend, const QGV::GeoRect &areaGeoRect)
{
    QList<QGV::GeoTilePos> retlist;
    for(int i = mZoombegin; i <= mZoomend; i++)
    {
        int mCurZoom = i;
        const QPoint topLeft = QGV::GeoTilePos::geoToTilePos(mCurZoom, areaGeoRect.topLeft()).pos();
        const QPoint bottomRight = QGV::GeoTilePos::geoToTilePos(mCurZoom, areaGeoRect.bottomRight()).pos();
        QRect mCurRect = QRect(topLeft, bottomRight);
        QMultiMap<qreal, QGV::GeoTilePos> missing;
        for (int x = mCurRect.left(); x < mCurRect.right(); ++x) {
            for (int y = mCurRect.top(); y < mCurRect.bottom(); ++y) {
                const auto tilePos = QGV::GeoTilePos(mCurZoom, QPoint(x, y));
                if(isCacheExist(tilePos))
                {
                    continue;
                }
                qreal radius = qSqrt(qPow(x - mCurRect.center().x(), 2) + qPow(y - mCurRect.center().y(), 2));
                missing.insert(radius, tilePos);
            }
        }

        for (const QGV::GeoTilePos& tilePos : missing) {
            retlist.push_back(tilePos);
        }
    }
    return retlist;
}

void QGVLayerTilesOnline::downloadcache(const QList<QGV::GeoTilePos> &titleposlist)
{
    m_itotalCount = titleposlist.size();
    m_ireplyCount = 0;
    for(auto &tilePos: titleposlist)
    {
        request2(tilePos);
    }
}

int QGVLayerTilesOnline::zoommax() const
{
    return m_downloadzoommax;
}

void QGVLayerTilesOnline::onProjection(QGVMap* geoMap)
{
    Q_ASSERT(QGV::getNetworkManager());
    QGVLayerTiles::onProjection(geoMap);
    connect(QGV::getNetworkManager(), &QNetworkAccessManager::finished, this, &QGVLayerTilesOnline::onReplyFinished);
    connect(QGV::getNetworkManager2(), &QNetworkAccessManager::finished, this, &QGVLayerTilesOnline::onReplyFinished2);

}

void QGVLayerTilesOnline::onClean()
{
    Q_ASSERT(QGV::getNetworkManager());
    disconnect(QGV::getNetworkManager(), 0, this, 0);
}

void QGVLayerTilesOnline::request(const QGV::GeoTilePos& tilePos)
{
    if(!loadLocalCache(tilePos))
    {
        const QUrl url(tilePosToUrl(tilePos));
        QNetworkRequest request(url);
        request.setRawHeader("User-Agent",
                             "Mozilla/5.0 (Windows; U; MSIE "
                             "6.0; Windows NT 5.1; SV1; .NET "
                             "CLR 2.0.50727)");
        request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        QNetworkReply* reply = QGV::getNetworkManager()->get(request);
        reply->setProperty("TILE_OWNER", QVariant::fromValue(this));
        reply->setProperty("TILE_REQUEST", true);
        reply->setProperty("TILE_POS", QVariant::fromValue(tilePos));
        mRequest[tilePos] = reply;
//        std::cout << "request net data ..... " <<url.toString().toStdString()<< std::endl;
        qgvDebug() << "request" << url;
    }
}

bool QGVLayerTilesOnline::loadLocalCache(const QGV::GeoTilePos &tilePos)
{
    QString imagePath = getCacheFilename(tilePos);

    QFileInfo fileInfo(imagePath);
    if(fileInfo.exists() && fileInfo.isFile())
    {
        QFile file(imagePath);
        if (file.open(QIODevice::ReadOnly))
        {
            const auto rawImage = file.readAll();
            file.close();
            auto tile = new QGVImage();
            tile->setGeometry(tilePos.toGeoRect());
            tile->loadImage(rawImage);
            tile->setProperty("drawDebug",
                              QString("%1\ntile(%2,%3,%4)")
                                  .arg(imagePath)
                                  .arg(tilePos.zoom())
                                  .arg(tilePos.pos().x())
                                  .arg(tilePos.pos().y()));
            onTile(tilePos, tile);
//            std::cout << "load cache data  succeed. " << imagePath.toStdString()<< std::endl;
            return true;
        }
    }
    return false;
}


void QGVLayerTilesOnline::cancel(const QGV::GeoTilePos& tilePos)
{
    removeReply(tilePos);
}

void QGVLayerTilesOnline::request2(const QGV::GeoTilePos &tilePos)
{
    if(!isCacheExist(tilePos))
    {
        const QUrl url(tilePosToUrl(tilePos));
        QNetworkRequest request(url);
        request.setRawHeader("User-Agent",
                             "Mozilla/5.0 (Windows; U; MSIE "
                             "6.0; Windows NT 5.1; SV1; .NET "
                             "CLR 2.0.50727)");
        request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        QNetworkReply* reply = QGV::getNetworkManager2()->get(request);
        reply->setProperty("TILE_OWNER", QVariant::fromValue(this));
        reply->setProperty("TILE_REQUEST", true);
        reply->setProperty("TILE_POS", QVariant::fromValue(tilePos));
        m_retlist[tilePos] = reply;

//        std::cout << "request net data ..... " <<url.toString().toStdString()<< std::endl;
    }
}

void QGVLayerTilesOnline::onReplyFinished(QNetworkReply* reply)
{
    const auto tileRequest = reply->property("TILE_REQUEST").toBool();
    if (!tileRequest) {
        return;
    }
    const auto tileOwner = reply->property("TILE_OWNER").value<QGVLayerTilesOnline*>();
    if (tileOwner != this) {
        return;
    }
    const auto tilePos = reply->property("TILE_POS").value<QGV::GeoTilePos>();

    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            qgvCritical() << "ERROR" << reply->errorString();
        }
        removeReply(tilePos);
        return;
    }
    const auto rawImage = reply->readAll();
    auto tile = new QGVImage();
    tile->setGeometry(tilePos.toGeoRect());
    tile->loadImage(rawImage);
    tile->setProperty("drawDebug",
                      QString("%1\ntile(%2,%3,%4)")
                              .arg(reply->url().toString())
                              .arg(tilePos.zoom())
                              .arg(tilePos.pos().x())
                              .arg(tilePos.pos().y()));
    saveImage(tilePos, rawImage,false);
    removeReply(tilePos);
    onTile(tilePos, tile);
}

void QGVLayerTilesOnline::onReplyFinished2(QNetworkReply *reply)
{
    const auto tileRequest = reply->property("TILE_REQUEST").toBool();
    if (!tileRequest) {
        return;
    }
    const auto tileOwner = reply->property("TILE_OWNER").value<QGVLayerTilesOnline*>();
    if (tileOwner != this) {
        return;
    }
    const auto tilePos = reply->property("TILE_POS").value<QGV::GeoTilePos>();

    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            qgvCritical() << "ERROR" << reply->errorString();
        }
        removeReply(tilePos);
        return;
    }
    m_ireplyCount++;

    emit updateDownloadProgresssig(m_itotalCount, m_ireplyCount, tilePos.zoom(),m_downloadzoommax);

    const auto rawImage = reply->readAll();
    saveImage(tilePos, rawImage ,true);
    QNetworkReply* replyret = m_retlist.value(tilePos, nullptr);
    if (reply == nullptr) {
        return;
    }

    mRequest.remove(tilePos);
    replyret->abort();
    replyret->close();
    replyret->deleteLater();
}

void QGVLayerTilesOnline::removeReply(const QGV::GeoTilePos& tilePos)
{
    QNetworkReply* reply = mRequest.value(tilePos, nullptr);
    if (reply == nullptr) {
        return;
    }
    mRequest.remove(tilePos);
    reply->abort();
    reply->close();
    reply->deleteLater();
}

void QGVLayerTilesOnline::saveImage(const QGV::GeoTilePos &tilePos, const QByteArray &by, bool bCacheArea)
{
    QString mapdirdesc = getMapDirDesc();
    QString URLTemplates = LOCAL_CACHE_DIR;
    URLTemplates += mapdirdesc;

    int z = tilePos.zoom();
    int x = tilePos.pos().x();
    int y = tilePos.pos().y();

    QString rootdirtmp = URLTemplates+QString("/%1").arg(tilePos.zoom());
    QDir dirTMP(rootdirtmp);
    if(!dirTMP.exists())
    {
        dirTMP.mkpath(rootdirtmp);
    }
    QString imagePath = getCacheFilename(tilePos);

    QImage image;
    image.loadFromData(by);
    bool res =image.save(imagePath);
    const QUrl url(tilePosToUrl(tilePos));
    if (res)
    {
        if(!bCacheArea)
        {
            //std::cout << "export succeed. x=" << x << " y=" << y << " z=" << z <<" "<<url.toString().toStdString()<< std::endl;
        }
    }
    else
    {
        std::cout << "export failed. x=" << x << " y=" << y << " z=" << z <<" "<<url.toString().toStdString()<< std::endl;
    }
}


QString QGVLayerTilesOnline::getMapDirDesc()
{
    return QString("");
}

QString QGVLayerTilesOnline::getCacheFilename(const QGV::GeoTilePos &tilePos)
{
    QString mapdirdesc = getMapDirDesc();
    QString URLTemplates = LOCAL_CACHE_DIR;
    URLTemplates += mapdirdesc;

#ifdef DIR_DIR_TYPE
    QString imagePath = URLTemplates+QString("/%1/%2/%3.png").arg(tilePos.zoom()).arg(tilePos.pos().x()).arg(tilePos.pos().y());
#else
    QString imagePath = URLTemplates+QString("/%1/%2-%3.png").arg(tilePos.zoom()).arg(tilePos.pos().x()).arg(tilePos.pos().y());
#endif
    return imagePath;
}

bool QGVLayerTilesOnline::isCacheExist(const QGV::GeoTilePos &tilePos)
{
    QString imagePath = getCacheFilename(tilePos);
    QFileInfo fileInfo(imagePath);
    if(fileInfo.exists() && fileInfo.isFile())
    {
        return true;
    }
    return false;
}

