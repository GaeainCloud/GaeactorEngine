#include "mainwindow.h"
#ifdef WIN32
#include "crashhelper.h"
#include <windows.h>
#endif
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication a(argc, argv);

    QString moduleName = "gaeactor-presentation-test2";
#ifdef Q_OS_WIN32
    // dump
    gaeactor_dump::CrashDumpHelper::setDump(moduleName);
#elif defined(Q_OS_LINUX)
    ///TODO:Linux
#endif
    MainWindow w;
    w.show();
    return a.exec();
}


//#include <osgEarthUtil/EarthManipulator>
//#include <osgEarth/Viewpoint>
//#include <osgViewer/Viewer>

//void switchViewpointAndZoom(osgViewer::Viewer* viewer, const osg::Vec3d& newCenter, double zoomOutDistance, double zoomInDistance, double duration)
//{
//    // 获取当前相机操作器
//    osg::ref_ptr<osgEarth::Util::EarthManipulator> manipulator = dynamic_cast<osgEarth::Util::EarthManipulator*>(viewer->getCameraManipulator());
//    if (!manipulator) {
//        manipulator = new osgEarth::Util::EarthManipulator();
//        viewer->setCameraManipulator(manipulator);
//    }

//    // 获取当前视点
//    osgEarth::Viewpoint vp = manipulator->getViewpoint();

//    // 创建新的视点，设置为新中心点，并设置拉远的距离
//    osgEarth::Viewpoint new_vp("", newCenter.x(), newCenter.y(), newCenter.z(), vp.heading()->getValue(), vp.pitch()->getValue(), zoomOutDistance);

//    // 设置视点过渡
//    manipulator->setViewpoint(new_vp, duration);

//    // 在过渡结束后，将相机拉近到指定的距离
//    osg::Timer timer;
//    while (timer.time_s() < duration) {
//        osgEarth::Viewpoint current_vp = manipulator->getViewpoint();
//        if (current_vp.range()->getValue() > zoomInDistance) {
//            osgEarth::Viewpoint updated_vp = current_vp;
//            updated_vp.range().mutable_value() = osg::clampBetween(current_vp.range()->getValue() - (zoomInDistance / duration) * timer.time_s(), zoomOutDistance, zoomInDistance);
//            manipulator->setViewpoint(updated_vp, 0.0);
//        }
//        viewer->frame();
//    }

//    // 最后设置为拉近后的距离
//    osgEarth::Viewpoint final_vp = manipulator->getViewpoint();
//    final_vp.range().mutable_value() = zoomInDistance;
//    manipulator->setViewpoint(final_vp, 0.0);
//}

//int main(int argc, char** argv)
//{
//    osgViewer::Viewer viewer;
//    // 设置场景数据
//    viewer.setSceneData(osgDB::readNodeFile("D:/earth/map.earth"));

//    // 切换视口中心到新的经纬度，并实现拉远再拉近的效果
//    osg::Vec3d newCenter(107.85, 32.35, 0); // 示例经纬度
//    double zoomOutDistance = 1000000.0; // 拉远距离
//    double zoomInDistance = 100000.0;   // 拉近距离
//    double duration = 50.0;             // 过渡时间，单位秒

//    switchViewpointAndZoom(&viewer, newCenter, zoomOutDistance, zoomInDistance, duration);

//    return viewer.run();
//}


//#include <osg/Camera>
//#include <osgEarthUtil/EarthManipulator>
//#include <osgEarth/Viewpoint>
//#include <osgGA/GUIEventAdapter>
//#include <osgGA/GUIEventHandler>
//#include <osgViewer/Viewer>
//#include <iostream>

//class CustomEventHandler : public osgGA::GUIEventHandler {
//public:
//    CustomEventHandler(osgViewer::Viewer* viewer, osg::ref_ptr<osgEarth::Util::EarthManipulator> manipulator)
//        : _viewer(viewer), _manipulator(manipulator) {}

//    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
//        if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN) {
//            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_E) {
//                switchViewpointAndZoom();
//                return true;
//            }
//        }
//        return false;
//    }

//    void switchViewpointAndZoom() {
//        // 假设新中心点和缩放距离
//        osg::Vec3d newCenter(107.85, 32.35, 0); // 示例经纬度
//        double zoomOutDistance = 10000000000.0; // 拉远距离
//        double zoomInDistance = 1000.0;   // 拉近距离
//        double duration = 50.0;             // 过渡时间，单位秒

//        // 创建新的视点，设置为新中心点，并设置拉远的距离
//        osgEarth::Viewpoint new_vp("", newCenter.x(), newCenter.y(), newCenter.z(), 0.0, 0.0, zoomOutDistance);

//        // 设置视点过渡
//        _manipulator->setViewpoint(new_vp, duration);

//        // 在过渡结束后，将相机拉近到指定的距离
//        osg::Timer timer;
//        while (timer.time_s() < duration) {
//            osgEarth::Viewpoint current_vp = _manipulator->getViewpoint();
//            if (current_vp.range()->getValue() > zoomInDistance) {
//                osgEarth::Viewpoint updated_vp = current_vp;
//                updated_vp.range().mutable_value() = osg::clampBetween(current_vp.range()->getValue() - (zoomInDistance / duration) * timer.time_s(), zoomOutDistance, zoomInDistance);
//                _manipulator->setViewpoint(updated_vp, 0.0);
//            }
//            _viewer->frame();
//        }

//        // 最后设置为拉近后的距离
//        osgEarth::Viewpoint final_vp = _manipulator->getViewpoint();
//        final_vp.range().mutable_value()=zoomInDistance;
//        _manipulator->setViewpoint(final_vp,  0.0);
//    }

//private:
//    osgViewer::Viewer* _viewer;
//    osg::ref_ptr<osgEarth::Util::EarthManipulator> _manipulator;
//};

//int main(int argc, char** argv) {
//    osgViewer::Viewer viewer;
//    // 设置场景数据
//    viewer.setSceneData(osgDB::readNodeFile("D:/earth/map.earth"));

//    // 获取当前相机操作器
//    osg::ref_ptr<osgEarth::Util::EarthManipulator> manipulator = dynamic_cast<osgEarth::Util::EarthManipulator*>(viewer.getCameraManipulator());
//    if (!manipulator) {
//        manipulator = new osgEarth::Util::EarthManipulator();
//        viewer.setCameraManipulator(manipulator);
//    }

//    // 创建自定义事件处理器并添加到视图中
//    CustomEventHandler* eventHandler = new CustomEventHandler(&viewer, manipulator);
//    viewer.addEventHandler(eventHandler);

//    return viewer.run();
//}


//#include <osg/Camera>
//#include <osg/MatrixTransform>
//#include <osgGA/GUIEventAdapter>
//#include <osgGA/GUIEventHandler>
//#include <osgViewer/Viewer>
//#include <osg/AnimationPath>
//#include <osg/Camera>
//#include <osg/io_utils>

//#include <osg/Camera>
//#include <osgEarthUtil/EarthManipulator>
//#include <osgEarth/Viewpoint>
//#include <osgGA/GUIEventAdapter>
//#include <osgGA/GUIEventHandler>
//#include <osgViewer/Viewer>
//#include <iostream>

//class ZoomOnKeyHandler : public osgGA::GUIEventHandler {
//public:
//    ZoomOnKeyHandler(osg::Camera* camera, osg::Vec3d startPt, osg::Vec3d endPt, osg::Vec3d upVector)
//        : _camera(camera), _startPt(startPt), _endPt(endPt), _upVector(upVector) {}

//    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) {

//        switch(ea.getEventType())
//        {
//    case osgGA::GUIEventAdapter::DOUBLECLICK:
//    {
//        if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
//        {
//            std::cout<<"zoom"<<std::endl;
//            createZoomAnimation();
//            return true;
//        }

//    }
//    break;
//    default:break;
//    }
////        if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN && ea.getKey() == osgGA::GUIEventAdapter::KEY_E) {
////            std::cout<<"zoom"<<std::endl;
////            createZoomAnimation();
////            return true;
////        }
//        return false;
//    }

//    void createZoomAnimation() {
//        osg::AnimationPath* path = new osg::AnimationPath;
//        path->setLoopMode(osg::AnimationPath::LOOP);

//        // Add start and end points with duration
//        path->insert(0.0, osg::AnimationPath::ControlPoint(_startPt));
//        path->insert(1.0, osg::AnimationPath::ControlPoint(_endPt));

//        // Create the callback
//        osg::ref_ptr<osg::AnimationPathCallback> pcb = new osg::AnimationPathCallback;
//        pcb->setAnimationPath(path);
//        pcb->setTimeOffset(osg::Timer::instance()->tick());
////        pcb->ssetDuration(2.0); // 2 seconds for the entire animation

//        // Add the callback to the camera to update its position
//        _camera->addUpdateCallback(pcb);
//        osg::Vec3d eye(0, 0, 100), center(0, 0, 0), up(0, 1, 0);
//        double startDistance = 100.0, endDistance = 300.0; // Start and end distances
//        double duration = 2.0; // Duration of the animation in seconds
//        // Set the initial view matrix with the start distance
//        osg::Matrixd viewMatrix;
//        viewMatrix.makeLookAt(osg::Vec3d(startDistance, 0, 0), center, up);
//        _camera->setViewMatrix(viewMatrix);
//    }

//private:
//    osg::ref_ptr<osg::Camera> _camera;
//    osg::Vec3d _startPt;
//    osg::Vec3d _endPt;
//    osg::Vec3d _upVector;
//};

//int main(int argc, char** argv) {
//    osgViewer::Viewer viewer;

//    // Set up the camera
//    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
//    camera->setViewport(0, 0, 800, 600);
//    camera->setProjectionMatrixAsPerspective(30.0, double(800) / 600.0, 1.0, 1000.0);
//    camera->setViewMatrixAsLookAt(osg::Vec3d(0, 0, 100), osg::Vec3d(0, 0, 0), osg::Vec3d(0, 1, 0));

//    // Add the camera to the viewer
//    viewer.addSlave(camera, false);

//    // Create and add the event handler
//    ZoomOnKeyHandler* handler = new ZoomOnKeyHandler(camera, osg::Vec3d(0, 0, 100), osg::Vec3d(0, 0, 300), osg::Vec3d(0, 1, 0));
//    viewer.addEventHandler(handler);

//    // Set the scene data
//    viewer.setSceneData(osgDB::readNodeFile("D:/earth/OpenSceneGraph-Data/cow.osg"));

//    return viewer.run();
//}


//#include <osgViewer/Viewer>
//#include <osg/Node>
//#include <osg/Group>
//#include <osg/MatrixTransform>
//#include <osg/Geode>
//#include <osg/Geometry>
//#include <osgDB/ReadFile>
//#include <osgUtil/Optimizer>
//#include <osg/ShapeDrawable>

//// 创建一个简单的场景
//osg::Node* createScene() {
//    osg::ref_ptr<osg::Box> box = new osg::Box(osg::Vec3(0.0, 0.0, 0.0), 1.0);
//    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
//    geode->addDrawable(new osg::ShapeDrawable(box.get()));
//    return geode.get();
//}

//// 设置相机的视角
//void setCameraView(osgViewer::Viewer* viewer, double nearDist, double farDist) {
//    osg::ref_ptr<osg::Camera> camera = viewer->getCamera();
//    if (camera.valid()) {
//        osg::Matrix viewMatrix;
//        viewMatrix.makeLookAt(osg::Vec3(0.0, 0.0, farDist), osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 1.0, 0.0));
//        camera->setViewMatrix(viewMatrix);
//        camera->setProjectionMatrixAsPerspective(30.0, static_cast<double>(camera->getGraphicsContext()->getTraits()->width) / camera->getGraphicsContext()->getTraits()->height, nearDist, farDist);
//    }
//}

//int main() {
//    osgViewer::Viewer viewer;
//    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFile("D:/earth/OpenSceneGraph-Data/cow.osg");
//    osgUtil::Optimizer optimizer;
//    optimizer.optimize(scene.get());
//    viewer.setSceneData(scene.get());
//    viewer.realize();

//    // 设置初始视角距离
//    double nearDist = 1.0;
//    double farDist = 5.0;
//    setCameraView(&viewer, nearDist, farDist);

//    // 在两个距离值之间切换
//    bool zoomIn = true;
//    osg::Timer timer;
//    timer.setStartTick();
//    int cc = 0;
//    while (!viewer.done()) {
//        viewer.frame();
//        cc++;
//        if (timer.time_s() > 0.1)
//        { // 切换视角的时间间隔
//            timer.setStartTick();
//            if (zoomIn) {
//                farDist += 1.0; // 拉远
//            } else {
//                farDist -= 1.0; // 拉近
//            }
//            if(cc%20==0)
//            {
//            zoomIn = !zoomIn; // 切换拉近拉远的状态
//            }
//            setCameraView(&viewer, nearDist, farDist);
//        }
//    }

//    return 0;
//}
