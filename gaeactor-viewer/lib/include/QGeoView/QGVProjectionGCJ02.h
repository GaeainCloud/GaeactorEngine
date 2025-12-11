#ifndef QGVPROJECTIONGCJ02_H
#define QGVPROJECTIONGCJ02_H

#include "QGVProjection.h"
/*
 * 使用GCJ02纠偏坐标系，类似于WGS84只是略带偏移，且仅在国内使用
*/

class QGV_LIB_DECL QGVProjectionGCJ02: public QGVProjection
{
public:
    QGVProjectionGCJ02();
    virtual ~QGVProjectionGCJ02() = default;
    void init();

private:
    QGV::GeoRect boundaryGeoRect() const override final;
    QRectF boundaryProjRect() const override final;

    QPointF geoToProj(QGV::GeoPos const& geoPos) const override final;
    QGV::GeoPos projToGeo(QPointF const& projPos) const override final;
    QRectF geoToProj(QGV::GeoRect const& geoRect) const override final;
    QGV::GeoRect projToGeo(QRectF const& projRect) const override final;

    double geodesicMeters(QPointF const& projPos1, QPointF const& projPos2) const override final;
    double geodesicMeters(QGV::GeoPos const& Pos1, QGV::GeoPos const& Pos2) const override final;


    void wgs84_to_gcj02(QGV::GeoPos& pos) const;
    void gcj02_to_wgs84(QGV::GeoPos& pos) const;

    double transfrom_lat(double x, double y) const;
    double transfrom_lon(double x, double y) const;

    bool out_of_boundary(double lon, double lat) const;

private:
    double mEarthRadius;
    double mOriginShift;
    QGV::GeoRect mGeoBoundary;
    QRectF mProjBoundary;
};

#endif // QGVPROJECTIONGCJ02_H
