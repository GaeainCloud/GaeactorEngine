#ifndef FUNCTION_H
#define FUNCTION_H

#include "ProjectionEPSG3857.h"
#include "LocationHelper.h"
#include <QColor>

#include <QJsonDocument>
#include <QJsonObject>
#include <glm/vec3.hpp>
#include <QPoint>

#define EARTH_RADIUS_SHORT  (6356752.314245)

//#define EARTH_RADIUS_  (6378137.0)
#define EARTH_RADIUS_  (6371007.180918475L)
//#define EARTH_RADIUS_  (EARTH_RADIUS_SHORT)

#define M_PI 3.14159265358979323846

typedef std::vector<LAT_LNG>  LATLNGS_VECTOR;
typedef struct {
    int x;  ///< longitude in radians
    int y;  ///< latitude in radians
} Title;


extern bool operator==(const LAT_LNG& object1, const LAT_LNG& object2);
extern bool operator!=(const LAT_LNG& object1, const LAT_LNG& object2);

struct GeoTilePos
{
    int mZoom;
    Title mPos;

    bool operator==(const GeoTilePos& other) const
    {
        return mZoom == other.mZoom && mPos.x == other.mPos.x && mPos.y == other.mPos.y;
    }


    GeoTilePos()
		: mZoom(-1)
	{}

    GeoTilePos(int zoom, const QPoint& pos)
		: mZoom(zoom)
	{
		mPos.x = pos.x();
		mPos.y = pos.y();
	}
    GeoTilePos(int zoom, const Title& pos)
		: mZoom(zoom)
		, mPos(pos)
	{
	}

    GeoTilePos(const GeoTilePos& other)
		: mZoom(other.mZoom)
		, mPos(other.mPos)
	{}

    GeoTilePos(const GeoTilePos&& other)
		: mZoom(std::move(other.mZoom))
		, mPos(std::move(other.mPos))
	{}

    GeoTilePos& operator=(const GeoTilePos& other)
	{
		mZoom = other.mZoom;
		mPos = other.mPos;
		return *this;
	}

    GeoTilePos& operator=(const GeoTilePos&& other)
	{
		mZoom = std::move(other.mZoom);
		mPos = std::move(other.mPos);
		return *this;
	}

    bool operator<(const GeoTilePos& other) const
	{
		if (mZoom < other.mZoom) {
			return true;
		}
		if (mZoom > other.mZoom) {
			return false;
		}
		if (mPos.x < other.mPos.x) {
			return true;
		}
		if (mPos.x > other.mPos.x) {
			return false;
		}
		return mPos.y < other.mPos.y;
	}
	QString toQuadKey() const
	{
		const int x = mPos.x;
		const int y = mPos.y;
		QString quadKey;
		for (int i = mZoom; i > 0; i--) {
			char cDigit = '0';
			int iMask = 1 << (i - 1);
			if ((x & iMask) != 0) {
				cDigit++;
			}
			if ((y & iMask) != 0) {
				cDigit++;
				cDigit++;
			}
			quadKey.append(cDigit);
		}
		return quadKey;
	}

};

Q_DECLARE_METATYPE(GeoTilePos)

struct waypointinfo
{
    quint64 waypointid;
    double lng;
    double lat;
    double alt;
    int alttype;
    int timestamp;
};

class QOpenGLExtraFunctions;
class QIODevice;
class FunctionAssistant
{
public:
	FunctionAssistant(QOpenGLExtraFunctions *_QOpenGLExtraFunctions);
	~FunctionAssistant();
	unsigned createTexture(int width, int height, int fmt, void* data);
	unsigned createTextureFromImage(const char* fileName);
    static Mercator latLng2WebMercator(const LAT_LNG &node);
    static LAT_LNG webMercator2LatLng(Mercator mercator);
	static double  resolution(int zoom);
	static double  resolution_short(int zoom);

    static double degreeToRadian(double degree);
    static double radianToDegree(double radian);
    static LAT_LNG extendSegmentOnPlane2(const LAT_LNG& startPoint, const LAT_LNG& endPoint, float distance);
    static QColor randColor(int alpha);
    static QByteArray generate_random_64bit_id();

    static quint64 generate_random_positive_uint64();


    static QJsonDocument read_json_file(QIODevice *file);

    static QJsonDocument read_json_file(const QString &path);

    static QJsonObject read_json_file_object(const QString &path);

	static void write_json_file_object(const QString& path, const QJsonObject& obj);
    static QString json_object_to_string(QJsonObject qjo, bool compact = true);
    static QJsonObject string_to_json_object(QString in);

	static double  lonToMeter(double lon);
	static double  latToMeter(double lat);
	static int     long2tilex(double lon, int z);
	static int     lat2tiley(double lat, int z);
	static double  tilex2long(int x, int z);
	static double  tiley2lat(int y, int z);
	static Mercator tileToWorld(Title id, int z);
	static Title    getKey(unsigned level, double rLong, double rLat);
	static Title    getKeyByMeter(unsigned level, double x, double y);
    static Title geoToTilePos(int zoom, const LAT_LNG& geoPos);

	// ����ǶȵĻ��ȱ�ʾ
	static double toRadians(double degree);
	static LAT_LNG calculateDestination(const LAT_LNG& srcpt, double distance);
	static LAT_LNG calculateExtendedPoint(const LAT_LNG& srcpt, double distance, double heading);
	static LAT_LNG calculateDirectionExtendPoint(const LAT_LNG& srcpt,const glm::dvec3& directionVector, double distance);
	static LAT_LNG rotatePoint(const LAT_LNG& p, const LAT_LNG& o, double angleInDegrees);
    static double calc_dist(const double& lat1, const double& lon1, const double& lat2, const double& lon2);

    static double calc_dist(const LAT_LNG &a, const LAT_LNG& b);

	static LAT_LNG calculate_intermediate_coordinate(const LAT_LNG &a, const LAT_LNG& b);

	static LAT_LNG calculate_intermediate_coordinate_ex(const LAT_LNG &a, const LAT_LNG& b);

    static glm::dvec3 calculateVector(const LAT_LNG& p1, const LAT_LNG& p2);

	static double angle_between_vectors(const glm::dvec3& a, const glm::dvec3& b);

	static QString getMapUrl(const GeoTilePos& tile,const QString & formaturl);

	static LAT_LNG generateExtendRunWay(const LAT_LNG &start, const LAT_LNG &end, double len);


	static QJsonObject generateLineGeoJson(QJsonObject& properties, const LATLNGS_VECTOR& line);
	static QJsonObject generatePointGeoJson(QJsonObject& properties, const LAT_LNG& point);
	static QJsonObject generatePolygonGeoJson(QJsonObject& properties, const LATLNGS_VECTOR& polygon);

	static void appendfeatureline(QJsonObject& properties, QJsonArray &features, const LATLNGS_VECTOR& _runway_total);

	static void appendfeaturepoint(QJsonObject& properties, QJsonArray &features, const LAT_LNG& pt);

	static void appendfeaturepolygon(QJsonObject& properties, QJsonArray &features, const LATLNGS_VECTOR& _runway_total);


	static LATLNGS_VECTOR extendLineToPolygonPlane(const LATLNGS_VECTOR& latlnglist, float width);


private:
	QOpenGLExtraFunctions * m_QOpenGLExtraFunctions;
};


#endif // MAINWINDOW_H
