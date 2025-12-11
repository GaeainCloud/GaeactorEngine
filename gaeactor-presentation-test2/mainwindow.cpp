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
#include <osg/Multisample>
#include <osg/Point>



#include "Depictor.h"
#include "SpaceGridEngine.h"
#include "SpaceGridEngineGlobals.h"


//#define GLOBAL_RADIUS_SCALE (10)
//#define SUN_SACLE   (10)

#define GLOBAL_RADIUS_SCALE (1)
#define SUN_SACLE   (1)
#ifdef USING_SCALE_1000
#define ST (1000)
#else
#define ST (1)
#endif
#define ST_RADIUS (696340.0f)


#define GRID_MESH_PICK_COLOR (osg::Vec4(0.0, 1.0, 1.0, 1.0))
#define GRID_MESH_PT_COLOR (osg::Vec4(0.0,0.0,1.0,1.0))
#define GRID_MESH_LINE_COLOR (osg::Vec4(0.0, 1.0, 0.0, 1.0))

#define PHYSICS_RADIUS_TO_COORDINATE_RADIUS(RADIUS) ((RADIUS)/(ST_RADIUS/ST)*(GLOBAL_RADIUS_SCALE))

#define ORBIT_PHYSICS_RADIUS_TO_COORDINATE_RADIUS(RADIUS) PHYSICS_RADIUS_TO_COORDINATE_RADIUS(RADIUS)

int get_sun_radius_num(const double& radius_len)
{
    return (int)((radius_len - ST_RADIUS)/(ST_RADIUS*2))/*+1*/;
}


#include <osg/BlendFunc>

void MainWindow::setTex(osg::Node* pNode, const std::string &path)
{
//    return;
    osg::ref_ptr<osg::Image> rpImage = osgDB::readImageFile(path);
    osg::ref_ptr<osg::Texture2D> rpTexture = new osg::Texture2D();

    rpTexture->setImage(rpImage);
    rpTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);//设置S方向的环绕模式
    rpTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);//设置R方向的环绕模式

    osg::ref_ptr<osg::StateSet> pState = pNode->getOrCreateStateSet();
    pState->setTextureAttributeAndModes(0, rpTexture, osg::StateAttribute::ON);

    //关闭光照确保纹理可见
    pState->setMode(GL_LIGHTING, osg::StateAttribute::OFF);



//    pState->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
//    osg::ref_ptr<osg::Material> material = new osg::Material;
//    material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 0.1f)); // 设置透明度为0.5
//    material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 0.1f));
//    material->setTransparency(osg::Material::FRONT_AND_BACK,25.5f);
//    pState->setAttribute(material.get(), osg::StateAttribute::OVERRIDE);

//    pState->setMode(GL_BLEND, osg::StateAttribute::ON);
//    pState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

//    // 设置混合函数
//    pState->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);


}

void MainWindow::updatePlanetGridMesh(const std::string& planetname, std::vector<std::vector<tagpos> > &_datas)
{
    auto planetNode = getPlanet(planetname);
    if(planetNode.has_value() && nullptr != planetNode.value())
    {
        const tagPlanetNode &_tagPlanetNode = *planetNode.value();
        osg::ref_ptr<osg::Switch> _planetGridGroup = _tagPlanetNode.planetGridGroup;
        if(_planetGridGroup.valid())
        {
            std::vector<DrawFunc::tagpos> all_pt_datas;
            for(int i = 0;i < _datas.size(); i++)
            {
                all_pt_datas.reserve(all_pt_datas.size() + _datas.at(i).size());
                all_pt_datas.insert(all_pt_datas.end(), _datas.at(i).begin(), _datas.at(i).end());
            }

            const double& dRadius = _tagPlanetNode._planet_coordinate_radius;
            osg::Vec3 center = osg::Vec3(0,0,0);

            auto redrawLinePt=[](osg::Node*  pNode,
                                   const std::vector<DrawFunc::tagpos> & vertexes,
                                   const double& dRadius,
                                   GLenum mode = GL_LINE_STRIP,
                                   const osg::Vec3& center_ = osg::Vec3(0,0,0),
                                   double scaleoffset = 1.0)
            {
                if(pNode)
                {
                    osg::Geode* pGeode = pNode->asGeode();
                    if(pGeode)
                    {
                        osg::Drawable * pGeomDrawable = pGeode->getDrawable(0);
                        if(pGeomDrawable)
                        {
                            osg::Geometry*  pGeom = pGeomDrawable->asGeometry();
                            if(pGeom)
                            {
                                osg::Vec3Array* pVertexes = nullptr;
                                osg::DrawArrays* pDrawArrays = nullptr;
                                osg::Array * pVertexesArray = pGeom->getVertexArray();
                                if(pVertexesArray)
                                {
                                    pVertexes = dynamic_cast<osg::Vec3Array*>(pVertexesArray);
                                }

                                osg::PrimitiveSet * pPrimitiveSet = pGeom->getPrimitiveSet(0);
                                if(pPrimitiveSet)
                                {
                                    pDrawArrays = dynamic_cast<osg::DrawArrays*>(pPrimitiveSet);
                                }

                                if(pVertexes && pDrawArrays)
                                {
                                    pVertexes->clear();

                                    for(int i = 0; i < vertexes.size(); i++)
                                    {
                                        pVertexes->push_back(osg::Vec3((center_.x() + vertexes[i].x) * dRadius * scaleoffset, (center_.y() + vertexes[i].y) * dRadius * scaleoffset, (center_.z() + vertexes[i].z) * dRadius * scaleoffset));
                                    }

                                    pVertexes->dirty();
                                    pDrawArrays->set(mode, 0, pVertexes->size());
                                    pGeom->dirtyGLObjects();
                                    pGeom->dirtyDisplayList();
                                    pGeom->dirtyBound();
                                    pGeode->dirtyBound();
                                }
                            }
                        }
                    }
                }
            };

            osg::Node*  pPtNode = _planetGridGroup->getChild(0);
            if(pPtNode)
            {
                redrawLinePt(pPtNode, all_pt_datas, dRadius, GL_POINTS, center);
            }
#if 0
            _planetGridGroup->removeChildren(1,_planetGridGroup->getNumChildren()-1);

            std::vector<osg::Vec4> cls;
            cls.push_back(osg::Vec4(0.0, 0.0, 1.0, 1.0));
            double lineWidth = 1.0;
            for(int i = 0; i < _datas.size(); i++)
            {
                osg::Geode*  pLineGeode = DrawFunc::createSphereGeometryEx(_datas.at(i), dRadius, GL_LINE_STRIP, center, cls, lineWidth);
                if(pLineGeode)
                {
                    pLineGeode->setName("geo_line"+QString::number(i).toStdString());
                    _planetGridGroup->addChild(pLineGeode);
                }
            }
#else
            osg::Node*  pLineNode = _planetGridGroup->getChild(1);
            if(pLineNode)
            {
                redrawLinePt(pLineNode, all_pt_datas, dRadius, GL_LINE_STRIP, center);
            }
#endif
        }
    }
}

void MainWindow::updatePlanetTracking(const std::string &planetname, const osg::Vec3f &position)
{
    auto planetNode = getPlanet(planetname);
    if(planetNode.has_value() && nullptr != planetNode.value())
    {
        const tagPlanetNode &_tagPlanetNode = *planetNode.value();
        osg::ref_ptr<osg::MatrixTransform> planetTransform = _tagPlanetNode.planetTransform;
        if(planetTransform.valid())
        {
#if 0
            osg::Vec3f center = planetTransform->getBound().center();
            osg::Matrix matrixTmp = planetTransform->getMatrix();
            //先平移到原点
            matrixTmp *= osg::Matrix::translate(-center);
#else
            osg::Matrix matrixTmp = osg::Matrix::identity();
#endif
            matrixTmp *= osg::Matrix::translate(position);
//            matrixTmp *= osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::Z_AXIS);
            planetTransform->setMatrix(matrixTmp);
        }
    }
}


void MainWindow::translatePlanetNode(const std::string &planetname,const osg::Vec3f &transPos)
{
    auto planetNode = getPlanet(planetname);
    if(planetNode.has_value() && nullptr != planetNode.value())
    {
        const tagPlanetNode &_tagPlanetNode = *planetNode.value();
        osg::ref_ptr<osg::MatrixTransform> planetTransform = _tagPlanetNode.planetTransform;
        if(planetTransform.valid())
        {
            osg::Matrix matrixTmp = planetTransform->getMatrix();
            matrixTmp *= osg::Matrix::translate(osg::Vec3f(transPos));
            planetTransform->setMatrix(matrixTmp);
        }
    }
}

void MainWindow::rotatePlanetNode(const std::string &planetname,const osg::Vec3f &eulerAngles)
{
    auto planetNode = getPlanet(planetname);
    if(planetNode.has_value() && nullptr != planetNode.value())
    {
        const tagPlanetNode &_tagPlanetNode = *planetNode.value();
        osg::ref_ptr<osg::MatrixTransform> planetTransform = _tagPlanetNode.planetTransform;
        if(planetTransform.valid())
        {
            osg::Vec3f center = planetTransform->getBound().center();

            osg::Matrix matrixTmp = planetTransform->getMatrix();
            //先平移到原点
            matrixTmp *= osg::Matrix::translate(-center);

            //根据欧拉角计算四元数
            auto quat = DrawFunc::getquat(glm::vec3(osg::DegreesToRadians(eulerAngles.x()), osg::DegreesToRadians(eulerAngles.y()), osg::DegreesToRadians(eulerAngles.z())));


#ifdef ROTATE_QUAT
            //            osg::Quat osgquat;
            //            osgquat.makeRotate(osg::DegreesToRadians(eulerAngles.x()), osg::X_AXIS,
            //                               osg::DegreesToRadians(eulerAngles.y()), osg::Y_AXIS,
            //                               osg::DegreesToRadians(eulerAngles.z()), osg::Z_AXIS);
            auto osgquat = osg::Quat(quat.x, quat.y, quat.z, quat.w);
            matrixTmp *= osg::Matrix::rotate(osgquat);
#else
            matrixTmp *= osg::Matrix::rotate(osg::DegreesToRadians(eulerAngles.x()), osg::X_AXIS);
            matrixTmp *= osg::Matrix::rotate(osg::DegreesToRadians(eulerAngles.y()), osg::Y_AXIS);
            matrixTmp *= osg::Matrix::rotate(osg::DegreesToRadians(eulerAngles.z()), osg::Z_AXIS);
#endif
            //从原点平移到原来的位置
            matrixTmp *= osg::Matrix::translate(center);
            planetTransform->setMatrix(matrixTmp);
        }
    }

}

void MainWindow::scalePlanetNode(const std::string &planetname,const osg::Vec3f &fscalef)
{
    auto planetNode = getPlanet(planetname);
    if(planetNode.has_value() && nullptr != planetNode.value())
    {
        const tagPlanetNode &_tagPlanetNode = *planetNode.value();
        osg::ref_ptr<osg::MatrixTransform> planetTransform = _tagPlanetNode.planetTransform;
        if(planetTransform.valid())
        {
            osg::Matrix matrixTmp = planetTransform->getMatrix();
            matrixTmp *= osg::Matrix::scale(fscalef);
            planetTransform->setMatrix(matrixTmp);
        }
    }
}


osg::Switch *MainWindow::createSphereGeometryExs(std::vector<std::vector<std::tuple<osg::Vec3, osg::Vec4>> > &_datas, const std::string& planetname, const double &dRadius, const osg::Vec3 &center, const osg::Vec4 &cl, const double &lineWidth, bool bVisiable)
{
    // if(_datas.empty())
    // {
    //     return nullptr;
    // }
    osg::ref_ptr<osg::Switch> pGroup = new osg::Switch();

    std::vector<osg::Vec3> all_pt_datas;
    std::vector<osg::Vec4> ptcls;
    std::vector<osg::Vec4> linecls;
    for(int i = 0;i < _datas.size(); i++)
    {
        all_pt_datas.reserve(all_pt_datas.size() + _datas.at(i).size());
        ptcls.reserve(ptcls.size() + _datas.at(i).size());
        linecls.reserve(linecls.size() + _datas.at(i).size());
        for(int j = 0; j < _datas.at(i).size(); j++)
        {
            all_pt_datas.push_back(std::get<0>(_datas.at(i).at(j)));
            ptcls.push_back(GRID_MESH_PT_COLOR);
            linecls.push_back(GRID_MESH_LINE_COLOR);
        }
    }

    osg::Geode*  pPtGeode = DrawFunc::createSphereGeometryEx(all_pt_datas, dRadius, GL_POINTS, std::vector<UINT32>(), center, ptcls, lineWidth*4);
    if(pPtGeode)
    {
        pPtGeode->setName("geo_pts_"+planetname);
        pGroup->addChild(pPtGeode);
    }



#if 1
    osg::Geode*  pLineGeode = DrawFunc::createSphereGeometryEx(all_pt_datas, dRadius, GL_LINE_STRIP, std::vector<UINT32>(), center, linecls, lineWidth);
    if(pLineGeode)
    {
        pLineGeode->setName("geo_line_"+planetname);
        pGroup->addChild(pLineGeode);
    }
#else
    for(int i = 0;i < _datas.size(); i++)
    {
        osg::Geode*  pLineGeode = DrawFunc::createSphereGeometryEx(_datas.at(i), dRadius, GL_LINE_STRIP, center, linecls, lineWidth);
        if(pLineGeode)
        {
            pLineGeode->setName("geo_line"+QString::number(i).toStdString());
            pGroup->addChild(pLineGeode);
        }
    }
#endif
    // 默认不显示
    if(!bVisiable)
    {
        pGroup->setAllChildrenOff();
    }
    else
    {
        pGroup->setAllChildrenOn();
    }
    pGroup->setName("gridgroupswitch_"+planetname);
    return pGroup.release();
}

#define ENABLE_PLANET_SCALE

tagPlanetNode * MainWindow::createPlanetGroup(const std::string& planetname, const std::string& texturePath, const osg::Vec3& place, const double& _planet_orbit_radius_km, const double& _planet_radius_km, const bool& bShoworbit, const bool& planet, const osg::Vec3& center/* = osg::Vec3(0, 0, 0)*/)
{    
    tagPlanetNode *pPlanetNode = nullptr;
    if(m_PlanetNodes.find(planetname) == m_PlanetNodes.end())
    {
        tagPlanetNode _tagPlanetNode;
        _tagPlanetNode.planetName = planetname;
        m_PlanetNodes.insert(std::make_pair(planetname, std::move(_tagPlanetNode)));
        std::cout<<" createPlanetGroup "<<planetname<<std::endl;

    }

    pPlanetNode = &m_PlanetNodes.at(planetname);

    if(pPlanetNode)
    {
        ///////////////////////////////////////////////////////////////////////////////////////////
        pPlanetNode->_planet_coordinate_radius = PHYSICS_RADIUS_TO_COORDINATE_RADIUS(_planet_radius_km);
#ifdef ENABLE_PLANET_SCALE
        if(planetname != "Sun")
        {
            if(planet)
            {
                pPlanetNode->_planet_coordinate_radius = pPlanetNode->_planet_coordinate_radius*30;
            }
            else
            {
                pPlanetNode->_planet_coordinate_radius = pPlanetNode->_planet_coordinate_radius*100;
            }
        }
#endif
        pPlanetNode->_planet_orbit_coordinate_radius = PHYSICS_RADIUS_TO_COORDINATE_RADIUS(_planet_orbit_radius_km);
        ///////////////////////////////////////////////////////////////////////////////////////////

        //    std::cout<<" makePlanetGroup "<<name<<" _planet_coordinate_radius "<<pPlanetNode->_planet_coordinate_radius<<"\n";
        pPlanetNode->planetTransform = new osg::MatrixTransform;
        pPlanetNode->planetTransform->setMatrix(osg::Matrix::translate(place));

        pPlanetNode->planetGeode = new osg::Geode;
        pPlanetNode->planetGeode->setName(planetname+"_Geode");
        pPlanetNode->planetTransform->setName(planetname+"_Transform");

        // 创建星球球体
        pPlanetNode->planetGeometry = DrawFunc::createSphereGeometry(osg::Vec3(0, 0, 0), pPlanetNode->_planet_coordinate_radius, 45);
        pPlanetNode->planetGeometry->setName(planetname+"_Geometry");
        pPlanetNode->planetGeode->addChild(pPlanetNode->planetGeometry);

        ///////////////////////////////////////////////////////////////////////////////////////////

        // 设置星球纹理
        setTex(pPlanetNode->planetGeode, texturePath); // 在球上贴纹理
        ///////////////////////////////////////////////////////////////////////////////////////////

        //当是地球时，添加月球
        auto iitor = m_planetParams.find(planetname);
        if(iitor != m_planetParams.end() && !iitor->second.satellites.empty())
        {
            QString nodepath = QCoreApplication::applicationDirPath() + QString::fromStdString("/res/osgEarth/Star/moon.jpg");

            pPlanetNode->subPlanets.reserve(iitor->second.satellites.size());
            auto itor = iitor->second.satellites.begin();
            while(itor != iitor->second.satellites.end())
            {
                tagPlanetNode* pSubPlanetNode = createPlanetGroup(itor->first,
                                                                nodepath.toStdString(),
                                                                  osg::Vec3(ORBIT_PHYSICS_RADIUS_TO_COORDINATE_RADIUS(itor->second.track_km()), 0, 0),
                                                                itor->second.track_km(),
                                                                itor->second.radius,
                                                                true,false);
                pPlanetNode->planetTransform->addChild(pSubPlanetNode->planetGroup);

                pSubPlanetNode->planetext = m_pModelWidget->pOSGManager()->getModelSenceData()->showText(itor->first, osg::Vec3(ORBIT_PHYSICS_RADIUS_TO_COORDINATE_RADIUS(itor->second.track_km()), 0, 0), itor->first, osg::Vec4(0.0,1.0,1.0,1.0),0.5);
                pSubPlanetNode->planetext->setName("text_switch_"+itor->first);
                pPlanetNode->planetTransform->addChild(pSubPlanetNode->planetext);

                pPlanetNode->subPlanets.push_back(pSubPlanetNode);
                itor++;
            }
        }
        ///////////////////////////////////////////////////////////////////////////////////////////

        pPlanetNode->planetGridGroup = createSphereGeometryExs(m_datas,planetname, pPlanetNode->_planet_coordinate_radius);
        if(pPlanetNode->planetGridGroup)
        {
            pPlanetNode->planetTransform->addChild(pPlanetNode->planetGridGroup);
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        pPlanetNode->planetAnchorGroup = createAnchorGroup(planetname,pPlanetNode->_planet_coordinate_radius);
        if(pPlanetNode->planetAnchorGroup)
        {
            pPlanetNode->planetTransform->addChild(pPlanetNode->planetAnchorGroup);
        }
        ///////////////////////////////////////////////////////////////////////////////////////////

        pPlanetNode->planetTransform->addChild(pPlanetNode->planetGeode);
        ///////////////////////////////////////////////////////////////////////////////////////////

        pPlanetNode->planetGeode->setUserValue<double>(PLANET_RADIUS_KM, _planet_radius_km);
        pPlanetNode->planetGeode->setUserValue<double>(PLANET_RADIUS_COORDINATE, pPlanetNode->_planet_coordinate_radius);

        pPlanetNode->planetGeode->setUserValue<double>(PLANET_ORBIT_RADIUS_KM, _planet_orbit_radius_km);
        pPlanetNode->planetGeode->setUserValue<double>(PLANET_ORBIT_RADIUS_COORDINATE, pPlanetNode->_planet_orbit_coordinate_radius);


        if(planet)
        {
            pPlanetNode->planetGeode->setUserValue(DISPLAY_MODEL_TYPE, 1);
        }
        else
        {
            pPlanetNode->planetGeode->setUserValue(DISPLAY_MODEL_TYPE, 0);
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        // 轨道设置
        pPlanetNode->orbitGeode = new osg::Geode;
        pPlanetNode->orbitGeode->setName(planetname+"_Geode_orbitGeometry");
        pPlanetNode->orbitGeometry = DrawFunc::createOrbit(center, place.x() - center.x() < 0 ? center.x() - place.x() : place.x() - center.x(), 50, osg::Vec4(0.5f, 0.7f, 0.8f, 1.f));
        pPlanetNode->orbitGeode->addChild(pPlanetNode->orbitGeometry);

        // 默认不显示
        pPlanetNode->orbitControl = new osg::Switch();
        if(!bShoworbit)
        {
            pPlanetNode->orbitControl->setAllChildrenOff();
        }
        else
        {
            pPlanetNode->orbitControl->setAllChildrenOn();
        }
        pPlanetNode->orbitControl->setName(planetname+"_orbit");
        pPlanetNode->orbitControl->addChild(pPlanetNode->orbitGeode);

        ///////////////////////////////////////////////////////////////////////////////////////////
        // 添加到星球组节点
        pPlanetNode->planetGroup = new osg::Group;
        pPlanetNode->planetGroup->setName(planetname+"_Group");
        pPlanetNode->planetGroup->addChild(pPlanetNode->planetTransform);
        pPlanetNode->planetGroup->addChild(pPlanetNode->orbitControl);
    }
    return pPlanetNode;
}


osg::Group *MainWindow::createStonesBetween(float nearadius, float farradius)
{
    // 创建一个组来容纳所有的陨石
    osg::ref_ptr<osg::Group> stonesGroup = new osg::Group();

    // 设置随机数种子
    srand(static_cast<unsigned int>(time(nullptr)));

    // 生成100个陨石
    for (int i = 0; i < 200; ++i) {
        // 随机生成石头的半径
        float radius = static_cast<float>(nearadius + (farradius - nearadius) * (rand() / static_cast<float>(RAND_MAX)));

        // 随机生成石头的角度
        float angle = static_cast<float>(rand() / static_cast<float>(RAND_MAX) * 2 * osg::PI);

        // 计算石头的x和y坐标
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        float z = 0.0f; // z坐标固定为0


        std::random_device rd;  // 用于获取随机数种子
        std::mt19937 gen(rd()); // 伪随机数生成器
        std::uniform_real_distribution<> dis(0.0001, 0.0005); // 定义分布范围
        float radiusst = dis(gen);// 生成随机浮点数
        // 将陨石节点添加到组中
        QString nodepath = QCoreApplication::applicationDirPath() + QString::fromStdString("/res/osgEarth/Star/moon.jpg");

        tagPlanetNode* pSubPlanetNode = createPlanetGroup("stones" + std::to_string(i), nodepath.toStdString(), osg::Vec3(x, y, z),x, radiusst,false,false);
        stonesGroup->addChild(pSubPlanetNode->planetGroup);
    }

    return stonesGroup.release();
}


void MainWindow::loadPositionsFromFile(const QString& path, std::vector<std::vector<std::tuple<osg::Vec3, osg::Vec4> > > &_datas)
{
    auto loadFile=[&_datas](const std::string& filename)
    {
        std::ifstream file(filename);
        std::string line;

        if (!file.is_open())
        {
            std::cerr << "Unable to open file: " << filename << std::endl;
        }
        std::vector<std::tuple<osg::Vec3, osg::Vec4>> positions;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            DrawFunc::tagpos pos;
            if (!(iss >> pos.x >> pos.y >> pos.z))
            {
                std::cerr << "Error reading line: " << line << std::endl;
                continue;
            }
            osg::Vec3 pt(pos.x, pos.y, pos.z);
            positions.push_back(std::make_tuple(pt, GRID_MESH_PICK_COLOR));
        }

        file.close();
        if(!positions.empty())
        {
            _datas.push_back(std::move(positions));
        }
    };

    QDir dir(path);

    // 检查路径是否存在且是一个目录
    if (!dir.exists())
    {
        return;
    }

    // 获取目录中的所有条目
    QFileInfoList entries ;
    entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &entry : entries)
    {
        if (entry.isDir())
        {
            loadPositionsFromFile(entry.absoluteFilePath(),_datas);
        }
        else
        {
            loadFile(entry.absoluteFilePath().toStdString());
        }
    }
}

osg::Switch *MainWindow::createAnchorGroup(const std::string &planetname, const double& dRadius, bool bVisiable)
{
    osg::ref_ptr<osg::Switch> pGroup = new osg::Switch();
    {
        std::vector<osg::Vec3> vertexes;
        vertexes.push_back(osg::Vec3(0,0,0));
        vertexes.push_back(osg::Vec3(1,0,0));
        vertexes.push_back(osg::Vec3(0,1,0));
        vertexes.push_back(osg::Vec3(0,0,1));

        std::vector<osg::Vec4> cls;
        cls.push_back(osg::Vec4(1.0, 1.0, 1.0, 1.0));
        cls.push_back(osg::Vec4(1.0, 0.0, 0.0, 1.0));
        cls.push_back(osg::Vec4(0.0, 1.0, 0.0, 1.0));
        cls.push_back(osg::Vec4(0.0, 0.0, 1.0, 1.0));
        osg::Geode*  pGeode = DrawFunc::createSphereGeometryEx(vertexes, dRadius, GL_POINTS, std::vector<UINT32>(), osg::Vec3(0,0,0), cls, 2*4);
        if(pGeode)
        {
            pGeode->setName("anchor_geo_pt_x_y_z");
            pGroup->addChild(pGeode);
        }
    }

    {
        std::vector<osg::Vec3> vertexes;
        vertexes.push_back(osg::Vec3(0,0,0));
        vertexes.push_back(osg::Vec3(2,0,0));
        vertexes.push_back(osg::Vec3(0,2,0));
        vertexes.push_back(osg::Vec3(0,0,2));

        std::vector<osg::Vec4> cls;
        cls.push_back(osg::Vec4(1.0, 1.0, 1.0, 1.0));
        cls.push_back(osg::Vec4(1.0, 0.0, 0.0, 1.0));
        cls.push_back(osg::Vec4(0.0, 1.0, 0.0, 1.0));
        cls.push_back(osg::Vec4(0.0, 0.0, 1.0, 1.0));
        std::vector<UINT32> indices;
        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(0);
        indices.push_back(3);
        osg::Geode*  pLineGeode = DrawFunc::createSphereGeometryEx(vertexes, dRadius, GL_LINES, indices, osg::Vec3(0,0,0), cls, 1);
        if(pLineGeode)
        {
            pLineGeode->setName("anchor_geo_line");
            pGroup->addChild(pLineGeode);
        }
    }
    // 默认不显示
    if(!bVisiable)
    {
        pGroup->setAllChildrenOff();
    }
    else
    {
        pGroup->setAllChildrenOn();
    }
    pGroup->setName("anchorgroupswitch_"+planetname);
    return pGroup.release();
}

void MainWindow::initPlanetInfo()
{
    m_planetParams.insert(std::make_pair("Sun",     tagParam{"Sun",     696340,     0,          0,      226000000,      0,              false,  0}));
    //水星
    m_planetParams.insert(std::make_pair("Mercury", tagParam{"Mercury", 2440,       0.3871,     47.89,  87.97,          58.653485,      false,  0}));
    //金星
    m_planetParams.insert(std::make_pair("Venus",   tagParam{"Venus",   6052,       0.7233,     35.03,  224.701,        243.02,         true,   0}));
    //地球 23小时56分钟为1天
    m_planetParams.insert(std::make_pair("Earth",   tagParam{"Earth",   6378,       1.0,        29.79,  365.26,         1,              true,   1}));
    //月球
    m_planetParams.at("Earth").satellites.insert(std::make_pair("Moon",    tagParam{"moon",    1738,       0.0025695552898,     1.023,  27.32,          27.32,          true,   0}));
    //火星
    m_planetParams.insert(std::make_pair("Mars",    tagParam{"Mars",    3397,       1.5237,     24.13,  686.98,         1.024305556,    true,   2}));
    //火卫一
    m_planetParams.at("Mars").satellites.insert(std::make_pair("Phobos",    tagParam{"phobos",    11.2,       9377/AU_KM,     1.35,  0.3179,         0.3179,    true,   0}));
    //火卫二
    m_planetParams.at("Mars").satellites.insert(std::make_pair("Deimos",    tagParam{"deimos",    6.3,       23458/AU_KM,     1.35,  1.26244,         1.26244,    true,   0}));
    //木星
    m_planetParams.insert(std::make_pair("Jupiter", tagParam{"Jupiter", 71492,      5.2026,     13.06,  4332.589,       0.409722222,    true,   67}));
    //木卫一
    m_planetParams.at("Jupiter").satellites.insert(std::make_pair("Io",    tagParam{"Io",    1821.3,       421700/AU_KM,     17.334,  1.77,         1.77,    true,   0}));
    //木卫二
    m_planetParams.at("Jupiter").satellites.insert(std::make_pair("Europa",    tagParam{"Europa",    1560.8,      671034/AU_KM,     13.7,  3.55,         3.55,    true,   0}));
    //木卫三
    m_planetParams.at("Jupiter").satellites.insert(std::make_pair("Ganymede",    tagParam{"Ganymede",    2631.2,       1070412/AU_KM,     10.880,  7.15,         7.15,    true,   0}));
    //木卫四
    m_planetParams.at("Jupiter").satellites.insert(std::make_pair("Ganymede",    tagParam{"Callisto",    2410.3,       1882709/AU_KM,     8.204,  717.3,         717.3,    true,   0}));
    //土星
    m_planetParams.insert(std::make_pair("Saturn",  tagParam{"Saturn",  60268,      9.5549,     9.64,   10759.5,        0.434722222,    false,  62}));
    //土卫二
    m_planetParams.at("Saturn").satellites.insert(std::make_pair("Enceladus",    tagParam{"Enceladus",    252.1,       237948/AU_KM,     10.880,  1.370833333,        1.370833333,    true,   0}));
    //土卫六
    m_planetParams.at("Saturn").satellites.insert(std::make_pair("Titan",    tagParam{"Titan",    2575,       1221850/AU_KM,     10.880,  15.95,         15.95,    true,   0}));
    //土卫九
    m_planetParams.at("Saturn").satellites.insert(std::make_pair("Phoebe",    tagParam{"Phoebe",    110,       12952000/AU_KM,     0.0697,  547.5,         0.4,    true,   0}));
    //天王星
    m_planetParams.insert(std::make_pair("Uranus",  tagParam{"Uranus",  25559,      19.2184,    6.81,   30799.095,      0.715277778,    true,   27}));

    //天卫一
    m_planetParams.at("Uranus").satellites.insert(std::make_pair("Titania",    tagParam{"Titania",    788.9,       436300/AU_KM,    0,  13.46,        13.46,    true,   0}));
    //天卫二
    m_planetParams.at("Uranus").satellites.insert(std::make_pair("Oberon",    tagParam{"Oberon",    761.4,       583500/AU_KM,     0,  8.706,         8.706,    true,   0}));
    //天卫三
    m_planetParams.at("Uranus").satellites.insert(std::make_pair("Umbriel",    tagParam{"Umbriel",    584.7,       266000/AU_KM,     0,  4.144,         4.144,    true,   0}));
    //天卫四
    m_planetParams.at("Uranus").satellites.insert(std::make_pair("Ariel",    tagParam{"Ariel",    578.9,       190900/AU_KM,     0,  2.52,        2.52,    true,   0}));
    //天卫五
    m_planetParams.at("Uranus").satellites.insert(std::make_pair("Miranda",    tagParam{"Miranda",    235.7,       129900/AU_KM,     0,  1.413,         1.413,    true,   0}));

    //海王星
    m_planetParams.insert(std::make_pair("Neptune", tagParam{"Neptune", 24764,      30.1104,    5.43,   60202.1532,     0.67125,        false,  14}));
    //海卫一
    m_planetParams.at("Neptune").satellites.insert(std::make_pair("Triton",    tagParam{"Triton",    1353,       354759/AU_KM,    0,  5.877,        5.877,    true,   0}));
    //海卫二
    m_planetParams.at("Neptune").satellites.insert(std::make_pair("Nereid",    tagParam{"Nereid",    340,       5513000/AU_KM,    0,  360.136,        0,    true,   0}));
    //海卫三
    m_planetParams.at("Neptune").satellites.insert(std::make_pair("Naiad",    tagParam{"Naiad",    66,       48227/AU_KM,    0,  0,        0,    true,   0}));
    //海卫四
    m_planetParams.at("Neptune").satellites.insert(std::make_pair("Thalassa",    tagParam{"Thalassa",    82,       500000/AU_KM,    0,  1.57,        0,    true,   0}));
    //海卫五
    m_planetParams.at("Neptune").satellites.insert(std::make_pair("Despina",    tagParam{"Despina",    75,       53200/AU_KM,    0,  0.335,        0,    true,   0}));
    //海卫六
    m_planetParams.at("Neptune").satellites.insert(std::make_pair("Galatea",    tagParam{"Galatea",    176,       61947/AU_KM,    0,  0.429,        0,    true,   0}));
    //海卫七
    m_planetParams.at("Neptune").satellites.insert(std::make_pair("Larissa",    tagParam{"Larissa",    194,       73548/AU_KM,    0,  0.555,        0,    true,   0}));
    //海卫八
    m_planetParams.at("Neptune").satellites.insert(std::make_pair("Proteus",    tagParam{"Proteus",    420,       117646/AU_KM,    0,  1.122,        0,    true,   0}));
    //海卫九
    m_planetParams.at("Neptune").satellites.insert(std::make_pair("Halimede",    tagParam{"Halimede",    62,       16611000/AU_KM,    0,  1.122,        0,    true,   0}));

}

void MainWindow::adjustPlanetGridMesh(const std::string &planetname,bool bPlus)
{
    auto planetNode = getPlanet(planetname);
    if(planetNode.has_value() && nullptr != planetNode.value())
    {
        tagPlanetNode &_tagPlanetNode = *planetNode.value();

        UINT32 old_nSeg = _tagPlanetNode.m_nSeg;

        UINT32 &_nSeg = _tagPlanetNode.m_nSeg;
        if(bPlus)
        {
            _nSeg++;
        }
        else
        {
            _nSeg --;
        }
        _nSeg = _nSeg < 1 ? 1:_nSeg;
        _nSeg = _nSeg >1024 ? 1024:_nSeg;

        if(old_nSeg != _nSeg)
        {
            std::cout<<" adjust node "<<m_cur_focus_planet<<" mesh seg to "<<_nSeg<<std::endl;

            std::vector<std::vector<DrawFunc::tagpos>> _datas;

            getSpherePremeshData(_datas,_nSeg,m_nPointEachEdge2);
            updatePlanetGridMesh(m_cur_focus_planet,_datas);
        }
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        switch (keyEvent->key())
        {
        case Qt::Key_Plus:
        {
            adjustPlanetGridMesh(m_cur_focus_planet, true);
        }
        break;
        case Qt::Key_Minus:
        {
            adjustPlanetGridMesh(m_cur_focus_planet, false);
        }
        break;
        default: {
        }
        }
    }

    return QMainWindow::eventFilter(watched,event);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->resize(1400,1200);

    this->installEventFilter(this);
    initPlanetInfo();
//    m_datas.clear();
//    getSpherePremeshData(m_datas,m_nSeg,m_nPointEachEdge2);

    m_pModelWidget = new QtOSGWidget(QtOSGWidget::E_OSG_SHOW_TYPE_MODEL2, this);
    this->setCentralWidget(m_pModelWidget);
    m_pModelWidget->setWidgetCallback(std::bind(&MainWindow::widget_callback,this,std::placeholders::_1));
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    updatePlanetRunningOrbitTracking();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::string sunplanetname = "Sun";

    osg::Vec3 center = osg::Vec3(0, 0, 0);
    QString nodepath = QCoreApplication::applicationDirPath() + QString::fromStdString("/res/osgEarth/Star/");


    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    tagPlanetNode *pSunPlanetNode = createPlanetGroup(sunplanetname,
                                                    nodepath.toStdString()+sunplanetname+".jpeg",
                                                      osg::Vec3(ORBIT_PHYSICS_RADIUS_TO_COORDINATE_RADIUS(m_planetParams.at(sunplanetname).track_km()), 0, 0),
                                                    m_planetParams.at(sunplanetname).track_km(),
                                                    m_planetParams.at(sunplanetname).radius,
                                                    true,
                                                    true);;
    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    pSunPlanetNode->planetext = m_pModelWidget->pOSGManager()->getModelSenceData()->showText(sunplanetname, center, sunplanetname, osg::Vec4(0.0,1.0,0.0,1.0),1.5);
    pSunPlanetNode->planetext->setName("text_switch_"+sunplanetname);
    pSunPlanetNode->planetTransform->addChild(pSunPlanetNode->planetext);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(pSunPlanetNode)
    {
#if 0
    auto appendSun=[&](const osg::Vec3& center)->osg::ref_ptr<osg::Geode>
    {
        // 新建太阳
        osg::ref_ptr<osg::Geode> sun = new osg::Geode;
        sun->setName("sun_Geode");
        double dCoordinateRadius = ST*SUN_SACLE;
        osg::ref_ptr<osg::Geometry> planetGeometry = DrawFunc::createSphereGeometry(center,dCoordinateRadius, 45);
        planetGeometry->setName("sun_Geometry");
        sun->addChild(planetGeometry);

        sun->setUserValue<double>(PLANET_RADIUS_KM, ST_RADIUS);
        sun->setUserValue<double>(PLANET_RADIUS_COORDINATE, dCoordinateRadius);

        setTex(sun, nodepath.toStdString()+"sun.jpeg");//在球上贴纹理
        return sun;
    };
    //在日地半径上放置太阳直径的太阳个数
    int se = get_sun_radius_num(m_planetParams.at("Earth").track_km());
    std::cout<<" num "<<se<<"\n";
    for(int i = 0; i< se; i++)
    {
        double x = get_standard_radius(ST_RADIUS*2 * (i+1)/ST);
        osg::ref_ptr<osg::Geode> suntmp = appendSun(osg::Vec3(x, 0, 0));
        pSunPlanetNode->planetTransform->addChild(suntmp);
    }
#endif

        // 将行星的变换节点添加到太阳节点中

        auto appendPlanet=[&](const std::string& planetame)
        {
            tagPlanetNode* pSubPlanetNode = createPlanetGroup(planetame,
                                                            nodepath.toStdString()+planetame+".jpeg",
                                                              osg::Vec3(ORBIT_PHYSICS_RADIUS_TO_COORDINATE_RADIUS(m_planetParams.at(planetame).track_km()), 0, 0),
                                                            m_planetParams.at(planetame).track_km(),
                                                            m_planetParams.at(planetame).radius,
                                                            true,
                                                            true);
            pSunPlanetNode->planetTransform->addChild(pSubPlanetNode->planetGroup);
            /////////////////////////////////////////////////////////////////////////////////////////////////////////
            pSubPlanetNode->planetext = m_pModelWidget->pOSGManager()->getModelSenceData()->showText(planetame, osg::Vec3(ORBIT_PHYSICS_RADIUS_TO_COORDINATE_RADIUS(m_planetParams.at(planetame).track_km()), 0, 0), planetame, osg::Vec4(0.0,1.0,0.0,1.0),1.5);
            pSubPlanetNode->planetext->setName("text_switch_"+planetame);
            pSunPlanetNode->planetTransform->addChild(pSubPlanetNode->planetext);
            /////////////////////////////////////////////////////////////////////////////////////////////////////////
            pSunPlanetNode->subPlanets.push_back(pSubPlanetNode);
        };
        appendPlanet("Mercury");
        appendPlanet("Venus");
        appendPlanet("Earth");
        appendPlanet("Mars");
        appendPlanet("Jupiter");
        appendPlanet("Saturn");
        appendPlanet("Uranus");
        appendPlanet("Neptune");

        osg::ref_ptr<osg::MatrixTransform> RootSunTransform = new osg::MatrixTransform();

        RootSunTransform->addChild(pSunPlanetNode->planetGroup);

        //将太阳系节点添加到根节点
            m_pModelWidget->pOSGManager()->getModelSenceData()->HideGroundScene(false);
        m_pModelWidget->pOSGManager()->getModelSenceData()->addNode(RootSunTransform);
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::widget_callback(const tagCallBackParams &params)
{
    QString planetGeode = QString::fromStdString(params.groupName);
    QStringList planetGeodelist = planetGeode.split("_");
    if(planetGeodelist.size() == 2)
    {
        m_cur_focus_planet = planetGeodelist.at(0).toStdString();
        switch (params.e_pick_type)
        {
        case tagCallBackParams::E_PICK_TYPE_SELECT_TO_FOUCS:
        {
        }break;
        case tagCallBackParams::E_PICK_TYPE_PICK_PIONT:
        {
            deal_pick_to_append_point(params);
        }
        break;
        default:
            break;
        }
    }
}

std::optional<tagPlanetNode*> MainWindow::getPlanet(const std::string &planetname)
{
    auto itor = m_PlanetNodes.find(planetname);
    if(itor != m_PlanetNodes.end())
    {
        tagPlanetNode &_tagPlanetNode = itor->second;
        return &_tagPlanetNode;
    }
    return std::nullopt;
}
//#define DISABLE_RUNNINGORIBIT

void MainWindow::updatePlanetRunningOrbitTracking()
{
#ifdef DISABLE_RUNNINGORIBIT
    return;
#endif
    for(int i = 0; i < 3601; i++)
    {
        int angle = (i+180)%3601;
        m_UpdateBaseVerts.push_back(osg::Vec3f(sin(glm::radians((double)angle)), cos(glm::radians((double)angle)), 0));
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    m_pUpdateTimer = new QTimer(this);

    connect(m_pUpdateTimer,&QTimer::timeout,[&](){
        static int index = 0;
#if 0
        auto _PlanetNodes_itor = m_PlanetNodes.begin();
        while(_PlanetNodes_itor != m_PlanetNodes.end())
        {
            if(_PlanetNodes_itor->first != "Sun")
            {
                const tagPlanetNode &_tagPlanetNode = _PlanetNodes_itor->second;
                osg::Vec3f pt_origin = m_UpdateBaseVerts[index];
                double radius = _tagPlanetNode._planet_orbit_coordinate_radius;
                osg::Vec3f pt = osg::Vec3f(pt_origin.x()*radius, pt_origin.y()*radius, 0);
                updatePlanetTracking(_PlanetNodes_itor->first,pt);
            }
            _PlanetNodes_itor++;
        }
#else
        std::unordered_map<std::string, tagPlanetNode> m_PlanetNodes;
        std::string planetname = "Moon";
        auto planetNode = getPlanet(planetname);
        if(planetNode.has_value() && nullptr != planetNode.value())
        {
            const tagPlanetNode &_tagPlanetNode = *planetNode.value();
            osg::Vec3f pt_origin = m_UpdateBaseVerts[index];
            double radius = _tagPlanetNode._planet_orbit_coordinate_radius;
            osg::Vec3f pt = osg::Vec3f(pt_origin.x()*radius, pt_origin.y()*radius, 0);
            updatePlanetTracking(planetname,pt);
        }
#endif
        index++;
        index = index % 360;
    });

    m_pUpdateTimer->start(60);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void MainWindow::nodeAppendLines(osg::Node *pNode, std::vector<std::vector<std::tuple<osg::Vec3,osg::Vec4>> > &lines)
{

    if(pNode)
    {
        osg::Geode* pGeode = pNode->asGeode();
        if(pGeode)
        {
            osg::Drawable * pGeomDrawable = pGeode->getDrawable(0);
            if(pGeomDrawable)
            {
                osg::Geometry*  pGeom = pGeomDrawable->asGeometry();
                if(pGeom)
                {
                    osg::Vec3Array* pVertexes = nullptr;
                    osg::Vec4Array* pColors = nullptr;
                    //////////////////////////////////////////////////////////////////////////
                    osg::Array * pVertexesArray = pGeom->getVertexArray();
                    if(pVertexesArray)
                    {
                        pVertexes = dynamic_cast<osg::Vec3Array*>(pVertexesArray);
                    }
                    //////////////////////////////////////////////////////////////////////////
                    osg::Array * pColorsArray = pGeom->getColorArray();
                    if(pColorsArray)
                    {
                        pColors = dynamic_cast<osg::Vec4Array*>(pColorsArray);
                    }
                    //////////////////////////////////////////////////////////////////////////

                    if(pVertexes && pColors)
                    {
                        auto linesize = lines.size();
                        for (auto i = 0; i < linesize; ++i)
                        {
                            const std::vector<std::tuple<osg::Vec3,osg::Vec4>> &line = lines[i];

                            auto subsize = line.size();
#if 0
                            int pos = pVertexes->size();
                            for (auto j = 0; j < subsize; ++j)
                            {
                                const osg::Vec3& pt = std::get<0>(line.at(j));
                                const osg::Vec4& cl = std::get<1>(line.at(j));
                                pVertexes->push_back(pt);
                                pColors->push_back(cl);
                            }
                            osg::ref_ptr<osg::DrawArrays> pDrawArrays = new osg::DrawArrays(GL_LINE_STRIP, pos, subsize);
                            pGeom->addPrimitiveSet(pDrawArrays);
#else
                            osg::DrawElementsUInt* pDrawElementsUInt = new osg::DrawElementsUInt(GL_LINE_STRIP);
                            for (auto j = 0; j < subsize; ++j)
                            {
                                pDrawElementsUInt->push_back(pVertexes->size());
                                const osg::Vec3& pt = std::get<0>(line.at(j));
                                const osg::Vec4& cl = std::get<1>(line.at(j));
                                pVertexes->push_back(pt);
                                pColors->push_back(cl);
                            }
                            pGeom->addPrimitiveSet(pDrawElementsUInt);
#endif
                        }
                        pVertexes->dirty();
                        pColors->dirty();
                        pGeom->dirtyGLObjects();
                        pGeom->dirtyDisplayList();
                        pGeom->dirtyBound();
                        pGeode->dirtyBound();
                    }
                }
            }
        }
    }
}

void MainWindow::deal_pick_to_append_point(const tagCallBackParams &params)
{
//    std::cout<<" pick node "<<params.groupName<<std::endl;
    std::function<double (const double&)> Phi_to_degrees=[](const double& radians)
    {
        double _degrees = glm::degrees(radians);
        return _degrees;
    };

    std::function<double (const double&)> Theta_to_degrees=[](const double& radians)
    {
        double _degrees = glm::degrees(radians);
        return 90.0f - _degrees;
    };

    // 计算方位角（Azimuth）和俯仰角（Elevation）
    auto calculateAngles = [](double x, double y, double z, double& phi, double& theta) {
        // 计算方位角
        phi = atan2(y,x);

        // 计算俯仰角
        theta = acos(z / sqrt(x * x + y * y + z * z));
    };
    auto planetNode = getPlanet(m_cur_focus_planet);
    if(planetNode.has_value() && nullptr != planetNode.value())
    {
        const tagPlanetNode &_tagPlanetNode = *planetNode.value();

        osg::ref_ptr<osg::Switch> _planetAnchorGroup = _tagPlanetNode.planetAnchorGroup;
        if(_planetAnchorGroup.valid())
        {
            //            //获取相对于太阳系中心的方位和俯仰角
            //方位
            //            double azimuth = 0;
            //俯仰
            //            double elevation = 0;
            //            calculateAngles(std::get<0>(params.mouseWorldPosition),
            //                            std::get<1>(params.mouseWorldPosition),
            //                            std::get<2>(params.mouseWorldPosition),
            //                            azimuth,
            //                            elevation);

            //获取星球本体的

            //方位
            double radsPhi = 0;
            //俯仰
            double radsTheta = 0;
            calculateAngles(std::get<0>(params.mouseWorldPosition_sub),
                            std::get<1>(params.mouseWorldPosition_sub),
                            std::get<2>(params.mouseWorldPosition_sub),
                            radsPhi,
                            radsTheta);

            double samplePhi = Phi_to_degrees(radsPhi);
            double sampleTheta = Theta_to_degrees(radsTheta);

            std::cout.precision(17);
            std::cout<<_planetAnchorGroup->getName()
                      <<" select ---- ["<<std::get<0>(params.mouseWorldPosition_sub)<<" , "<<std::get<1>(params.mouseWorldPosition_sub)<<" , "<<std::get<2>(params.mouseWorldPosition_sub)<<"]"
                      <<" Phi ---- ["<<samplePhi<<"]"
                      <<" Theta ---- ["<<sampleTheta<<"]\n";
            {
                osg::Node*  pNode = _planetAnchorGroup->getChild(0);
                if(pNode)
                {
                    osg::Geode* pGeode = pNode->asGeode();
                    if(pGeode)
                    {
                        osg::Drawable * pGeomDrawable = pGeode->getDrawable(0);
                        if(pGeomDrawable)
                        {
                            osg::Geometry*  pGeom = pGeomDrawable->asGeometry();
                            if(pGeom)
                            {
                                osg::Vec3Array* pVertexes = nullptr;
                                osg::Vec4Array* pColors = nullptr;
                                osg::DrawArrays* pDrawArrays = nullptr;
                                //////////////////////////////////////////////////////////////////////////
                                osg::Array * pVertexesArray = pGeom->getVertexArray();
                                if(pVertexesArray)
                                {
                                    pVertexes = dynamic_cast<osg::Vec3Array*>(pVertexesArray);
                                }
                                //////////////////////////////////////////////////////////////////////////
                                osg::Array * pColorsArray = pGeom->getColorArray();
                                if(pColorsArray)
                                {
                                    pColors = dynamic_cast<osg::Vec4Array*>(pColorsArray);
                                }
                                //////////////////////////////////////////////////////////////////////////
                                osg::PrimitiveSet * pPrimitiveSet = pGeom->getPrimitiveSet(0);
                                if(pPrimitiveSet)
                                {
                                    pDrawArrays = dynamic_cast<osg::DrawArrays*>(pPrimitiveSet);
                                }
                                //////////////////////////////////////////////////////////////////////////

                                if(pVertexes && pColors && pDrawArrays)
                                {
                                    pVertexes->push_back(osg::Vec3(std::get<0>(params.mouseWorldPosition_sub), std::get<1>(params.mouseWorldPosition_sub), std::get<2>(params.mouseWorldPosition_sub)));
                                    pColors->push_back(GRID_MESH_PICK_COLOR);
                                    pVertexes->dirty();
                                    pColors->dirty();
                                    pDrawArrays->set(GL_POINTS, 0, pVertexes->size());
                                    pGeom->dirtyGLObjects();
                                    pGeom->dirtyDisplayList();
                                    pGeom->dirtyBound();
                                    pGeode->dirtyBound();
                                }
                            }
                        }
                    }
                }
            }
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            {

            dpctr::Depictor depictor;

            sge::SpaceGridEngineConfig config;
            config.starCode = 0;
            config.R0 = 1;
            config.pgN = 3;
            config.setLayerRadii({
                config.R0,
                config.R0 + 1,
            });

            sge::SpaceGridEngine unitEngine;
            unitEngine.initialize(config);


            // sample premesh cell
            std::vector<arma::vec> xarcs2;
            std::vector<arma::vec> yarcs2;
            std::vector<arma::vec> zarcs2;

//                auto samplePhi = -0.00001;
//                auto sampleTheta = 49.5;
            auto sampleRadius = 1.5;
            auto dumpFileFlag2 = false;
//                depictor.samplePremeshCellUVFace(unitEngine, samplePhi, sampleTheta, m_nPointEachEdge2, xarcs2, yarcs2, zarcs2, dumpFileFlag2);

//                // sample index cell
//                std::vector<arma::vec> xarcs3;
//                std::vector<arma::vec> yarcs3;
//                std::vector<arma::vec> zarcs3;

            uint8_t indexLevel = 4;
//                depictor.sampleIndexCellUVFace(unitEngine, samplePhi, sampleTheta, indexLevel, m_nPointEachEdge2, xarcs3, yarcs3, zarcs3, dumpFileFlag2);

            // sample premesh cell block
            std::vector<arma::vec> xarcs4;
            std::vector<arma::vec> yarcs4;
            std::vector<arma::vec> zarcs4;

            auto drawLines=[&](osg::Node*  pNode,
                                const std::vector<arma::vec>& xarcs,
                                const std::vector<arma::vec>& yarcs,
                                const std::vector<arma::vec>& zarcs)
            {
#if 0
                auto size = xarcs.size();

                for (auto i = 0; i < size; ++i)
                {
                    const auto &xarc = xarcs[i];
                    const auto &yarc = yarcs[i];
                    const auto &zarc = zarcs[i];

                    auto subsize = xarc.size();
                    std::vector<DrawFunc::tagpos> _pt_datas;
                    _pt_datas.reserve(subsize);
                    for (auto j = 0; j < subsize; ++j)
                    {
                        auto val = DrawFunc::tagpos{xarc[j], yarc[j], zarc[j]};
                        _pt_datas.push_back(val);
                    }

                    std::vector<osg::Vec4> linecls;
                    linecls.push_back(osg::Vec4(0.0,1.0,1.0,1.0));
                    osg::Geode*  pLineGeode = DrawFunc::createSphereGeometryEx(_pt_datas, _tagPlanetNode._planet_coordinate_radius, GL_LINE_STRIP, std::vector<UINT32>(), osg::Vec3(0,0,0), linecls);
                    if(pLineGeode)
                    {
                        pLineGeode->setName("geo_line_"+params.groupName);
                        _tagPlanetNode.planetAnchorGroup->addChild(pLineGeode);
                    }
                }
#else

                std::vector<std::vector<std::tuple<osg::Vec3,osg::Vec4>> > lines;

                auto size = xarcs.size();
                lines.resize(size);
                const double dRadius = _tagPlanetNode._planet_coordinate_radius;
                const osg::Vec3 center = osg::Vec3(0,0,0);
                const double scaleoffset = 1.0;

                for (auto i = 0; i < size; ++i)
                {
                    const auto &xarc = xarcs[i];
                    const auto &yarc = yarcs[i];
                    const auto &zarc = zarcs[i];

                    auto subsize = xarc.size();
                    std::vector<std::tuple<osg::Vec3,osg::Vec4>> line;
                    line.resize(subsize);
                    for (auto j = 0; j < subsize; ++j)
                    {
                        osg::Vec3 pt((center.x() + xarc[j]) * dRadius * scaleoffset, (center.y() + yarc[j]) * dRadius*scaleoffset, (center.z() + zarc[j]) * dRadius*scaleoffset);
                        line[j] = std::make_tuple(pt, GRID_MESH_PICK_COLOR);
                    }
                    lines[i] = std::move(line);
                }

                nodeAppendLines(pNode, lines);
#endif
            };

            osg::Node*  pNode = _planetAnchorGroup->getChild(1);
//            {
//                // draw premesh cell block
//                depictor.samplePremeshCellBlock(unitEngine, samplePhi, sampleTheta, sampleRadius, m_nPointEachEdge2, xarcs4, yarcs4, zarcs4, dumpFileFlag2);
//                drawLines(pNode,xarcs4, yarcs4, zarcs4);
//            }

//            {
//                // sample index cell block
//                depictor.sampleIndexCellBlock(unitEngine, samplePhi, sampleTheta, sampleRadius, indexLevel, m_nPointEachEdge2, xarcs4, yarcs4, zarcs4, dumpFileFlag2);
//                drawLines(pNode,xarcs4, yarcs4, zarcs4);
//            }

//                sge::PreMeshCellInfo premeshCellInfo;
//                // sample premesh cell info
//                depictor.samplePremshCellInfo(unitEngine, samplePhi, sampleTheta, sampleRadius, premeshCellInfo);

//                sge::IndexCellInfo indexCellInfo;
//                depictor.sampleIndexCellInfo(unitEngine, samplePhi, sampleTheta, sampleRadius, indexLevel, indexCellInfo);

            }
        }
    }
}

void MainWindow::getSpherePremeshData(std::vector<std::vector<tagpos> > &_datas, UINT32 nSeg, UINT32 nPointEachEdge)
{



    std::vector<arma::vec> xarcs;
    std::vector<arma::vec> yarcs;
    std::vector<arma::vec> zarcs;

    // xarcs, yarcs, zarcs are the result x, y, z coordinates of the arcs
    auto dumpFileFlag = false;
    {
        auto curveRotateCopy=[](const std::vector<arma::vec> &xcurves0,
                                  const std::vector<arma::vec> &ycurves0,
                                  const std::vector<arma::vec> &zcurves0,
                                  const double rotateAngle,
                                  const int nCopies,
                                  std::vector<arma::vec> &xcurves,
                                  std::vector<arma::vec> &ycurves,
                                  std::vector<arma::vec> &zcurves)
        {
            auto originSize = xcurves0.size();

            xcurves.resize(originSize * nCopies);
            ycurves.resize(originSize * nCopies);
            zcurves.resize(originSize * nCopies);

            for (auto i = 0; i < originSize; ++i)
            {
                arma::vec xcurve0 = xcurves0[i];
                arma::vec ycurve0 = ycurves0[i];
                arma::vec zcurve0 = zcurves0[i];

                arma::vec xcurve;
                arma::vec ycurve;
                arma::vec zcurve;

                auto nSamples = xcurve0.size();
                auto rotxy = rotateAngle / 180.0 * arma::datum::pi;
                arma::cx_vec xyro(nSamples);
                xyro.fill(arma::cx_double(0.0, -rotxy));

                for (auto j = 0; j < nCopies; ++j)
                {
                    arma::cx_vec xy = xcurve0 + arma::cx_double(0.0, 1.0) * ycurve0;
                    xy = xy % arma::exp(xyro);
                    xcurve = arma::real(xy);
                    ycurve = arma::imag(xy);
                    zcurve = zcurve0;

                    xcurves[i * nCopies + j] = xcurve;
                    ycurves[i * nCopies + j] = ycurve;
                    zcurves[i * nCopies + j] = zcurve;

                    rotxy += rotateAngle / 180.0 * arma::datum::pi;
                    xyro.fill(arma::cx_double(0.0, -rotxy));
                }
            }
        };

        dpctr::Depictor depictor;
        depictor.spherePremesh(nSeg, nPointEachEdge, xarcs, yarcs, zarcs, dumpFileFlag);



        std::vector<arma::vec> xarcsCopy;
        std::vector<arma::vec> yarcsCopy;
        std::vector<arma::vec> zarcsCopy;

        // rotate the arcs by 90 degrees and copy 4 times
        curveRotateCopy(xarcs, yarcs, zarcs, 90, 4, xarcsCopy, yarcsCopy, zarcsCopy);

        {
            auto size = xarcsCopy.size();
            _datas.resize(size);
            for (auto i = 0; i < size; ++i)
            {
                const auto &xarc = xarcsCopy[i];
                const auto &yarc = yarcsCopy[i];
                const auto &zarc = zarcsCopy[i];


                auto subsize = xarc.size();
                std::vector<DrawFunc::tagpos> positions;
                positions.resize(subsize);
                for (auto j = 0; j < xarc.size(); ++j)
                {
                    positions[j]=DrawFunc::tagpos{xarc[j], yarc[j], zarc[j]};
                }
                _datas[i] = std::move(positions);
            }
        }
    }
    std::cout<<" arx "<<_datas.size()<<std::endl;
}


