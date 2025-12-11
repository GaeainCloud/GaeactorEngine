#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QtMath>
#include <QDateTime>

#include <QDir>
#include <QFileInfoList>
#include <random>
#include "components/function.h"

#include <ogr_geometry.h>
#define EXTEND_POINTS

#define INTERVAL_MS (1000)
#include "widget3d/QtOsgWidget.h"
#include "widget3d/OSGManager.h"
#include "widget3d/ModelSceneData.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->resize(1400,1200);

//    QString path = QCoreApplication::applicationDirPath() + "/res/model/A350_1000.fbx";
//    QString path = "D:/model/model/su.fbx";
//    QString path = "D:/model/model/shandong_navy.fbx";
//    QString path = "D:/model/model/type_094.fbx";
    QString path = "D:/model/model/OK/A380/ive/A380-AIRFRANCE.ive";

#if 0
    m_pModelWidget = new QtOSGWidget(QtOSGWidget::E_OSG_SHOW_TYPE_MODEL, this);
    this->setCentralWidget(m_pModelWidget);
    m_pModelWidget->pOSGManager()->addModelNode(path,QModelIndex(),true);
#else
    m_pModelWidget = new QtOSGWidget(QtOSGWidget::E_OSG_SHOW_TYPE_MAP, this);
    this->setCentralWidget(m_pModelWidget);

    lon = 103.94037755013733;
    lat = 30.56454233609911;
    hgt = 600;


    m_pModelWidget->updateViewPoint(lon, lat);
    m_ptimer = new QTimer(this);



    // 计算方位角函数
    auto calculateBearing=[](double lat1, double lon1, double lat2, double lon2)->double{
        double phi1 = (lat1)* M_PI / 180.0;
        double phi2 = (lat2)* M_PI / 180.0;
        double lambda1 = (lon1)* M_PI / 180.0;
        double lambda2 = (lon2)* M_PI / 180.0;

        double deltaLambda = lambda2 - lambda1;

        // 使用 atan2 计算反正切，并将结果转换为角度
        double y = sin(deltaLambda) * cos(phi2);
        double x = cos(phi1) * sin(phi2) - sin(phi1) * cos(phi2) * cos(deltaLambda);
        double bearing = atan2(y, x);

        // 将弧度转换为角度
        bearing = bearing * 180.0 / M_PI;

        // 将角度转换为0到360度的范围
        if (bearing < 0) {
            bearing += 360.0;
        }

        return bearing;
    };



    // 计算方位角函数
    auto calculatePitch =[](double h2, double h1, double d)->double{
        // 计算高度差
        double height_diff = h1- h2;

        // 计算俯仰角（弧度）
        double theta = atan(height_diff / d);

        // 将弧度转换为度数
        theta = theta * (180 / M_PI);

        return theta;
    };

    // 将度转换为弧度
    auto degreesToRadians=[&](double degrees)->double
    {
        return degrees * M_PI / 180.0;
    };

    // Haversine公式计算两点之间的距离
    auto haversineDistance=[&](double lat1, double lon1, double lat2, double lon2) ->double{
        double R = 6371000; // 地球半径，米
        double dLat = degreesToRadians(lat2 - lat1);
        double dLon = degreesToRadians(lon2 - lon1);
        double a = sin(dLat / 2) * sin(dLat / 2) +
                   cos(degreesToRadians(lat1)) * cos(degreesToRadians(lat2)) *
                       sin(dLon / 2) * sin(dLon / 2);
        double c = 2 * atan2(sqrt(a), sqrt(1 - a));
        return R * c;
    };


    bool bAdd = true;
    yaw = 0;
    pitch = 0;
    roll = 0;


    //        double lon = 113.16472931376614;
    //        double lat = 22.23526795474291;

//            double lon = 106.584814;
//            double lat = 26.6861167;

    //double lon = 103.9742826951358;
    //double lat = 30.613675975314578;


    double elevation = m_pModelWidget->getPositionElevation(lon, lat);
    std::cout<<" msl "<<elevation<<"\n";

//    int id = 0;
//    std::function<void (const QString & ,const QString&, int index)> appendModel=[&](const QString& path,const QString& filename, int index)
//    {
//        m_pModelWidget->initNode(agentid+index,filename,0,path, lon+0.03*index,lat,hgt*10,roll,pitch,yaw,1);
////        m_pModelWidget->visibleEntityTracking(agentid+index,false,(UINT64)QtOSGWidget::E_VISIABLE_ATTRIBUTE_TAIL);
//        m_pModelWidget->visibleEntityTracking(agentid+index,false,(UINT64)QtOSGWidget::E_VISIABLE_ATTRIBUTE_LABEL |
//                                                                  (UINT64)QtOSGWidget::E_VISIABLE_ATTRIBUTE_TAIL);
//    };



//    std::function<void (const QString &)> findTrajectoryFolders=[&](const QString &path)
//    {
//        QDir dir(path);

//        // 检查路径是否存在且是一个目录
//        if (!dir.exists()) {
//            qDebug() << "提供的路径不是一个有效的目录";
//            return;
//        }

//        // 获取目录中的所有条目
//        QFileInfoList entries ;
//        entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
//        for (const QFileInfo &entry : entries)
//        {
//            if (entry.isDir())
//            {
//                QString filenamepath = entry.absolutePath();
//                findTrajectoryFolders(entry.absoluteFilePath());
////                std::cout<<" find model "<<filenamepath.toStdString()<<"\n";
//            }
//            else
//            {
//                QString filenamepath = entry.absoluteFilePath();
//                QString filename = entry.fileName();
//                if(/*filenamepath.endsWith(".fbx")|| */ filenamepath.endsWith(".ive"))
//                {
//                    std::cout<<" append model "<<filenamepath.toStdString()<<"\n";
//                    appendModel(filenamepath,filename,id++);
//                }
//            }
//        }
//    };
//    findTrajectoryFolders("D:/model/model/PARASE");
#if 1
#if 0
    path="C:/Users/lj/Downloads/ImageToStl.com_dcc388cf-5e10-4a6f-b8ef-d4113697e72bc/ImageToStl.com_dcc388cf-5e10-4a6f-b8ef-d4113697e72bc.obj";
    m_pModelWidget->initNode(agentid,"",0,path, lon,lat,hgt*10,roll,pitch,yaw,100);
    m_pModelWidget->updateEntityTracking(agentid, lon,lat,hgt,roll,pitch,yaw);
    m_pModelWidget->visibleEntityTracking(agentid,false,(UINT64)QtOSGWidget::E_VISIABLE_ATTRIBUTE_LABEL |
                                                              (UINT64)QtOSGWidget::E_VISIABLE_ATTRIBUTE_TAIL);
    connect(m_ptimer, &QTimer::timeout,[&]()
            {
                double oldlon = lon;
                double oldlat = lat;
                double oldhgt = hgt;
                lon += 0.005;
                if(hgt>10000)
                {
                    bAdd = false;
                }
                if(hgt<2500)
                {
                    bAdd = true;
                }

                if(bAdd)
                {
                    hgt += 100;
                }
                else
                {
                    hgt -= 100;
                }
//                double dist = haversineDistance(oldlat,oldlon,lat,lon);
//                pitch = calculatePitch(oldhgt, hgt, dist);
                yaw = calculateBearing(oldlat,oldlon,lat,lon);

//                pitch +=1;
//                if(pitch>360)
//                {
//                    pitch = 0.0;
//                }

//                yaw +=1;
//                if(yaw>360)
//                {
//                    yaw = 0.0;
//                }

//                roll +=1;
//                if(roll>360)
//                {
//                    roll = 0.0;
//                }

                m_pModelWidget->updateEntityTracking(agentid, lon,lat,hgt,roll,pitch,yaw);
            });



    m_ptimer->start(100);


#endif

#if 1
    std::function<void (const QString & ,bool)> findTrajectoryFolders=[&](const QString &path,bool bTrajectoryDir)
    {
        QDir dir(path);

        // 检查路径是否存在且是一个目录
        if (!dir.exists()) {
            qDebug() << "提供的路径不是一个有效的目录";
            return;
        }

        // 获取目录中的所有条目
        QFileInfoList entries ;
        entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
        for (const QFileInfo &entry : entries)
        {
            if (entry.isDir())
            {
                if (entry.fileName() == "sm3Agent")
                {
                    findTrajectoryFolders(entry.absoluteFilePath(),true);
                }
                else
                {
                    findTrajectoryFolders(entry.absoluteFilePath(),false);
                }
            }
            else
            {
                std::vector<std::tuple<double, double, double> > poslist;
                QStringList pathParts = dir.absolutePath().split("/");
                QString flightcode = pathParts.at(pathParts.size()-2);
                QFile file(entry.absoluteFilePath());
                UINT64 agentiid_ = 0;
                std::cout.precision(17);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    QTextStream in(&file);
                    if(!in.atEnd())
                    {
                        in.readLine();
                    }
                    while (!in.atEnd())
                    {
                        QString line = in.readLine();
                        QStringList fields = line.split(QRegExp("\\s+")/*, QString::SkipEmptyParts*/);
                        if(fields.size() == 7)
                        {
                            agentiid_ = fields[2].toULongLong();
                            auto lon = fields[4].toDouble();
                            auto lat = fields[5].toDouble();
                            auto lgt = fields[6].toDouble();

                            poslist.push_back(std::make_tuple(
                                lon,
                                lat,
                                lgt));
                        }
                    }
                }

                if(bTrajectoryDir)
                {
                    m_pModelWidget->initNode(agentiid_,"",1, "", lon,lat,hgt,roll,pitch,yaw,1.0f, QColor(0,0,255,255),2);
                }
                else
                {
                    m_pModelWidget->initNode(agentiid_,"",1, "", lon,lat,hgt,roll,pitch,yaw,1.0f, QColor(255,0,0,255),2);
                }

                qDebug() << "找到 "<<agentiid_<<" size "<<poslist.size();
                m_pModelWidget->updatePosList(agentiid_,poslist);
            }
        }
    };

    findTrajectoryFolders("D:/project/WorkProject_20240402001152/h5parse/output",false);
#endif

#if 0
    auto generateRandomNumber=[]()->int {
        // 创建随机数生成器
        std::random_device rd;
        std::mt19937 gen(rd());

        // 创建一个在0到255之间的均匀分布
        std::uniform_int_distribution<> dis(0, 255);

        // 生成随机数
        return dis(gen);
    };


    int agentid_tmp = 0;

    std::function<void (const QString &)> findShpFolders=[&](const QString &path)
    {
        QDir dir(path);

        // 检查路径是否存在且是一个目录
        if (!dir.exists()) {
            qDebug() << "提供的路径不是一个有效的目录";
            return;
        }

        // 获取目录中的所有条目
        QFileInfoList entries ;
        entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
        for (const QFileInfo &entry : entries)
        {
            if (entry.isDir())
            {
                findShpFolders(entry.absoluteFilePath());
            }
            else
            {
                if(entry.suffix() == "shp")
                {
                    m_pModelWidget->initNode(agentid_tmp,"",4, entry.absoluteFilePath(), lon,lat,hgt,roll,pitch,yaw,1.0f, QColor(generateRandomNumber(),generateRandomNumber(),generateRandomNumber(),255),QColor(generateRandomNumber(),generateRandomNumber(),generateRandomNumber(),32),2);
                    agentid_tmp++;
                }
            }
        }
    };

    QString shppath = "D:/earth/gzsczt_fwjz_pro_cgcs2000/gz_50";
    findShpFolders(shppath);

#endif


#if 0


    // 创建一个线环（LinearRing）
//    OGRLinearRing ring;
//    ring.addPoint(        113.26587006121008,
//                  23.505144318180797);
//    ring.addPoint(113.01806999650262,
//                  23.369055682729552);
//    ring.addPoint(113.15610439139448,
//                  23.251034094572134);
//    ring.addPoint(113.52796531494772,
//                  23.3485537042406);
//    ring.addPoint(113.5532266556649,
//                  23.37416307514262);
//    ring.addPoint(113.26587006121008,
//                  23.505144318180797);


//    // 创建一个线环（LinearRing）
//    OGRLinearRing ring;
//    ring.addPoint(113.28807582426617,
//                  23.398118198725243);
//    ring.addPoint(113.29631739767188,
//                  23.390241324007945);
//    ring.addPoint(113.30170850217894,
//                  23.39811819878912);
//    ring.addPoint(113.29715394837024,
//                  23.404117958705044);

//    ring.addPoint(113.31078662643665,
//                  23.398288811831875);
//    ring.addPoint(113.32287013654076,
//                  23.390525695868746);
//    ring.addPoint(113.32730075691154,
//                  23.39956840264277);
//    ring.addPoint(113.32262226966685,
//                  23.406620147894444);
//    ring.addPoint(113.31078662643665,
//                  23.398288811831875);

//    ring.addPoint(113.28807582426617,
//                  23.398118198725243);

//    // 创建一个多边形（Polygon）
//    OGRPolygon polygon;
//    polygon.addRing(&ring);

//    // 计算凸包

    float modelsize = 110000.0f;
    QColor lineColor = QColor(255,255,0,128);
    QColor fillColor = QColor(255,255,0,128);


//    m_pModelWidget->initNode(agentid+2,"",8, "", lon,lat,5000,roll,pitch,yaw,5000,lineColor,fillColor);

//    m_pModelWidget->initNode(agentid+3,"",9, "", lon,lat,5000,roll,pitch,yaw,110000.0f,lineColor,fillColor);

//    m_pModelWidget->initNode(agentid,"",7, "", lon,lat,5000,roll,pitch,yaw,modelsize,lineColor,fillColor);

    lon = 104.40838779319563;
    lat = 30.880514717765323;
    double le_r = 10000;

    m_pModelWidget->initNode(agentid+2,"",10, "", lon,lat,0,roll,pitch,yaw,5000,lineColor,fillColor);

    std::vector<std::tuple<double, double, double> > poslist;
        std::vector<std::tuple<QColor,QColor>> cls;

    LAT_LNG srcpt{30.56454233609911,103.94037755013733};
    LAT_LNG dstpt{30.56454233609911,103.94037855013733};
    LAT_LNG dstpt2{30.56454253609911,103.94037755013733};
    for(int j = 0;j < 10;j++)
    {
        poslist.push_back(std::make_tuple(srcpt.lng,srcpt.lat,  le_r*j));

        cls.push_back(std::make_tuple(FunctionAssistant::randColor(128),FunctionAssistant::randColor(128)));
    }
    glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(srcpt, dstpt);
    for(int j = 0;j < 50;j++)
    {
        LAT_LNG ret = FunctionAssistant::calculateDirectionExtendPoint(srcpt,directionVectorArr,le_r*(j+1));
        poslist.push_back(std::make_tuple(ret.lng,ret.lat, le_r));
        cls.push_back(std::make_tuple(FunctionAssistant::randColor(128),FunctionAssistant::randColor(128)));
        for(int j = 0;j < 50;j++)
        {
            poslist.push_back(std::make_tuple(ret.lng,ret.lat,  le_r*j));
            cls.push_back(std::make_tuple(FunctionAssistant::randColor(128),FunctionAssistant::randColor(128)));
        }
    }

    for(int j = 0;j < 50;j++)
    {
        LAT_LNG ret = FunctionAssistant::calculateDirectionExtendPoint(srcpt,-directionVectorArr,le_r*(j+1));
        poslist.push_back(std::make_tuple(ret.lng,ret.lat, le_r));
        cls.push_back(std::make_tuple(FunctionAssistant::randColor(128),FunctionAssistant::randColor(128)));
        for(int j = 0;j < 50;j++)
        {
            poslist.push_back(std::make_tuple(ret.lng,ret.lat,  le_r*j));
            cls.push_back(std::make_tuple(FunctionAssistant::randColor(128),FunctionAssistant::randColor(128)));
        }
    }

    glm::dvec3 directionVectorArr2 = FunctionAssistant::calculateVector(srcpt, dstpt2);
    for(int j = 0;j < 50;j++)
    {
        LAT_LNG ret = FunctionAssistant::calculateDirectionExtendPoint(srcpt,directionVectorArr2,le_r*(j+1));
        poslist.push_back(std::make_tuple(ret.lng,ret.lat, le_r));
        cls.push_back(std::make_tuple(FunctionAssistant::randColor(128),FunctionAssistant::randColor(128)));
        for(int j = 0;j < 50;j++)
        {
            poslist.push_back(std::make_tuple(ret.lng,ret.lat,  le_r*j));
            cls.push_back(std::make_tuple(FunctionAssistant::randColor(128),FunctionAssistant::randColor(128)));
        }
    }

    for(int j = 0;j < 50;j++)
    {
        LAT_LNG ret = FunctionAssistant::calculateDirectionExtendPoint(srcpt,-directionVectorArr2,le_r*(j+1));
        poslist.push_back(std::make_tuple(ret.lng,ret.lat, le_r));
        cls.push_back(std::make_tuple(FunctionAssistant::randColor(128),FunctionAssistant::randColor(128)));
        for(int j = 0;j < 50;j++)
        {
            poslist.push_back(std::make_tuple(ret.lng,ret.lat,  le_r*j));
            cls.push_back(std::make_tuple(FunctionAssistant::randColor(128),FunctionAssistant::randColor(128)));
        }
    }

    std::vector<double> llaList_down;
    llaList_down.push_back(le_r/2);
    llaList_down.push_back(3);
//    std::vector<std::tuple<QColor,QColor>> cls;
//    cls.push_back(std::make_tuple(QColor(0,0,255,128),QColor(0,255,255,255)));
    m_pModelWidget->updatePosList(agentid+2,poslist,std::move(llaList_down),cls);

//    llaList_down.push_back(50000);
//    llaList_down.push_back(50000);
//    llaList_down.push_back(500000);
//    cls.push_back(std::make_tuple(QColor(0,0,255,128),QColor(0,255,255,255)));
//    m_pModelWidget->updatePosList(agentid+3,poslist,std::move(llaList_down),cls);
//    connect(m_ptimer, &QTimer::timeout,[&]()
//            {
//                lon -= 0.01;
//                m_pModelWidget->updateEntityTracking(agentid, lon,lat,hgt,roll,pitch,yaw);
//            });
//    m_ptimer->start(100);
#endif

#else

#if 1
    m_pModelWidget->initNode(agentid,"",1, "", lon,lat,hgt,roll,pitch,yaw);
    std::vector<std::tuple<double, double, double> > poslist;
        poslist.push_back(std::make_tuple(
            113.26587006121008,
            23.505144318180797,
            90000));
        poslist.push_back(std::make_tuple(
            113.01806999650262,
            23.369055682729552,
            50000));
        poslist.push_back(std::make_tuple(
            113.15610439139448,
            23.251034094572134,
            50000));
        poslist.push_back(std::make_tuple(
            113.39742228933045,
            23.176364215666695,
            50000));
        poslist.push_back(std::make_tuple(
            113.52796531494772,
            23.3485537042406,
            50000));
        poslist.push_back(std::make_tuple(
            113.5532266556649,
            23.37416307514262,
            50000));
        poslist.push_back(std::make_tuple(
            113.26587006121008,
            23.505144318180797,
            90000));

        std::vector<double> llaList_down;
        llaList_down.push_back(50000);
        llaList_down.push_back(30000);
        llaList_down.push_back(30000);
        llaList_down.push_back(30000);
        llaList_down.push_back(30000);
        llaList_down.push_back(30000);
        llaList_down.push_back(50000);

        std::vector<std::tuple<QColor,QColor>> cls;
    //    cls.push_back(std::make_tuple(QColor(0,255,255,0),QColor(0,0,255,255)));
        m_pModelWidget->updatePosList(agentid,poslist,std::move(llaList_down),cls);
#endif


#if 0
    auto refile=[&](const QString& filepath)->std::vector<tagTrajectoryItem>{






        // 计算新的经纬度
        auto calculateNewCoordinates=[&](double lat, double lon, double course, double distance)->std::tuple<double,double> {
            double radLat = degreesToRadians(lat);
            double radLon = degreesToRadians(lon);
            double radCourse = degreesToRadians(course);

            double radDistance = distance / 6371000; // 地球半径，米

            double newLat = asin(sin(radLat) * cos(radDistance) + cos(radLat) * sin(radDistance) * cos(radCourse));
            double newLon = radLon + atan2(sin(radDistance) * sin(radCourse) * cos(radLat), cos(radDistance) - sin(radLat) * sin(newLat));

            return std::make_tuple(newLon * 180.0 / M_PI, newLat * 180.0 / M_PI);
        };

        std::vector<tagTrajectoryItem> trajectory;
        QFile file(filepath);
        std::cout.precision(17);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);
            if(!in.atEnd())
            {
                in.readLine();
            }
            while (!in.atEnd())
            {
                QString line = in.readLine();
                QStringList fields = line.split(QRegExp("\\s+")/*, QString::SkipEmptyParts*/);
                if(fields.size() >= 5)
                {
                    tagTrajectoryItem _tagTrajectory_cur;
                    _tagTrajectory_cur.strtimestamp = fields[0]+" "+fields[1];
                    _tagTrajectory_cur.itimestamp = QDateTime::fromString(_tagTrajectory_cur.strtimestamp,"yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch();
                    _tagTrajectory_cur.lon = fields[2].toDouble();
                    _tagTrajectory_cur.lat = fields[3].toDouble();
                    _tagTrajectory_cur.hgt = fields[4].toDouble();
                    _tagTrajectory_cur.speed = fields[5].toDouble();
                    _tagTrajectory_cur.yaw = fields[6].toDouble();
//                    std::cout << "时间:" << _tagTrajectory_cur.strtimestamp.toStdString()
//                             << " 时间:" << _tagTrajectory_cur.itimestamp
//                             << ", 经度:" << _tagTrajectory_cur.lon
//                             << ", 纬度:" << _tagTrajectory_cur.lat
//                             << ", 高度:" << _tagTrajectory_cur.hgt
//                             << "米, 速度:" << _tagTrajectory_cur.speed
//                             << "千米/小时, 航向:" << _tagTrajectory_cur.yaw << "度\n";


                    if(trajectory.size() >=1)
                    {
                        tagTrajectoryItem last_tagTrajectory = trajectory.back();
#ifdef EXTEND_POINTS
                        // 假设你有一个速度值（例如，以千米/小时为单位）
                        double speed = last_tagTrajectory.speed; // 千米/小时
                        double speedMs = (speed * 1000.0) / (3600.0); // 转换为米/秒

                        // 计算两个时间点之间的总时间（毫秒）
                        qlonglong totalTimeMs = qAbs(_tagTrajectory_cur.itimestamp - last_tagTrajectory.itimestamp);

                        // 计算插值的时间间隔（500毫秒）
                        qlonglong intervalMs = INTERVAL_MS;

                        // 计算在两个时间点之间需要插值的次数
                        int intervals = totalTimeMs / intervalMs;


//                        double oldlon = last_tagTrajectory.lon;
//                        double oldlat = last_tagTrajectory.lat;
//                        double oldhgt = last_tagTrajectory.hgt;

//                        double newlon = _tagTrajectory_cur.lon;
//                        double newlat = _tagTrajectory_cur.lat;
//                        double newhgt = _tagTrajectory_cur.hgt;

//                        double dist = FunctionAssistant::calc_dist(oldlat,oldlon,newlat,newlon);
//                        double cur_pitch = calculatePitch(oldhgt, newhgt, dist);

                        if(intervals == 0)
                        {
                            double oldlon = last_tagTrajectory.lon;
                            double oldlat = last_tagTrajectory.lat;
                            double oldhgt = last_tagTrajectory.hgt;

                            double newlon = _tagTrajectory_cur.lon;
                            double newlat = _tagTrajectory_cur.lat;
                            double newhgt = _tagTrajectory_cur.hgt;

                            double dist = FunctionAssistant::calc_dist(oldlat,oldlon,newlat,newlon);
                            _tagTrajectory_cur.pitch = calculatePitch(oldhgt, newhgt, dist);
                            //                      _tagTrajectory_cur.pitch.yaw = calculateBearing(oldlat,oldlon,newlat,newlon);


                            trajectory.push_back(std::move(_tagTrajectory_cur));
                        }
                        else
                        {

                            for (int i = 1; i <= intervals; ++i)
                            {
                                // 计算插值的时间戳
                                qlonglong interpolatedTimestamp = i * intervalMs + last_tagTrajectory.itimestamp;

                                // 计算距离
                                double distance = speedMs * (i * intervalMs / 1000.0);

                                // 计算新的经纬度
                                std::tuple<double,double> newCoords = calculateNewCoordinates(last_tagTrajectory.lat, last_tagTrajectory.lon, last_tagTrajectory.yaw, distance);

                                // 线性插值计算新的高度
                                double interpolatedHeight = last_tagTrajectory.hgt + (_tagTrajectory_cur.hgt - last_tagTrajectory.hgt) * (i * intervalMs / static_cast<double>(totalTimeMs));

                                // 线性插值计算新的ywa
                                double interpolatedYaw = last_tagTrajectory.yaw + (_tagTrajectory_cur.yaw - last_tagTrajectory.yaw) * (i * intervalMs / static_cast<double>(totalTimeMs));

                                // 线性插值计算新的speed
                                double interpolatedSpeed = last_tagTrajectory.speed + (_tagTrajectory_cur.speed - last_tagTrajectory.speed) * (i * intervalMs / static_cast<double>(totalTimeMs));


                                tagTrajectoryItem _tagTrajectory_tmp;
                                _tagTrajectory_tmp.strtimestamp = QDateTime::fromMSecsSinceEpoch(interpolatedTimestamp).toString("yyyy-MM-dd HH:mm:ss.zzz");
                                _tagTrajectory_tmp.itimestamp = interpolatedTimestamp;
                                _tagTrajectory_tmp.lon = std::get<0>(newCoords);
                                _tagTrajectory_tmp.lat = std::get<1>(newCoords);
                                _tagTrajectory_tmp.hgt = interpolatedHeight;
                                _tagTrajectory_tmp.speed = interpolatedSpeed;
                                _tagTrajectory_tmp.yaw = interpolatedYaw;

                                //                            std::cout << "t:" << _tagTrajectory_tmp.strtimestamp.toStdString()
                                //                                      << " t:" << _tagTrajectory_tmp.itimestamp
                                //                                      << ", lon:" << _tagTrajectory_tmp.lon
                                //                                      << ", lat:" << _tagTrajectory_tmp.lat
                                //                                      << ", hgt:" << _tagTrajectory_tmp.hgt
                                //                                      << "米, speed:" << _tagTrajectory_tmp.speed
                                //                                      << "千米/小时, yaw:" << _tagTrajectory_tmp.yaw << "度\n";

                                double oldlon = trajectory.back().lon;
                                double oldlat = trajectory.back().lat;
                                double oldhgt = trajectory.back().hgt;

                                double newlon = _tagTrajectory_tmp.lon;
                                double newlat = _tagTrajectory_tmp.lat;
                                double newhgt = _tagTrajectory_tmp.hgt;

                                double dist = FunctionAssistant::calc_dist(oldlat,oldlon,newlat,newlon);
                                _tagTrajectory_tmp.pitch = calculatePitch(oldhgt, newhgt, dist);

                                trajectory.push_back(std::move(_tagTrajectory_tmp));
                            }
                        }
#else
                        double oldlon = last_tagTrajectory.lon;
                        double oldlat = last_tagTrajectory.lat;
                        double oldhgt = last_tagTrajectory.hgt;

                        double newlon = _tagTrajectory_cur.lon;
                        double newlat = _tagTrajectory_cur.lat;
                        double newhgt = _tagTrajectory_cur.hgt;

                        double dist = FunctionAssistant::calc_dist(oldlat,oldlon,newlat,newlon);
                        _tagTrajectory_cur.pitch = calculatePitch(oldhgt, newhgt, dist);
//                      _tagTrajectory_cur.pitch.yaw = calculateBearing(oldlat,oldlon,newlat,newlon);


                        trajectory.push_back(std::move(_tagTrajectory_cur));
#endif
                    }
                    else
                    {
                        trajectory.push_back(std::move(_tagTrajectory_cur));
                    }
                }
            }
        }

        file.close();
        return trajectory;
    };



    std::function<void (const QString & ,bool)> findTrajectoryFolders=[&](const QString &path,bool bTrajectoryDir)
    {
        QDir dir(path);

        // 检查路径是否存在且是一个目录
        if (!dir.exists()) {
            qDebug() << "提供的路径不是一个有效的目录";
            return;
        }

        // 获取目录中的所有条目
        QFileInfoList entries ;
        if(bTrajectoryDir)
        {
            entries = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);

        }
        else
        {
            entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        }
        for (const QFileInfo &entry : entries)
        {
            if (entry.isDir())
            {
                if (entry.fileName() == "Trajectory")
                {
                    findTrajectoryFolders(entry.absoluteFilePath(),true);
                }
                else
                {
                    findTrajectoryFolders(entry.absoluteFilePath(),false);
                }
            }
            else
            {
                QStringList pathParts = dir.absolutePath().split("/");
                QString flightcode = pathParts.at(pathParts.size()-2);
                qDebug() << "找到 Trajectory 文件夹:" << flightcode <<" "<<entry.absoluteFilePath();

                                                                                      std::vector<tagTrajectoryItem> trajectory =refile(entry.absoluteFilePath());

                tagFightTrajectorys tagFightTrajectorys;
                tagFightTrajectorys.filghtcode = flightcode;
                auto itor = m_FightTrajectorys.find(flightcode);
                if(itor != m_FightTrajectorys.end())
                {
                    itor->second.trajectorys.push_back(std::move(trajectory));
                }
                else
                {
                    tagFightTrajectorys.trajectorys.push_back(std::move(trajectory));
                    m_FightTrajectorys.insert(std::make_pair(flightcode, std::move(tagFightTrajectorys)));
                }
            }
        }
    };

//    findTrajectoryFolders("E:/2016YFB0502300-04-005_Data/2016YFB0502300-04-005_Data/航班",false);

//    findTrajectoryFolders("E:/0123",false);
    findTrajectoryFolders("E:/2020-01-23",false);
//    findTrajectoryFolders("E:/123",false);


#if 0
    int agentid = 0;
    auto itor = _flights.begin();
    while(itor != _flights.end())
    {
        const tagFightTrajectorys &tagFightTrajectorys = itor->second;
        auto iitor = tagFightTrajectorys.trajectorys.begin();
        while(iitor != tagFightTrajectorys.trajectorys.end())
        {
            const std::vector<tagTrajectory> &trajectory = *iitor;
            m_pModelWidget->initNode(agentid,"",1, "", lon,lat,hgt,roll,pitch,yaw);
            std::vector<std::tuple<double, double, double> > poslist;
            for(int i = 0 ; i < trajectory.size(); i++)
            {
                poslist.push_back(std::make_tuple(trajectory.at(i).lon,
                                                  trajectory.at(i).lat,
                                                  trajectory.at(i).hgt));
            }
            m_pModelWidget->updatePosList(agentid,poslist);
            agentid++;
            iitor++;
        }
        itor++;
    }
#else

    {
        uint64_t agentid = 0;
        auto itor = m_FightTrajectorys.begin();
        while(itor != m_FightTrajectorys.end())
        {
            const QString &flightcode = itor->first;
            const tagFightTrajectorys &tagFightTrajectorys = itor->second;
            auto iitor = tagFightTrajectorys.trajectorys.begin();
            while(iitor != tagFightTrajectorys.trajectorys.end())
            {
                const std::vector<tagTrajectoryItem> &trajectory = *iitor;

                for(int index = 0;index < trajectory.size();index++)
                {
                    const tagTrajectoryItem& item = trajectory.at(index);
                    if(item.itimestamp < min_timestamp)
                    {
                        min_timestamp = item.itimestamp;
                    }
                    if(item.itimestamp > max_timestamp)
                    {
                        max_timestamp = item.itimestamp;
                    }


                    tagFlightVenet _tagFlightVenet;
                    _tagFlightVenet.filghtcode = flightcode;
                    _tagFlightVenet.agentid = agentid;
                    _tagFlightVenet.ptagTrajectoryItem = &item;
                    auto _flightsitor = m_flights.find(item.itimestamp);
                    if(_flightsitor != m_flights.end())
                    {
                        std::list<tagFlightVenet>& _tagFlightVenetlist = _flightsitor->second;
                        _tagFlightVenetlist.push_back(std::move(_tagFlightVenet));
                    }
                    else
                    {
                        std::list<tagFlightVenet> _tagFlightVenetlist;
                        _tagFlightVenetlist.push_back(std::move(_tagFlightVenet));

                        m_flights.insert(std::make_pair(item.itimestamp, std::move(_tagFlightVenetlist)));
                    }
                }

                QString path = QCoreApplication::applicationDirPath() + "/res/model/A350_1000.fbx";

                m_pModelWidget->initNode(agentid,
                                         flightcode,
                                         0,
                                         path,
                                         trajectory.at(0).lon,
                                         trajectory.at(0).lat,
                                         trajectory.at(0).hgt,
                                         trajectory.at(0).roll,
                                         trajectory.at(0).pitch,
                                         trajectory.at(0).yaw,0.5);

                m_pModelWidget->visibleEntityTracking(agentid,false,(UINT64)QtOSGWidget::E_VISIABLE_ATTRIBUTE_LABEL |
                                                                          (UINT64)QtOSGWidget::E_VISIABLE_ATTRIBUTE_TAIL);
                agentid++;
                iitor++;
            }
            itor++;
        }
    }


    start_itimeindex = min_timestamp;

//    {
//        auto _flightsitor = m_flights.begin();
//        while(_flightsitor != m_flights.end())
//        {
//            auto trigger_timestamp = _flightsitor->first;
//            uint64_t standrad_timestamp = start_itimeindex + ((trigger_timestamp-start_itimeindex)/INTERVAL_MS) * INTERVAL_MS;
//            auto m_flights_triggeritor = m_flights_trigger.find(standrad_timestamp);
//            if(m_flights_triggeritor != m_flights_trigger.end())
//            {
//                std::list<uint64_t>& _tagFlightVenetlist = m_flights_triggeritor->second;
//                _tagFlightVenetlist.push_back(std::move(trigger_timestamp));
//            }
//            else
//            {
//                std::list<uint64_t> _tagFlightVenetlist;
//                _tagFlightVenetlist.push_back(std::move(trigger_timestamp));
//                m_flights_trigger.insert(std::make_pair(standrad_timestamp, std::move(_tagFlightVenetlist)));
//            }

//            _flightsitor++;
//        }
//    }

    {

        connect(m_ptimer, &QTimer::timeout,[&](){

#if 0
            uint64_t agentid = 0;
            auto _flightsitor = m_FightTrajectorys.begin();
            while(_flightsitor != m_FightTrajectorys.end())
            {
                const tagFightTrajectorys &tagFightTrajectorys = _flightsitor->second;
                auto iitor = tagFightTrajectorys.trajectorys.begin();
                while(iitor != tagFightTrajectorys.trajectorys.end())
                {
                    const std::vector<tagTrajectoryItem> &trajectory = *iitor;
                    if(start_itimeindex < trajectory.size())
                    {
                        m_pModelWidget->updateEntityTracking(agentid,
                                                             trajectory.at(start_itimeindex).lon,
                                                             trajectory.at(start_itimeindex).lat,
                                                             trajectory.at(start_itimeindex).hgt,
                                                             trajectory.at(start_itimeindex).roll,
                                                             trajectory.at(start_itimeindex).pitch,
                                                             trajectory.at(start_itimeindex).yaw);
                    }
                    agentid++;
                    iitor++;
                }
                _flightsitor++;
            }

            start_itimeindex++;
#else
//            auto m_flights_triggeritor = m_flights_trigger.find(start_itimeindex);
//            if(m_flights_triggeritor != m_flights_trigger.end())
//            {
//                std::list<uint64_t>& _tagFlighttimelist = m_flights_triggeritor->second;
//                for(auto _tagFlighttimelist_itor = _tagFlighttimelist.begin(); _tagFlighttimelist_itor != _tagFlighttimelist.end(); _tagFlighttimelist_itor++)
//                {
//                    auto _flightsitor = m_flights.find(*_tagFlighttimelist_itor);
                    auto _flightsitor = m_flights.find(start_itimeindex);
                    if(_flightsitor != m_flights.end())
                    {
                        std::list<tagFlightVenet>& _tagFlightVenetlist = _flightsitor->second;
                        auto _tagFlightVenetlist_itor = _tagFlightVenetlist.begin();
                        while (_tagFlightVenetlist_itor != _tagFlightVenetlist.end())
                        {
                            const tagFlightVenet& _tagFlightVenet = *_tagFlightVenetlist_itor;
                            if(_tagFlightVenet.ptagTrajectoryItem)
                            {

                                m_pModelWidget->updateEntityTracking(_tagFlightVenet.agentid,
                                                                     _tagFlightVenet.ptagTrajectoryItem->lon,
                                                                     _tagFlightVenet.ptagTrajectoryItem->lat,
                                                                     _tagFlightVenet.ptagTrajectoryItem->hgt,
                                                                     _tagFlightVenet.ptagTrajectoryItem->roll,
                                                                     _tagFlightVenet.ptagTrajectoryItem->pitch,
                                                                     _tagFlightVenet.ptagTrajectoryItem->yaw);
                            }

                            _tagFlightVenetlist_itor++;
                        }
                    }
//                }
//            }

            start_itimeindex += INTERVAL_MS;
#endif
        });
        m_ptimer->start(20);

    }
#endif


#endif
#endif

#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

