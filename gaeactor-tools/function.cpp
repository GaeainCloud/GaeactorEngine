
#include "function.h"

//#include <FreeImage/FreeImage.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#ifdef USING_QT_OPENGL_FUNCTION
#include<QOpenGLFunctions_3_3_Compatibility>
#else
#include <glad/glad.h>
#endif
#include <iostream>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>


#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <random>
#include <QOpenGLExtraFunctions>


const static    int     tileSize = 256;
const static    double  initialResolution = 2 * M_PI * EARTH_RADIUS_ / tileSize;
const static    double  originShift = 2 * M_PI * EARTH_RADIUS_ / 2.0;


std::default_random_engine rd_generator_(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count());


#define EPS7	(0.0000001)

bool operator==(const LAT_LNG& object1, const LAT_LNG& object2)
{
	if (fabs(object1.lat - object2.lat) < EPS7 && fabs(object1.lng - object2.lng) < EPS7)
	{
		return true;
	}
	return false;
}

bool operator!=(const LAT_LNG& object1, const LAT_LNG& object2)
{
	if (fabs(object1.lat - object2.lat) < EPS7 && fabs(object1.lng - object2.lng) < EPS7)
	{
		return false;
	}
	return true;
}


FunctionAssistant::FunctionAssistant(QOpenGLExtraFunctions *_QOpenGLExtraFunctions)
	:m_QOpenGLExtraFunctions(_QOpenGLExtraFunctions)
{
}

FunctionAssistant::~FunctionAssistant()
{

}


double FunctionAssistant::degreeToRadian(double degree)
{
	return degree * M_PI / 180.0;
}

double FunctionAssistant::radianToDegree(double radian)
{
	return radian * 180.0 / M_PI;
}

LAT_LNG normalizeVector(const LAT_LNG& vector)
{
	double length = std::sqrt(vector.lat * vector.lat + vector.lng * vector.lng);
	if (length > 0)
	{
		return { vector.lat / length, vector.lng / length };
	}
	else
	{
		return { vector.lat, vector.lng };
	}
}

LAT_LNG FunctionAssistant::extendSegmentOnPlane2(const LAT_LNG &startPoint, const LAT_LNG &endPoint, float distance)
{
	double lat1 = degreeToRadian(startPoint.lat);
	double lon1 = degreeToRadian(startPoint.lng);
	double lat2 = degreeToRadian(endPoint.lat);
	double lon2 = degreeToRadian(endPoint.lng);

	// calculate direction vector of the line segment
	double x = std::cos(lat2) * std::sin(lon2 - lon1);
	double y = std::cos(lat1) * std::sin(lat2) - std::sin(lat1) * std::cos(lat2) * std::cos(lon2 - lon1);

	// rotate direction vector 90 degrees to get perpendicular vector
	double perpX = -y;
	double perpY = x;

    LAT_LNG v{ perpY,perpX };

    LAT_LNG vtmp = normalizeVector(v);

	//double length = std::sqrt(diffLat * diffLat + diffLon * diffLon);

	// extend the line segment by given distance in perpendicular direction
	double extendedLon = lon2 + vtmp.lng / 50.06145 * distance / (111320.0 * std::cos(lat1));
	double extendedLat = lat2 + vtmp.lat / 50.06145 * distance / 110574.0;

	return { radianToDegree(extendedLat),radianToDegree(extendedLon) };
}


QColor FunctionAssistant::randColor(int alpha)
{
	static const int range = 255;
	return QColor(rand() % (range), rand() % (range), rand() % (range), alpha);
}
QByteArray FunctionAssistant::generate_random_64bit_id()
{
	const static QByteArray candidates = "0123456789abcdef";
	const quint32 nbits = 16;
	static std::uniform_int_distribution<quint32> u(0, nbits - 1); // uniform generation for index [0-15]
	static QByteArray buffer(nbits, 'f'); // 4bits each, 16x4bits = 64bits
	quint32 index = 0;
	for (auto j = 0; j < nbits; ++j) {
		index = u(rd_generator_); // index within 0-15
		memcpy(buffer.data() + j, candidates.data() + index, 1);
	}
	QByteArray data = QByteArray::fromHex(buffer);
	return data;
}



QJsonDocument FunctionAssistant::read_json_file(QIODevice *file)
{
	QByteArray qba = file->readAll();
	QJsonParseError error;
	QJsonDocument document = QJsonDocument::fromJson(qba, &error);
	if (error.error != QJsonParseError::NoError)
	{
		int char_offset = error.offset;
		QByteArray qba_left = qba.left(char_offset);
	}
	return document;
}

QJsonDocument FunctionAssistant::read_json_file(const QString &path)
{
	QFile file(path);
	if (!QFileInfo(path).exists()) {
	}
	if (!file.open(QIODevice::ReadOnly))
	{
	}
	QJsonDocument doc = read_json_file(&file);
	file.close();
	return doc;
}

QJsonObject FunctionAssistant::read_json_file_object(const QString &path)
{
	QJsonDocument doc = read_json_file(path);
	if (!doc.isObject())
	{
	}
	return doc.object();
}

void FunctionAssistant::write_json_file_object(const QString& path, const QJsonObject& obj)
{
	QJsonDocument doc;
	doc.setObject(obj);
	QFile file(path);
	if (file.open(QFile::WriteOnly))
	{
		file.write(doc.toJson());
		file.flush();
	}
}

QString FunctionAssistant::json_object_to_string(QJsonObject qjo, bool compact)
{
	QJsonDocument doc(qjo);
	if (compact) {
		return QString(doc.toJson(QJsonDocument::Compact));
	}
	else {
		return QString(doc.toJson(QJsonDocument::Indented));
	}
}

QJsonObject FunctionAssistant::string_to_json_object(QString in)
{
	QJsonObject obj;
	QJsonDocument doc = QJsonDocument::fromJson(in.toUtf8());

	// check validity of the document
	if (!doc.isNull())
	{
		if (doc.isObject())
		{
			obj = doc.object();
		}
		else
		{
			return obj;
		}
	}
	else
	{
		return obj;
	}
	return obj;
}


quint64 FunctionAssistant::generate_random_positive_uint64()
{
	const auto data = generate_random_64bit_id();
	quint64 result = 0;
	memcpy(&result, data.data(), data.size());
	return std::move(result);
}
unsigned FunctionAssistant::createTexture(int width, int height, int fmt, void* data)
{
	unsigned tex = 0;
	m_QOpenGLExtraFunctions->glGenTextures(1, &tex);
	m_QOpenGLExtraFunctions->glBindTexture(GL_TEXTURE_2D, tex);
	m_QOpenGLExtraFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_QOpenGLExtraFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_QOpenGLExtraFunctions->glTexImage2D(
		GL_TEXTURE_2D,      //! 指定是二维图片
		0,                  //! 指定为第一级别，纹理可以做mipmap,即lod,离近的就采用级别大的，远则使用较小的纹理
		fmt,                //! 纹理的使用的存储格式
		width,
		height,
		0,                  //! 是否的边
		fmt,                //! 数据的格式，bmp中，windows,操作系统中存储的数据是bgr格式
		GL_UNSIGNED_BYTE,   //! 数据是8bit数据
		data
	);
	return  tex;
}

unsigned FunctionAssistant::createTextureFromImage(const char* fileName)
{
	int width, height, nrChannels;
	unsigned char *pixels = stbi_load(fileName, &width, &height, &nrChannels, 0);
	unsigned    res = 0;
	if (pixels)
	{
		res = createTexture(width, height, GL_RGBA, pixels);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(pixels);

	return  res;
}

Mercator FunctionAssistant::latLng2WebMercator(const LAT_LNG &node)
{
	Mercator mercator;
	mercator.x = node.lng * originShift / 180;
	mercator.y = -log(tan((90 + node.lat)*M_PI / 360)) / (M_PI / 180);
	mercator.y = mercator.y * originShift / 180;

	return mercator; //[12727039.383734727, 3579066.6894065146]
}

LAT_LNG FunctionAssistant::webMercator2LatLng(Mercator mercator)
{
    LAT_LNG node;
	node.lng = mercator.x / originShift * 180;
	node.lat = -mercator.y / originShift * 180;
	node.lat = 180 / M_PI * (2 * atan(exp(node.lat * M_PI / 180)) - M_PI / 2);
	return node; //[114.32894001591471, 30.58574800385281]
}


double  FunctionAssistant::lonToMeter(double lon)
{
	return  lon * originShift / 180.0;
}
double  FunctionAssistant::latToMeter(double lat)
{
	double  my = log(tan((90 + lat) *M_PI / 360.0)) / (M_PI / 180.0);
	return  my = my * originShift / 180.0;
}

double  FunctionAssistant::resolution(int zoom)
{
	//  return (2 * math.pi * EARTH_RADIUS_) / (self.tileSize * 2**zoom)
	return  initialResolution / (pow(2, double(zoom)));
}

double  FunctionAssistant::resolution_short(int zoom)
{
	//  return (2 * math.pi * EARTH_RADIUS_) / (self.tileSize * 2**zoom)
	return  (2 * M_PI * EARTH_RADIUS_ / tileSize) / (pow(2, double(zoom)));
}


int     FunctionAssistant::long2tilex(double lon, int z)
{
	return (int)(floor((lon + 180.0) / 360.0 * pow(2.0, z)));
}

int     FunctionAssistant::lat2tiley(double lat, int z)
{
	return  (int)(floor((1.0 - log(tan(lat * M_PI / 180.0) + 1.0 / cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * pow(2.0, z)));
}

double  FunctionAssistant::tilex2long(int x, int z)
{
	return x / pow(2.0, z) * 360.0 - 180;
}

double  FunctionAssistant::tiley2lat(int y, int z)
{
	double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

Mercator FunctionAssistant::tileToWorld(Title id, int z)
{
	double  dLong = tilex2long(id.x, z);
	double  dLat = tiley2lat(id.y, z);

    return  latLng2WebMercator(LAT_LNG{ dLat, dLong });
}

Title FunctionAssistant::getKey(unsigned level, double rLong, double rLat)
{
	int     xTile = long2tilex(rLong, level);
	int     yTile = lat2tiley(rLat, level);
	return  Title{ xTile, yTile };
}

Title FunctionAssistant::getKeyByMeter(unsigned level, double x, double y)
{
    LAT_LNG lonLat = webMercator2LatLng(Mercator{ x, y });
	int     xTile = long2tilex(lonLat.lng, level);
	int     yTile = lat2tiley(lonLat.lat, level);
	return  Title{ xTile, yTile };
}

Title FunctionAssistant::geoToTilePos(int zoom, const LAT_LNG &geoPos)
{
	const double lon = geoPos.lng;
	const double lat = geoPos.lat;
	int     xTile = long2tilex(lon, zoom);
	int     yTile = lat2tiley(lat, zoom);
	return  Title{ xTile, yTile };
	//const double x = floor((lon + 180.0) / 360.0 * pow(2.0, zoom));
	//const double y =
	//	floor((1.0 - log(tan(lat * M_PI / 180.0) + 1.0 / cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * pow(2.0, zoom));
	//return Title{ static_cast<int>(x), static_cast<int>(y) };
}

double FunctionAssistant::toRadians(double degree)
{
	return degree * M_PI / 180.0;
}

LAT_LNG FunctionAssistant::calculateDestination(const LAT_LNG& srcpt, double distance)
{
	double angularDistance = distance / (EARTH_RADIUS_ / 1000.0f); // 角距离，单位为弧度

// 将纬度和经度转换为弧度表示
	double radianLat = glm::radians(srcpt.lat);
	double radianLon = glm::radians(srcpt.lng);

	// 根据正算公式计算新的纬度和经度
	double newLat = asin(sin(radianLat) * cos(angularDistance) + cos(radianLat) * sin(angularDistance) * cos(0));
	double newLon = radianLon + atan2(sin(0) * sin(angularDistance) * cos(radianLat), cos(angularDistance) - sin(radianLat) * sin(newLat));

	// 将弧度表示转换为角度表示
	newLat = newLat * 180.0 / M_PI;
	newLon = newLon * 180.0 / M_PI;
    return LAT_LNG{ newLat , newLon };
}

// 计算扩展点的经纬度
LAT_LNG FunctionAssistant::calculateExtendedPoint(const LAT_LNG& srcpt, double distance, double heading)
{
	// 将经纬度转换为弧度
	double lonRad = glm::radians(srcpt.lng);
	double latRad = glm::radians(srcpt.lat);
	double headingRad = glm::radians(heading);

	// 计算扩展点的纬度
	double extendedLatRad = asin(sin(latRad) * cos(distance) + cos(latRad) * sin(distance) * cos(headingRad));

	// 计算扩展点的经度
	double y = sin(headingRad) * sin(distance) * cos(latRad);
	double x = cos(distance) - sin(latRad) * sin(extendedLatRad);
	double extendedLonRad = lonRad + atan2(y, x);

	// 将经纬度转换回度数
	double extendedLongitude = glm::degrees(extendedLonRad);
	double extendedLatitude = glm::degrees(extendedLatRad);
    return LAT_LNG{ extendedLatitude ,extendedLongitude };
}

LAT_LNG FunctionAssistant::calculateDirectionExtendPoint(const LAT_LNG& srcpt, const glm::dvec3& directionVector, double distance)
{
	// 将经纬度转换为三维坐标系中的点
	auto toCartesian = [](double longitude, double latitude, double radius)->glm::dvec3 {
		double x = radius * cos(glm::radians(latitude)) * cos(glm::radians(longitude));
		double y = radius * cos(glm::radians(latitude)) * sin(glm::radians(longitude));
		double z = radius * sin(glm::radians(latitude));
		return glm::dvec3(x, y, z);
	};

	// 将三维坐标系中的点转换为经纬度
    auto toGeographic = [](const glm::dvec3& cartesian)->LAT_LNG {
		double longitude = glm::degrees(atan2(cartesian.y, cartesian.x));
		double latitude = glm::degrees(asin(cartesian.z / glm::length(cartesian)));
        return LAT_LNG{ latitude,longitude };
	};

	double longitude = srcpt.lng; // 经度
	double latitude = srcpt.lat; // 纬度
	double radius = EARTH_RADIUS_ / 1000; // 地球半径，单位为公里
	double distancekm = distance / 1000.0f; // 扩展距离，单位为公里
	double heading = 45.0f; // 航向角，单位为度

	// 将经纬度转换为笛卡尔坐标系中的三维点
	glm::dvec3 startPoint = toCartesian(longitude, latitude, radius);

	// 将扩展向量转换为笛卡尔坐标系中的三维向量
	//glm::vec3 directionVector = glm::rotateZ(glm::rotateY(glm::vec3(1, 0, 0), glm::radians(90.0f - latitude)), glm::radians(-longitude));

	// 辅助函数，将 glm::vec3 与 double 相乘
	auto multiply = [](const glm::dvec3& vector, double scalar)->glm::dvec3 {
		return glm::dvec3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
	};

	glm::dvec3  extenddirectionVector = multiply(-directionVector, (distancekm / glm::length(directionVector)));

	// 计算扩展点的笛卡尔坐标系中的坐标
	glm::dvec3 extendedPointCartesian = startPoint + extenddirectionVector;

	// 将扩展点的笛卡尔坐标系中的坐标转换为经纬度
    LAT_LNG extendedPointGeographic = toGeographic(extendedPointCartesian);

	return extendedPointGeographic;
	//std::cout << "Extended point: (" << extendedPointGeographic.x << ", " << extendedPointGeographic.y << ")" << std::endl;
}

LAT_LNG FunctionAssistant::rotatePoint(const LAT_LNG& p, const LAT_LNG& o, double angleInDegrees) {
	double angleInRadians = angleInDegrees * M_PI / 180.0;
	double x1 = p.lng - o.lng;
	double y1 = p.lat - o.lat;
	double x2 = x1 * cos(angleInRadians) - y1 * sin(angleInRadians);
	double y2 = x1 * sin(angleInRadians) + y1 * cos(angleInRadians);
	return { y2 + o.lat, x2 + o.lng };
}


/* 由经纬度计算两点距离，抄的googleMap */
double FunctionAssistant::calc_dist(const double &lat1, const double &lon1, const double &lat2, const double &lon2) {
	double radLat1 = toRadians(lat1);
	double radLat2 = toRadians(lat2);
	double a = radLat1 - radLat2;
	double b = toRadians(lon1) - toRadians(lon2);
	double s = 2 * asin(sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2)));
	s = s * EARTH_RADIUS_;
	s = round(s * 10000) / 10000;
	return s;

	//    double earthRadius = 6371.0; // 地球半径，单位为千米

	//    // 将经纬度转换为弧度
	//    double dLat = toRadians(lat2 - lat1);
	//    double dLon = toRadians(lon2 - lon1);

	//    double a = sin(dLat/2) * sin(dLat/2) +
	//               cos(toRadians(lat1)) * cos(toRadians(lat2)) *
	//                   sin(dLon/2) * sin(dLon/2);
	//    double c = 2 * atan2(sqrt(a), sqrt(1-a));
	//    double distance = earthRadius * c;

	//    return distance;
}

double FunctionAssistant::calc_dist(const LAT_LNG &a, const LAT_LNG &b)
{
	return calc_dist(a.lat, a.lng, b.lat, b.lng);
}



LAT_LNG FunctionAssistant::calculate_intermediate_coordinate(const LAT_LNG &a, const LAT_LNG& b)
{
	// 将角度转换为弧度
	double rad_lat1 = glm::radians(a.lat);
	double rad_lon1 = glm::radians(a.lng);
	double rad_lat2 = glm::radians(b.lat);
	double rad_lon2 = glm::radians(b.lng);

	// 中间经度
	double lon_intermediate = rad_lon1 + (rad_lon2 - rad_lon1) / 2;

	// 使用平均纬度计算中间纬度
	double x = cos(rad_lat2) * cos(rad_lon2 - rad_lon1);
	double y = cos(rad_lat2) * sin(rad_lon2 - rad_lon1);
	double lat_intermediate = atan2(sin(rad_lat1) + sin(rad_lat2), sqrt((cos(rad_lat1) + x) * (cos(rad_lat1) + x) + y * y));

	// 将弧度转换为角度
    return LAT_LNG{ glm::degrees(lat_intermediate),glm::degrees(lon_intermediate) };
}

LAT_LNG FunctionAssistant::calculate_intermediate_coordinate_ex(const LAT_LNG &a, const LAT_LNG& b)
{
	glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(a, b);
	double dis = FunctionAssistant::calc_dist(a, b);
    LAT_LNG currentextendpt = FunctionAssistant::calculateDirectionExtendPoint(a, directionVectorArr, dis / 2);
	return currentextendpt;
}

glm::dvec3 FunctionAssistant::calculateVector(const LAT_LNG& p1, const LAT_LNG& p2) {

	double R = EARTH_RADIUS_ / 1000; // 地球平均半径，单位：公里

	double lat1 = toRadians(p1.lat);
	double lon1 = toRadians(p1.lng);
	double lat2 = toRadians(p2.lat);
	double lon2 = toRadians(p2.lng);

	double x1 = R * cos(lat1) * cos(lon1);
	double y1 = R * cos(lat1) * sin(lon1);
	double z1 = R * sin(lat1);

	double x2 = R * cos(lat2) * cos(lon2);
	double y2 = R * cos(lat2) * sin(lon2);
	double z2 = R * sin(lat2);

	glm::dvec3 vec;
	vec.x = x1 - x2;
	vec.y = y1 - y2;
	vec.z = z1 - z2;

	return vec;
}

double FunctionAssistant::angle_between_vectors(const glm::dvec3& a, const glm::dvec3& b)
{
	auto dot_product = [](double x1, double y1, double z1, double x2, double y2, double z2) ->double {
		return x1 * x2 + y1 * y2 + z1 * z2;
	};

	auto vector_length = [](double x, double y, double z)->double {
		return sqrt(x * x + y * y + z * z);
	};
	double dot = dot_product(a[0], a[1], a[2], b[0], b[1], b[2]);
	double length1 = vector_length(a[0], a[1], a[2]);
	double length2 = vector_length(b[0], b[1], b[2]);
	double cos_angle = dot / (length1 * length2);
	double angle = acos(cos_angle);
	return angle;
}

QString FunctionAssistant::getMapUrl(const GeoTilePos& tile, const QString & formaturl)
{
	QString requesturl = formaturl;

	QLocale mLocale = QLocale();
	if (requesturl.contains("{lcl}"))
	{
		requesturl.replace("{lcl}", mLocale.name());
	}
	if (requesturl.contains("{qk}"))
	{
		requesturl.replace("{qk}", tile.toQuadKey());
	}

	if (requesturl.contains("{z}"))
	{
		requesturl.replace("{z}", QString::number(tile.mZoom));
	}

	if (requesturl.contains("{x}"))
	{
		requesturl.replace("{x}", QString::number(tile.mPos.x));
	}

	if (requesturl.contains("{y}"))
	{
		requesturl.replace("{y}", QString::number(tile.mPos.y));
	}

	return requesturl;
}

LAT_LNG FunctionAssistant::generateExtendRunWay(const LAT_LNG &start, const LAT_LNG &end, double len)
{
	glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(start, end);
    LAT_LNG extendpt = FunctionAssistant::calculateDirectionExtendPoint(end, directionVectorArr, len);
	return extendpt;
};



QJsonObject FunctionAssistant::generateLineGeoJson(QJsonObject& properties, const LATLNGS_VECTOR& line)
{
	QJsonObject geojson;
	geojson.insert("type", "FeatureCollection");
	QJsonArray features;
	appendfeatureline(properties, features, line);
	geojson.insert("features", features);
	//qDebug() << geojson;
	return geojson;
}
QJsonObject FunctionAssistant::generatePointGeoJson(QJsonObject& properties, const LAT_LNG& point)
{
	QJsonObject geojson;
	geojson.insert("type", "FeatureCollection");
	QJsonArray features;
	appendfeaturepoint(properties, features, point);
	geojson.insert("features", features);
	//qDebug() << geojson;
	return geojson;
}
QJsonObject FunctionAssistant::generatePolygonGeoJson(QJsonObject& properties, const LATLNGS_VECTOR& polygon)
{
	QJsonObject geojson;
	geojson.insert("type", "FeatureCollection");
	QJsonArray features;
	appendfeaturepolygon(properties, features, LATLNGS_VECTOR());
	geojson.insert("features", features);
	//qDebug() << geojson;
	return geojson;
}


void FunctionAssistant::appendfeatureline(QJsonObject& properties, QJsonArray &features, const LATLNGS_VECTOR& _runway_total)
{
	QJsonObject featureitem;
	featureitem.insert("type", "Feature");
	featureitem.insert("properties", properties);
	QJsonObject geometry;
	geometry.insert("type", "LineString");
	QJsonArray coordinates;
	for (auto item : _runway_total)
	{
		QJsonArray coordinateitem;
		coordinateitem.append(item.lng);
		coordinateitem.append(item.lat);
		coordinates.append(coordinateitem);
	}
	geometry.insert("coordinates", coordinates);
	featureitem.insert("geometry", geometry);

	features.append(featureitem);
}

void FunctionAssistant::appendfeaturepoint(QJsonObject& properties, QJsonArray &features, const LAT_LNG& pt)
{
	QJsonObject featureitem;
	featureitem.insert("type", "Feature");
	featureitem.insert("properties", properties);
	QJsonObject geometry;
	geometry.insert("type", "Point");
	QJsonArray coordinates;
	coordinates.append(pt.lng);
	coordinates.append(pt.lat);
	geometry.insert("coordinates", coordinates);
	featureitem.insert("geometry", geometry);
	features.append(featureitem);
}

void FunctionAssistant::appendfeaturepolygon(QJsonObject& properties, QJsonArray &features, const LATLNGS_VECTOR& _runway_total)
{
	QJsonObject featureitem;
	featureitem.insert("type", "Feature");
	featureitem.insert("properties", properties);
	QJsonObject geometry;
	geometry.insert("type", "Polygon");
	QJsonArray coordinates;
	for (auto item : _runway_total)
	{
		QJsonArray coordinateitem;
		coordinateitem.append(item.lng);
		coordinateitem.append(item.lat);
		coordinates.append(coordinateitem);
	}
	geometry.insert("coordinates", coordinates);
	featureitem.insert("geometry", geometry);

	features.append(featureitem);
}

LATLNGS_VECTOR FunctionAssistant::extendLineToPolygonPlane(const LATLNGS_VECTOR& latlnglist, float width)
{
	LATLNGS_VECTOR extentmp;
	int extendcount = latlnglist.size() * 2;
	extentmp.resize(extendcount + 1);
	for (int index = 0; index < latlnglist.size(); index++)
	{
        LAT_LNG startPoint;
        LAT_LNG endPoint;
		if (index == 0)
		{
			startPoint = latlnglist.at(index + 1);
			endPoint = latlnglist.at(index);
		}
		else
		{
			startPoint = latlnglist.at(index - 1);
			endPoint = latlnglist.at(index);
		}
        LAT_LNG extendedPoint = FunctionAssistant::extendSegmentOnPlane2(startPoint, endPoint, width);
        LAT_LNG extendedPoint2 = FunctionAssistant::extendSegmentOnPlane2(startPoint, endPoint, -width);

		if (index == 0)
		{
			extentmp[index] = extendedPoint2;
			extentmp[(extendcount - 1) - index] = extendedPoint;
		}
		else
		{
			extentmp[index] = extendedPoint;
			extentmp[(extendcount - 1) - index] = extendedPoint2;
		}
	}
	extentmp[extendcount] = extentmp[0];

	return extentmp;
}
