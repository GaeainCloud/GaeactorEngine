#include "QGVProjectionGCJ02.h"

#include <QLineF>
#include <QtMath>

#define ee (0.00669342162296594323)
QGVProjectionGCJ02::QGVProjectionGCJ02()
    : QGVProjection("",
                    "GCJ02",
                    "")
{
    mEarthRadius = 6378137.0; /* meters */
    mOriginShift = 2.0 * M_PI * mEarthRadius / 2.0;
    mGeoBoundary = QGV::GeoRect(85, -180, -85, +180);
    mProjBoundary = geoToProj(mGeoBoundary);
}

QGV::GeoRect QGVProjectionGCJ02::boundaryGeoRect() const
{
    return mGeoBoundary;
}

QRectF QGVProjectionGCJ02::boundaryProjRect() const
{
    return mProjBoundary;
}

QPointF QGVProjectionGCJ02::geoToProj(const QGV::GeoPos &geoPos) const
{
    auto pos = geoPos;
//    wgs84_to_gcj02(pos);
    gcj02_to_wgs84(pos);
    const double lon = pos.longitude();
    const double lat = (pos.latitude() > mGeoBoundary.topLeft().latitude()) ? mGeoBoundary.topLeft().latitude()
                                                                               : pos.latitude();
    const double x = lon * mOriginShift / 180.0;
    const double preY = -qLn(qTan((90.0 + lat) * M_PI / 360.0)) / (M_PI / 180.0);
    const double y = preY * mOriginShift / 180.0;
    return QPointF(x, y);
}

QGV::GeoPos QGVProjectionGCJ02::projToGeo(const QPointF &projPos) const
{
    const double lon = (projPos.x() / mOriginShift) * 180.0;
    const double preLat = (-projPos.y() / mOriginShift) * 180.0;
    const double lat = 180.0 / M_PI * (2.0 * qAtan(qExp(preLat * M_PI / 180.0)) - M_PI / 2.0);
    auto pos = QGV::GeoPos(lat, lon);
    wgs84_to_gcj02(pos);
//    gcj02_to_wgs84(pos);
    return pos;
}

QRectF QGVProjectionGCJ02::geoToProj(const QGV::GeoRect &geoRect) const
{
    QRectF rect;
    rect.setTopLeft(geoToProj(geoRect.topLeft()));
    rect.setBottomRight(geoToProj(geoRect.bottomRight()));
    return rect;
}

QGV::GeoRect QGVProjectionGCJ02::projToGeo(const QRectF &projRect) const
{
    return QGV::GeoRect(projToGeo(projRect.topLeft()), projToGeo(projRect.bottomRight()));
}

double QGVProjectionGCJ02::geodesicMeters(const QPointF &projPos1, const QPointF &projPos2) const
{
    const QGV::GeoPos geoPos1 = projToGeo(projPos1);
    const QGV::GeoPos geoPos2 = projToGeo(projPos2);
    return geodesicMeters(geoPos1, geoPos2);
}

double QGVProjectionGCJ02::geodesicMeters(const QGV::GeoPos &Pos1, const QGV::GeoPos &Pos2) const
{
    const double latitudeArc = (Pos1.latitude() - Pos2.latitude()) * M_PI / 180.0;
    const double longitudeArc = (Pos1.longitude() - Pos2.longitude()) * M_PI / 180.0;
    const double latitudeH = qPow(sin(latitudeArc * 0.5), 2);
    const double lontitudeH = qPow(sin(longitudeArc * 0.5), 2);
    const double lonFactor = cos(Pos1.latitude() * M_PI / 180.0) * cos(Pos2.latitude() * M_PI / 180.0);
    const double arcInRadians = 2.0 * asin(sqrt(latitudeH + lonFactor * lontitudeH));
    return mEarthRadius * arcInRadians;
}


void QGVProjectionGCJ02::wgs84_to_gcj02(QGV::GeoPos &pos) const
{
    if(out_of_boundary(pos.longitude(), pos.latitude())) return;

    double dLat = transfrom_lat(pos.longitude() - 105.0, pos.latitude() - 35.0);
    double dLon = transfrom_lon(pos.longitude() - 105.0, pos.latitude() - 35.0);
    double radLat = pos.latitude() / 180.0 * M_PI;
    double magic = sin(radLat);
    magic = 1 - ee * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((mEarthRadius * (1 - ee)) / (magic * sqrtMagic) * M_PI);
    dLon = (dLon * 180.0) / (mEarthRadius / sqrtMagic * cos(radLat) * M_PI);
    pos.setLat(pos.latitude() + dLat);
    pos.setLon(pos.longitude() + dLon);
}

void QGVProjectionGCJ02::gcj02_to_wgs84(QGV::GeoPos &pos) const
{
    if(out_of_boundary(pos.longitude(), pos.latitude())) return;

    double dlat = transfrom_lat(pos.longitude() - 105.0, pos.latitude() - 35.0);
    double dlng = transfrom_lon(pos.longitude() - 105.0, pos.latitude() - 35.0);
    double radlat = pos.latitude() / 180.0 * M_PI;
    double magic = sin(radlat);
    magic = 1 - ee * magic * magic;
    double sqrtmagic = sqrt(magic);
    dlat = (dlat * 180.0) / ((mEarthRadius * (1 - ee)) / (magic * sqrtmagic) * M_PI);
    dlng = (dlng * 180.0) / (mEarthRadius / sqrtmagic * cos(radlat) * M_PI);
    double mglat = pos.latitude() + dlat;
    double mglng = pos.longitude() + dlng;
    pos.setLat(pos.latitude() * 2 - mglat);
    pos.setLon(pos.longitude() * 2 - mglng);
}

double QGVProjectionGCJ02::transfrom_lat(double x, double y) const
{
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(abs(x));
    ret += (20.0 * sin(6.0 * x * M_PI) + 20.0 * sin(2.0 * x * M_PI)) * 2.0 / 3.0;
    ret += (20.0 * sin(y * M_PI) + 40.0 * sin(y / 3.0 * M_PI)) * 2.0 / 3.0;
    ret += (160.0 * sin(y / 12.0 * M_PI) + 320 * sin(y * M_PI / 30.0)) * 2.0 / 3.0;
    return ret;
}

double QGVProjectionGCJ02::transfrom_lon(double x, double y) const
{
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(abs(x));
    ret += (20.0 * sin(6.0 * x * M_PI) + 20.0 * sin(2.0 * x * M_PI)) * 2.0 / 3.0;
    ret += (20.0 * sin(x * M_PI) + 40.0 * sin(x / 3.0 * M_PI)) * 2.0 / 3.0;
    ret += (150.0 * sin(x / 12.0 * M_PI) + 300.0 * sin(x / 30.0 * M_PI)) * 2.0 / 3.0;
    return ret;
}

bool QGVProjectionGCJ02::out_of_boundary(double lon, double lat) const
{
    if (lon < 72.004 || lon > 137.8347)
        return true;
    if (lat < 0.8293 || lat > 55.8271)
        return true;
    return false;
}
