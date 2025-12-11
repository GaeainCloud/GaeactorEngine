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

#include "QGVLayerLocal.h"

#include <QtMath>
#include <QDir>
#include <QFile>
#include "QGVImage.h"

const QString URLTemplates =  "E:/SaveMapTiles/Google/Hybrid";
//const QString URLTemplates =  "./SaveMapTiles/Google/Hybrid";

QGVLayerLocal::QGVLayerLocal()
{
    createName();
    setDescription("Copyrights ©Google");
}


void QGVLayerLocal::createName()
{
    setName("Google Maps ");
}

int QGVLayerLocal::minZoomlevel() const
{
    return 1;
}

int QGVLayerLocal::maxZoomlevel() const
{
    return 19;
}

QString QGVLayerLocal::tilePosToUrl(const QGV::GeoTilePos& tilePos) const
{
#ifdef DIR_DIR_TYPE
    QString rootdirtmp = URLTemplates+QString("/%1").arg(tilePos.zoom());
    QString url = rootdirtmp+QString("/%1/%2.png").arg(tilePos.pos().x()).arg(tilePos.pos().y());
#else
    QString rootdirtmp = URLTemplates+QString("/%1").arg(tilePos.zoom());
    QString url = rootdirtmp+QString("/%1-%2.png").arg(tilePos.pos().x()).arg(tilePos.pos().y());
#endif
    return url;
}

void QGVLayerLocal::request(const QGV::GeoTilePos &tilePos)
{
    const QUrl url(tilePosToUrl(tilePos));
    int z = tilePos.zoom();
    int x = tilePos.pos().x();
    int y = tilePos.pos().y();

    QString imagePath = url.toString();
    QFileInfo fileInfo(imagePath);
    if(fileInfo.isFile())
    {
        QFile file(imagePath);
        if (file.open(QIODevice::ReadOnly))
        {
            QByteArray rawImage = file.readAll();
            file.close();
            auto tile = new QGVImage();
            tile->setGeometry(tilePos.toGeoRect());
            tile->loadImage(rawImage);
            tile->setProperty("drawDebug",
                              QString("%1\ntile(%2,%3,%4)")
                                  .arg(url.toString())
                                  .arg(tilePos.zoom())
                                  .arg(tilePos.pos().x())
                                  .arg(tilePos.pos().y()));
//            removeReply(tilePos);
            onTile(tilePos, tile);
        }
    }
}
