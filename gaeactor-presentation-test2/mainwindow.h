#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/Switch>
#include <osgViewer/Viewer>
#include <random>


#include <osgParticle/ExplosionEffect>
#include <osgParticle/ExplosionDebrisEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/FireEffect>
#include <osgParticle/Particle>
#include <osgParticle/LinearInterpolator>

#include <osgParticle/ParticleSystemUpdater>
#include <osgParticle/ModularProgram>
#include <osg/MatrixTransform>

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Simplifier>
#include <osgUtil/Optimizer>
#include <osgGA/StateSetManipulator>

#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>

#include "widget3d/CommFunc.h"

#include <optional>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class QtOSGWidget;
class QTimer;


#define AU_KM (149597870.7)


struct tagParam
{
    std::string name;
    double radius;
    double track_au;
    double speed;
    double revolution_day;
    double rotation_day;
    bool clockwise;

    int satellite;
    std::unordered_map<std::string, tagParam> satellites;
    double track_km()
    {
        return track_au*AU_KM;
    }
};

struct tagPlanetNode
{
    tagPlanetNode() {m_nSeg = 8;}

    std::string planetName;
    UINT32 m_nSeg;
    //星球半径 km
    double _planet_radius_km;
    //星球半径 换算成得x y z 空间得长度
    double _planet_coordinate_radius;
    //星球轨道半径 km
    double _planet_orbit_radius_km;
    //星球轨道半径 换算成得x y z 空间得长度
    double _planet_orbit_coordinate_radius;

    osg::ref_ptr<osg::Geometry> planetGeometry;
    osg::ref_ptr<osg::Geode> planetGeode;
    osg::ref_ptr<osg::Switch> planetext;
    osg::ref_ptr<osg::MatrixTransform> planetTransform;
    osg::ref_ptr<osg::Switch> planetGridGroup;
    osg::ref_ptr<osg::Switch> planetAnchorGroup;
    osg::ref_ptr<osg::Geode> orbitGeode;
    osg::ref_ptr<osg::Geometry> orbitGeometry;
    osg::ref_ptr<osg::Switch> orbitControl;
    osg::ref_ptr<osg::Group> planetGroup;

    std::vector<tagPlanetNode*> subPlanets;
};

struct tagpos{
    double x;
    double y;
    double z;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    //////////////////////////////////////////////////////////////////////////////////////////
    void initPlanetInfo();
    //////////////////////////////////////////////////////////////////////////////////////////
    void updatePlanetGridMesh(const std::string &planetname, std::vector<std::vector<tagpos>> &_datas);
    void updatePlanetTracking(const std::string &planetname, const osg::Vec3f & position);

    //////////////////////////////////////////////////////////////////////////////////////////
    void translatePlanetNode(const std::string &planetname,const osg::Vec3f &transPos);
    void rotatePlanetNode(const std::string &planetname,const osg::Vec3f &eulerAngles);
    void scalePlanetNode(const std::string &planetname,const osg::Vec3f &fscalef);
    //////////////////////////////////////////////////////////////////////////////////////////
    osg::Switch* createSphereGeometryExs(std::vector<std::vector<std::tuple<osg::Vec3, osg::Vec4>> >&_datas, const std::string& planetname, const double& dRadius, const osg::Vec3& center = osg::Vec3(0,0,0), const osg::Vec4& cl = osg::Vec4(1.0, 1.0, 1.0, 1.0), const double& lineWidth = 1.0,bool bVisiable = true);
    osg::Switch* createAnchorGroup(const std::string& planetname,const double &dRadius,bool bVisiable = true);
    tagPlanetNode* createPlanetGroup(const std::string& planetname, const std::string &texturePath, const osg::Vec3 &place, const double &_planet_orbit_radius_km, const double &radius, const bool &bShoworbit, const bool &planet, const osg::Vec3 &center = osg::Vec3(0, 0, 0));
    osg::Group* createStonesBetween(float nearadius, float farradius);
    void setTex(osg::Node* pNode, const std::string& path);
    //////////////////////////////////////////////////////////////////////////////////////////
    void loadPositionsFromFile(const QString &path, std::vector<std::vector<std::tuple<osg::Vec3, osg::Vec4>>>& _datas);
    void getSpherePremeshData(std::vector<std::vector<tagpos>>& _datas,UINT32 nSeg = 8, UINT32 nPointEachEdge=21);
    void adjustPlanetGridMesh(const std::string &planetname,bool bPlus);
    //////////////////////////////////////////////////////////////////////////////////////////
    void deal_pick_to_append_point(const tagCallBackParams &params);
    //////////////////////////////////////////////////////////////////////////////////////////
    void widget_callback(const tagCallBackParams & params);

    std::optional<tagPlanetNode *> getPlanet(const std::string &planetname);

    void updatePlanetRunningOrbitTracking();

    void nodeAppendLines(osg::Node* pNode, std::vector<std::vector<std::tuple<osg::Vec3, osg::Vec4> > > &lines);

private:
    virtual bool eventFilter(QObject *watched, QEvent *event);
private:
    Ui::MainWindow *ui;

    QtOSGWidget* m_pModelWidget;


    std::vector<std::vector<std::tuple<osg::Vec3, osg::Vec4>> > m_datas;
    std::unordered_map<std::string, tagPlanetNode> m_PlanetNodes;

    std::unordered_map<std::string, tagParam> m_planetParams;
    std::string m_cur_focus_planet ="Sun";
    UINT32 m_nSeg = 8;
    UINT32 m_nPointEachEdge2 = 21;

    QTimer * m_pUpdateTimer;
    std::vector<osg::Vec3f> m_UpdateBaseVerts;
};
#endif // MAINWINDOW_H
