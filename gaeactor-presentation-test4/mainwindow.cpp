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




//创建火球(燃烧)
void createFireBall(osg::MatrixTransform* smokeNode)
{
    // 创建粒子对象，设置其属性并交由粒子系统使用。
    osgParticle::Particle particleTempalte;
    particleTempalte.setShape(osgParticle::Particle::QUAD);
    particleTempalte.setLifeTime(1.5); // 单位：秒
    particleTempalte.setSizeRange(osgParticle::rangef(3.0f, 1.0f)); // 单位：米
    particleTempalte.setAlphaRange(osgParticle::rangef(1, 0));
    particleTempalte.setColorRange(osgParticle::rangev4(osg::Vec4(1.0f, 0.2f, 0.0f, 1.0f),//0.1f,0.3f,0.4f,1.0f
                                                        osg::Vec4(0.1f, 0.1f, 0.1f, 0)//0.95f,0.75f,0,1(1,1,1,1)
                                                        ));
    particleTempalte.setPosition(osg::Vec3(0.0f, 0.0f, 0.0f));
    particleTempalte.setVelocity(osg::Vec3(0.0f, 0.0f, 0.0f));
    particleTempalte.setMass(0.1f); //单位：千克
    particleTempalte.setRadius(0.2f);
    particleTempalte.setSizeInterpolator(new osgParticle::LinearInterpolator);
    particleTempalte.setAlphaInterpolator(new osgParticle::LinearInterpolator);
    particleTempalte.setColorInterpolator(new osgParticle::LinearInterpolator);
    // 创建并初始化粒子系统。
    osgParticle::ParticleSystem* particleSystem = new osgParticle::ParticleSystem;
    particleSystem->setDataVariance(osg::Node::STATIC);

    QString smokepath = QCoreApplication::applicationDirPath() + QString::fromStdString("/res/osgearth/OpenSceneGraph-Data/Images/smoke.rgb");
    // 设置材质，是否放射粒子，以及是否使用光照。
    particleSystem->setDefaultAttributes(smokepath.toStdString(), true, false);
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(particleSystem);
    smokeNode->addChild(geode);
    //设置为粒子系统的缺省粒子对象。
    particleSystem->setDefaultParticleTemplate(particleTempalte);
    //获取放射极中缺省计数器的句柄，调整每帧增加的新粒子数目
    osgParticle::RandomRateCounter* particleGenerateRate = new osgParticle::RandomRateCounter();
    particleGenerateRate->setRateRange(30, 50);
    // 每秒新生成的粒子范围
    particleGenerateRate->setDataVariance(osg::Node::DYNAMIC);
    // 自定义一个放置器，这里创建并初始化一个点放置器
    osgParticle::PointPlacer* particlePlacer = new osgParticle::PointPlacer;
    particlePlacer->setCenter(osg::Vec3(0.0f, 0.0f, 0.0f));
    particlePlacer->setDataVariance(osg::Node::DYNAMIC);
    // 自定义一个弧度发射器
    osgParticle::RadialShooter* particleShooter = new osgParticle::RadialShooter;
    // 设置发射器的属性
    particleShooter->setDataVariance(osg::Node::DYNAMIC);
    particleShooter->setThetaRange(-0.1f, 0.1f);
    // 弧度值，与Z 轴夹角
    particleShooter->setPhiRange(-0.1f, 0.1f);
    particleShooter->setInitialSpeedRange(5, 7.5f);//单位：米/秒
        //创建标准放射极对象
    osgParticle::ModularEmitter* emitter = new osgParticle::ModularEmitter;
    emitter->setDataVariance(osg::Node::DYNAMIC);
    emitter->setCullingActive(false);
    // 将放射极对象与粒子系统关联。
    emitter->setParticleSystem(particleSystem);
    // 设置计数器
    emitter->setCounter(particleGenerateRate);
    // 设置放置器
    emitter->setPlacer(particlePlacer);
    // 设置发射器
    emitter->setShooter(particleShooter);
    // 把放射极添加为变换节点
    smokeNode->addChild(emitter);
    // 添加更新器，以实现每帧的粒子管理。
    osgParticle::ParticleSystemUpdater* particleSystemUpdater = new osgParticle::ParticleSystemUpdater;
    // 将更新器与粒子系统对象关联。
    particleSystemUpdater->addParticleSystem(particleSystem);
    // 将更新器节点添加到场景中。
    smokeNode->addChild(particleSystemUpdater);
    // 创建标准编程器对象并与粒子系统相关联。
    osgParticle::ModularProgram* particleMoveProgram = new osgParticle::ModularProgram;
    particleMoveProgram->setParticleSystem(particleSystem);
    // 最后，将编程器添加到场景中。
    smokeNode->addChild(particleMoveProgram);
}


//创建浓烟
void createDarkSmoke(osg::MatrixTransform* smokeNode)
{
    // 创建粒子对象，设置其属性并交由粒子系统使用。
    osgParticle::Particle particleTempalte;
    particleTempalte.setShape(osgParticle::Particle::QUAD);
    particleTempalte.setLifeTime(10); // 单位：秒
    particleTempalte.setSizeRange(osgParticle::rangef(1.0f, 12.0f)); // 单位：米
    particleTempalte.setAlphaRange(osgParticle::rangef(1, 0));
    particleTempalte.setColorRange(osgParticle::rangev4(
        osg::Vec4(0.0f, 0.0f, 0.0f, 0.5f),//(0.1f,0.1f,0.1f,0.5f)
        osg::Vec4(0.5f, 0.5f, 0.5f, 1.5f)//0.95f,0.75f,0,1
        ));
    particleTempalte.setPosition(osg::Vec3(0.0f, 0.0f, 0.0f));
    particleTempalte.setVelocity(osg::Vec3(0.0f, 0.0f, 0.0f));
    particleTempalte.setMass(0.1f); //单位：千克
    particleTempalte.setRadius(0.2f);
    particleTempalte.setSizeInterpolator(new osgParticle::LinearInterpolator);
    particleTempalte.setAlphaInterpolator(new osgParticle::LinearInterpolator);
    particleTempalte.setColorInterpolator(new osgParticle::LinearInterpolator);
    // 创建并初始化粒子系统。
    osgParticle::ParticleSystem* particleSystem = new osgParticle::ParticleSystem;
    particleSystem->setDataVariance(osg::Node::STATIC);
    // 设置材质，是否放射粒子，以及是否使用光照。
    QString smokepath = QCoreApplication::applicationDirPath() + QString::fromStdString("/res/osgearth/OpenSceneGraph-Data/Images/smoke.rgb");
    particleSystem->setDefaultAttributes(smokepath.toStdString(), false, false);
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(particleSystem);
    smokeNode->addChild(geode);
    //设置为粒子系统的缺省粒子对象。
    particleSystem->setDefaultParticleTemplate(particleTempalte);
    //获取放射极中缺省计数器的句柄，调整每帧增加的新粒
    //子数目
    osgParticle::RandomRateCounter* particleGenerateRate = new osgParticle::RandomRateCounter();
    particleGenerateRate->setRateRange(30, 50);
    // 每秒新生成的粒子范围
    particleGenerateRate->setDataVariance(osg::Node::DYNAMIC);
    // 自定义一个放置器，这里我们创建并初始化一个点放置器
    osgParticle::PointPlacer* particlePlacer = new osgParticle::PointPlacer;
    particlePlacer->setCenter(osg::Vec3(0.0f, 0.0f, 0.05f));
    particlePlacer->setDataVariance(osg::Node::DYNAMIC);
    // 自定义一个弧度发射器
    osgParticle::RadialShooter* particleShooter = new osgParticle::RadialShooter;
    // 设置发射器的属性
    particleShooter->setDataVariance(osg::Node::DYNAMIC);
    particleShooter->setThetaRange(-0.1f, 0.1f);
    // 弧度值，与Z 轴夹角0.392699f
    particleShooter->setPhiRange(-0.1f, 0.1f);
    particleShooter->setInitialSpeedRange(10, 15);
    //单位：米/秒
    //创建标准放射极对象
    osgParticle::ModularEmitter* emitter = new osgParticle::
        ModularEmitter;
    emitter->setDataVariance(osg::Node::DYNAMIC);
    emitter->setCullingActive(false);
    // 将放射极对象与粒子系统关联。
    emitter->setParticleSystem(particleSystem);
    // 设置计数器
    emitter->setCounter(particleGenerateRate);
    // 设置放置器
    emitter->setPlacer(particlePlacer);
    // 设置发射器
    emitter->setShooter(particleShooter);
    // 把放射极添加为变换节点
    smokeNode->addChild(emitter);
    // 添加更新器，以实现每帧的粒子管理。
    osgParticle::ParticleSystemUpdater* particleSystemUpdater = new osgParticle::ParticleSystemUpdater;
    // 将更新器与粒子系统对象关联。
    particleSystemUpdater->addParticleSystem(particleSystem);
    // 将更新器节点添加到场景中。
    smokeNode->addChild(particleSystemUpdater);
    osgParticle::ModularProgram* particleMoveProgram = new osgParticle::ModularProgram;
    particleMoveProgram->setParticleSystem(particleSystem);
    // 最后，将编程器添加到场景中。
    smokeNode->addChild(particleMoveProgram);
}

void WriteFireballAndSmoke()
{
    osg::ref_ptr<osg::Group> root = new osg::Group();
    osg::MatrixTransform* flightTransform = new osg::MatrixTransform();
    root->addChild(flightTransform);
    //引擎烟雾
    osg::MatrixTransform* fireballAndSmoke = new osg::MatrixTransform();
    createFireBall(fireballAndSmoke);

    fireballAndSmoke->setMatrix(osg::Matrix::rotate(-osg::PI / 2, 1, 0, 0) * osg::Matrix::translate(-8, -10, -3));
    flightTransform->addChild(fireballAndSmoke);
    createDarkSmoke(fireballAndSmoke);

    QString smokepath = QCoreApplication::applicationDirPath() + QString::fromStdString("/res/osgearth/OpenSceneGraph-Data/Images/MyScene.osg");
    osgDB::writeNodeFile(*(root.get()), smokepath.toStdString());
}

void ReadFireballAndSmoke()
{
    osg::ref_ptr<osg::Group> root = new osg::Group();
    QString smokepath = QCoreApplication::applicationDirPath() + QString::fromStdString("/res/osgearth/OpenSceneGraph-Data/Images/MyScene.osg");
    osg::Node* flightNode = osgDB::readNodeFile(smokepath.toStdString());
    if (!flightNode)
    {
        return;
    }
    osg::MatrixTransform* flightTransform = new osg::MatrixTransform();
    flightTransform->addChild(flightNode);
    root->addChild(flightTransform);
    osgViewer::Viewer viewer;

    osgUtil::Simplifier simplifier(0.3f, 4.0f);
    osgUtil::Optimizer optimzer;

    optimzer.optimize(root.get());
    viewer.setSceneData(root.get());

    //添加一个事件句柄 相当于添加一个响应 响应鼠标或是键盘 响应L键(控制灯光开关)
    viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

    //    //窗口大小变化事件 添加窗口大小改变的句柄 这里响应的是F键
    //    viewer.addEventHandler(new osgViewer::WindowSizeHandler);

    //    //添加一些常用状态设置  添加常用的状态操作，这里会响应S键、W键等等
    //  viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.realize();
    viewer.run();
}

//陨石坠落
//int main()
//{
//  WriteFireballAndSmoke();
//  ReadFireballAndSmoke();
//
//  return 0;
//}

//1km-->100格子
//1格子--->10m
//#define PHYSICS_RADIUS_TO_SOLAR_SYSTEM_COORDINATES_RADIUS (100)
#define PHYSICS_RADIUS_TO_SOLAR_SYSTEM_COORDINATES_RADIUS (1000)

#define PRASE_M_TO_COORDINATES (PHYSICS_RADIUS_TO_SOLAR_SYSTEM_COORDINATES_RADIUS/1000.0f)

#include <osg/ComputeBoundsVisitor>
#include "widget3d/CommFunc.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->resize(1400,1200);


    m_pModelWidget = new QtOSGWidget(QtOSGWidget::E_OSG_SHOW_TYPE_MODEL, this);
    this->setCentralWidget(m_pModelWidget);


//    QString nodepath = QCoreApplication::applicationDirPath() + QString::fromStdString("/res/osgearth/OpenSceneGraph-Data/cessna.osgt");
//    QString nodepath = QCoreApplication::applicationDirPath() + QString::fromStdString("/res/osgearth/model/W/china/ive/su27.ive");

//    QString nodepath = QCoreApplication::applicationDirPath() + QString::fromStdString("/res/osgearth/model/spacestation.ive");
//    QString nodepath = QCoreApplication::applicationDirPath() + QString::fromStdString("/res/osgearth/model/Satellites.ive");
//    QString nodepath = QCoreApplication::applicationDirPath() + QString::fromStdString("/res/osgearth/model/spacestation.fbx");
//    QString nodepath = QString::fromStdString("D:/model/stallites/tiangong.fbx");
//    QString nodepath = QString::fromStdString("D:/model/stallites/weixin.fbx");
//    QString nodepath = QString::fromStdString("D:/model/stallites/dimianzhan.fbx");
//    QString nodepath = QString::fromStdString("D:/model/stallites/tiangong2.fbx");
//    QString nodepath = QString::fromStdString("D:/model/stallites/kongjianzhan.fbx");
    QString nodepath = QString::fromStdString("D:/model/stallites/Satellites.fbx");

    osg::ref_ptr<osg::Group> root = new osg::Group();
    osg::Node* flightNode = osgDB::readNodeFile(nodepath.toStdString());
    if (!flightNode)
    {
        return ;
    }


    osg::ComputeBoundsVisitor cbv;
    flightNode->accept(cbv);
    osg::BoundingBox bb = cbv.getBoundingBox();

    osg::BoundingSphere bs = flightNode->getBound();
    double radius = flightNode->getBound().radius();
    double _radius = (radius / 100.0f) * PRASE_M_TO_COORDINATES;

    // 计算模型的尺寸
    osg::Vec3d size = bb._max - bb._min;
    double x_m = size.x() / 100.0f;
    double y_m = size.y() / 100.0f;
    double z_m = size.z() / 100.0f;

    double x_coordinates = PRASE_M_TO_COORDINATES * x_m;
    double y_coordinates = PRASE_M_TO_COORDINATES * y_m;
    double z_coordinates = PRASE_M_TO_COORDINATES * z_m;

    double scale = x_coordinates / size.x();

    osg::Vec3f fscalef(scale, scale, scale);

    std::cout << "scale: " <<scale<< std::endl;
    std::cout <<size.x()<< " width: " << x_m << " m " <<x_coordinates<<" "<<x_coordinates / size.x()<< std::endl;
    std::cout <<size.y()<< " height: " << y_m << " m " <<y_coordinates<<" "<<y_coordinates / size.y()<< std::endl;
    std::cout <<size.z()<< " depth: " << z_m << " m " <<z_coordinates<<" "<<z_coordinates / size.z()<< std::endl;
    osg::MatrixTransform* flightNodeTransform = new osg::MatrixTransform;

    osg::Matrix matrixTmp = flightNodeTransform->getMatrix();
    matrixTmp *= osg::Matrix::scale(fscalef);
    flightNodeTransform->setMatrix(matrixTmp);

    flightNodeTransform->addChild(flightNode);


//    osg::Vec3f center = flightNodeTransform->getBound().center();

//    osg::Matrix matrixTmp2 = flightNodeTransform->getMatrix();
//    //先平移到原点
//    matrixTmp2 *= osg::Matrix::translate(-center);


//    //从原点平移到原来的位置
////    matrixTmp2 *= osg::Matrix::translate(center);
//    flightNodeTransform->setMatrix(matrixTmp);



    osg::ref_ptr<osg::Switch> flightNodeSwitch = new osg::Switch;

    flightNodeSwitch->addChild(flightNodeTransform);


    auto addLight=[&](osg::ref_ptr<osg::Switch>& pSwitch, osg::Vec4 ambient, osg::Vec4 diffuse, bool bOn)
    {
        osg::ref_ptr<osg::Node> node = pSwitch->getChild(0);
        std::string nodeLightSwitchName = "_lightSwitch";

        osg::ref_ptr<osg::Switch> pLightSwitch = new osg::Switch;
        pLightSwitch->setName(nodeLightSwitchName);

        osg::ref_ptr<osg::StateSet> pStateSet = node->getOrCreateStateSet();
        pStateSet = pSwitch->getOrCreateStateSet();
        pStateSet->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON);
        pStateSet->setMode(GL_NORMALIZE, osg::StateAttribute::ON);

        pStateSet = pSwitch->getOrCreateStateSet();
        pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);
        pStateSet->setMode(GL_LIGHT0, bOn ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
        pStateSet->setMode(GL_LIGHT1, bOn ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
        pStateSet->setMode(GL_LIGHT2, bOn ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
        pStateSet->setMode(GL_LIGHT3, bOn ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
        pStateSet->setMode(GL_LIGHT4, bOn ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
        pStateSet->setMode(GL_LIGHT5, bOn ? osg::StateAttribute::ON : osg::StateAttribute::OFF);

        osg::BoundingSphere bs;
        pSwitch->computeBound();
        bs = pSwitch->getBound();
        float lightRadius = bs.radius();

        osg::Vec4 posCenter = osg::Vec4(bs.center().x(), bs.center().y(), bs.center().z(), 0);
        osg::Vec4 posMinus_Y = osg::Vec4(posCenter.x(), posCenter.y() - lightRadius, posCenter.z(), 0.0);
        osg::Vec4 posPlus_Y = osg::Vec4(posCenter.x(), posCenter.y() + lightRadius, posCenter.z(), 0.0);
        osg::Vec4 posMinus_Z = osg::Vec4(posCenter.x(), posCenter.y(), posCenter.z() - lightRadius, 0.0);
        osg::Vec4 posPlus_Z = osg::Vec4(posCenter.x(), posCenter.y(), posCenter.z() + lightRadius, 0.0);
        osg::Vec4 posMinus_X = osg::Vec4(posCenter.x() - lightRadius, posCenter.y(), posCenter.z(), 0.0);
        osg::Vec4 posPlus_X = osg::Vec4(posCenter.x() + lightRadius, posCenter.y(), posCenter.z(), 0.0);

        osg::ref_ptr<osg::LightSource> pLightSource1 = DrawFunc::creteOneLight(tagLightInfo(1, posPlus_Z, osg::Vec3(0.0, 0.0, -1.0), ambient, diffuse));
//        osg::ref_ptr<osg::LightSource> pLightSource0 = DrawFunc::creteOneLight(tagLightInfo(1, posMinus_Z, osg::Vec3(0.0, 0.0, 1.0), ambient, diffuse));
//        osg::ref_ptr<osg::LightSource> pLightSource2 = DrawFunc::creteOneLight(tagLightInfo(2, posPlus_Y, osg::Vec3(0.0, -1.0, 0.0), ambient, diffuse));
//        osg::ref_ptr<osg::LightSource> pLightSource3 = DrawFunc::creteOneLight(tagLightInfo(3, posMinus_Y, osg::Vec3(0.0, 1.0, 0.0), ambient, diffuse));
//        osg::ref_ptr<osg::LightSource> pLightSource4 = DrawFunc::creteOneLight(tagLightInfo(4, posPlus_X, osg::Vec3(-1.0, 0.0, 0.0), ambient, diffuse));
//        osg::ref_ptr<osg::LightSource> pLightSource5 = DrawFunc::creteOneLight(tagLightInfo(5, posMinus_X, osg::Vec3(1.0, 0.0, 0.0), ambient, diffuse));


        pLightSwitch->addChild(pLightSource1.get());
//        pLightSwitch->addChild(pLightSource0.get());
//        pLightSwitch->addChild(pLightSource2.get());
//        pLightSwitch->addChild(pLightSource3.get());
//        pLightSwitch->addChild(pLightSource4.get());
//        pLightSwitch->addChild(pLightSource5.get());

        DrawFunc::enableNodeLight(pLightSwitch, bOn);

        pSwitch->addChild(pLightSwitch.get());
    };

    addLight(flightNodeSwitch, osg::Vec4(1.0, 1.0, 1.0, 0.5), osg::Vec4(1.0, 1.0, 1.0, 0.5),true);


    //飞机变换节点

//    osg::MatrixTransform* flightNodeTransform = new osg::MatrixTransform();


////    osg::Vec3f fscalef(0.01,0.01,0.01);
////    osg::Matrix matrixTmp = flightNodeTransform->getMatrix();

////    matrixTmp *= osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::X_AXIS);
////    matrixTmp *= osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::Y_AXIS);
////    matrixTmp *= osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::Z_AXIS);
////    matrixTmp *= osg::Matrix::scale(fscalef);
////    flightNodeTransform->setMatrix(matrixTmp);

//    flightNodeTransform->addChild(flightNode);
    osg::MatrixTransform* flightTransform = new osg::MatrixTransform();

    flightTransform->addChild(flightNodeSwitch);
    root->addChild(flightTransform);
    //引擎烟雾
    osg::MatrixTransform* fireballAndSmoke = new osg::MatrixTransform();
    createFireBall(fireballAndSmoke);

    fireballAndSmoke->setMatrix(osg::Matrix::rotate(-osg::PI / 2, 1, 0, 0) * osg::Matrix::translate(-4, -10, -3));
    flightTransform->addChild(fireballAndSmoke);
    createDarkSmoke(fireballAndSmoke);
//    osgViewer::Viewer viewer;

//    osgUtil::Simplifier simplifier(0.3f, 4.0f);
//    osgUtil::Optimizer optimzer;

//    optimzer.optimize(root.get());
//    viewer.setSceneData(root.get());

//    m_pModelWidget->pOSGManager()->getModelSenceData()->setCoordianteGroupVisiable(false,osg::Matrix::identity(), nullptr);

//    m_pModelWidget->pOSGManager()->getModelSenceData()->HideGroundScene(false);
    m_pModelWidget->pOSGManager()->getModelSenceData()->addNode(root);
}

MainWindow::~MainWindow()
{
    delete ui;
}

