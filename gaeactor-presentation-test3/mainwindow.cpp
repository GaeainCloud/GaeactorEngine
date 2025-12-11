#include "mainwindow.h"
#include "ex/MeshHolder.h"
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

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/PolygonMode>
#include <osg/OcclusionQueryNode>
#include <osg/BlendFunc>
#include <osg/Uniform>
#include <osg/FrontFace>

#define SCALE (1000.0)

// Create a cube with one side missing. This makes a great simple occluder.
osg::ref_ptr<osg::Node>
createBox()
{
    osg::ref_ptr<osg::Geode> box = new osg::Geode;

    osg::StateSet* state = box->getOrCreateStateSet();

    osg::PolygonMode* pm = new osg::PolygonMode( osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL );
    state->setAttributeAndModes( pm, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
    geom->setVertexArray( v.get() );

    osg::ref_ptr<osg::Vec3Array> n = new osg::Vec3Array;
    geom->setNormalArray( n.get(), osg::Array::BIND_PER_VERTEX );

    {
        const float x( 0.f *SCALE);
        const float y( 0.f *SCALE );
        const float z( 0.f *SCALE);
        const float r( 1.1f *SCALE );

        v->push_back( osg::Vec3( x-r, y-r, z-r ) ); //left -X
        v->push_back( osg::Vec3( x-r, y-r, z+r ) );
        v->push_back( osg::Vec3( x-r, y+r, z+r ) );
        v->push_back( osg::Vec3( x-r, y+r, z-r ) );

        n->push_back( osg::Vec3( -1.f, 0.f, 0.f ) );
        n->push_back( osg::Vec3( -1.f, 0.f, 0.f ) );
        n->push_back( osg::Vec3( -1.f, 0.f, 0.f ) );
        n->push_back( osg::Vec3( -1.f, 0.f, 0.f ) );

        v->push_back( osg::Vec3( x+r, y-r, z+r ) ); //right +X
        v->push_back( osg::Vec3( x+r, y-r, z-r ) );
        v->push_back( osg::Vec3( x+r, y+r, z-r ) );
        v->push_back( osg::Vec3( x+r, y+r, z+r ) );

        n->push_back( osg::Vec3( 1.f, 0.f, 0.f ) );
        n->push_back( osg::Vec3( 1.f, 0.f, 0.f ) );
        n->push_back( osg::Vec3( 1.f, 0.f, 0.f ) );
        n->push_back( osg::Vec3( 1.f, 0.f, 0.f ) );

        v->push_back( osg::Vec3( x-r, y-r, z-r ) ); // bottom -Z
        v->push_back( osg::Vec3( x-r, y+r, z-r ) );
        v->push_back( osg::Vec3( x+r, y+r, z-r ) );
        v->push_back( osg::Vec3( x+r, y-r, z-r ) );

        n->push_back( osg::Vec3( 0.f, 0.f, -1.f ) );
        n->push_back( osg::Vec3( 0.f, 0.f, -1.f ) );
        n->push_back( osg::Vec3( 0.f, 0.f, -1.f ) );
        n->push_back( osg::Vec3( 0.f, 0.f, -1.f ) );

        v->push_back( osg::Vec3( x-r, y-r, z+r ) ); // top +Z
        v->push_back( osg::Vec3( x+r, y-r, z+r ) );
        v->push_back( osg::Vec3( x+r, y+r, z+r ) );
        v->push_back( osg::Vec3( x-r, y+r, z+r ) );

        n->push_back( osg::Vec3( 0.f, 0.f, 1.f ) );
        n->push_back( osg::Vec3( 0.f, 0.f, 1.f ) );
        n->push_back( osg::Vec3( 0.f, 0.f, 1.f ) );
        n->push_back( osg::Vec3( 0.f, 0.f, 1.f ) );

        v->push_back( osg::Vec3( x-r, y+r, z-r ) ); // back +Y
        v->push_back( osg::Vec3( x-r, y+r, z+r ) );
        v->push_back( osg::Vec3( x+r, y+r, z+r ) );
        v->push_back( osg::Vec3( x+r, y+r, z-r ) );

        n->push_back( osg::Vec3( 0.f, 1.f, 0.f ) );
        n->push_back( osg::Vec3( 0.f, 1.f, 0.f ) );
        n->push_back( osg::Vec3( 0.f, 1.f, 0.f ) );
        n->push_back( osg::Vec3( 0.f, 1.f, 0.f ) );
    }

    osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array;
    geom->setColorArray( c.get(), osg::Array::BIND_OVERALL );

    c->push_back( osg::Vec4( 0.f, 1.f, 1.f, 1.f ) );

    geom->addPrimitiveSet( new osg::DrawArrays( GL_QUADS, 0, 20 ) );
    box->addDrawable( geom.get() );

    return box.get();
}

// Make a Geometry that renders slow intentionally.
// To make sure it renders slow, we do the following:
//  * Disable display lists
//  * Force glBegin/glEnd slow path
//  * Lots of vertices and color data per vertex
//  * No vertex sharing
//  * Draw the triangles as wireframe
osg::ref_ptr<osg::Node>
createRandomTriangles( unsigned int numTriangles )
{
    unsigned int numVertices = numTriangles*3;

    osg::ref_ptr<osg::Geode> tris = new osg::Geode;

    osg::StateSet* ss = tris->getOrCreateStateSet();

    // Force wireframe. Many gfx cards handle this poorly.
    osg::PolygonMode* pm = new osg::PolygonMode( osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );
    ss->setAttributeAndModes( pm, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

    // Disable display lists to decrease performance.
//    geom->setUseDisplayList( false );
    geom->setUseDisplayList( true );

    osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
    geom->setVertexArray( v.get() );
    v->resize( numVertices );

    osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array;
    geom->setColorArray( c.get(),  osg::Array::BIND_PER_VERTEX);
    c->resize( numVertices );

    unsigned int i;
    srand( 0 );
#define RAND_NEG1_TO_1 ( ((rand()%20)-10)*.1 )
#define RAND_0_TO_1 ( (rand()%10)*.1 )
    for (i=0; i<numTriangles; i++)
    {
        osg::Vec3& v0 = (*v)[ i*3+0 ];
        osg::Vec3& v1 = (*v)[ i*3+1 ];
        osg::Vec3& v2 = (*v)[ i*3+2 ];
        v0 = osg::Vec3( RAND_NEG1_TO_1*SCALE, RAND_NEG1_TO_1*SCALE, RAND_NEG1_TO_1*SCALE );
        v1 = osg::Vec3( RAND_NEG1_TO_1*SCALE, RAND_NEG1_TO_1*SCALE, RAND_NEG1_TO_1*SCALE );
        v2 = osg::Vec3( RAND_NEG1_TO_1*SCALE, RAND_NEG1_TO_1*SCALE, RAND_NEG1_TO_1*SCALE );

        osg::Vec4& c0 = (*c)[ i*3+0 ];
        osg::Vec4& c1 = (*c)[ i*3+1 ];
        osg::Vec4& c2 = (*c)[ i*3+2 ];
        c0 = osg::Vec4( RAND_0_TO_1, RAND_0_TO_1, RAND_0_TO_1, 1. );
        c1 = c0;
        c2 = c0;
    }


    geom->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 0, numVertices ) );
    tris->addDrawable( geom.get() );

    return tris.get();
}

// Create the stock scene:
// Top level Group
//   Geode (simple occluder
//   OcclusionQueryNode
//     Geode with complex, slow geometry.
osg::ref_ptr<osg::Node>
createStockScene()
{
    // Create a simple box occluder
    osg::ref_ptr<osg::Group> root = new osg::Group();
//    root->addChild( createBox().get() );

    // Create a complex mess of triangles as a child below an
    //   OcclusionQueryNode. The OQN will ensure that the
    //   subgraph isn't rendered when it's not visible.
    osg::ref_ptr<osg::OcclusionQueryNode> oqn = new osg::OcclusionQueryNode;
    oqn->addChild( createBox());

    osg::ref_ptr<osg::OcclusionQueryNode> oqn2 = new osg::OcclusionQueryNode;
    oqn2->addChild( createRandomTriangles( 20000*4 ) );
    root->addChild( oqn.get() );
    root->addChild( oqn2.get() );

    return root.get();
}

//#include <osg/Node>
//#include <osg/NodeVisitor>
//#include <osg/Uniform>
//#include <osg/Matrix>
//#include <osgViewer/Viewer>
//#include <osg/NodeCallback>

//class UpdateUniformsVisitor : public osg::NodeVisitor
//{
//public:
//    UpdateUniformsVisitor(osgViewer::Viewer* viewer) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _viewer(viewer) {}

//    virtual void apply(osg::Node& node)
//    {
//        // 获取模型矩阵
//        osg::Matrixd modelMatrix = node.getWorldMatrices()[0];
//        // 获取视图矩阵
//        osg::Matrixd viewMatrix = _viewer->getCamera()->getViewMatrix();
//        // 获取投影矩阵
//        osg::Matrixd projectionMatrix = _viewer->getCamera()->getProjectionMatrix();
//        // 计算模型视图投影矩阵
//        osg::Matrixd modelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;

//        // 获取状态集
//        osg::StateSet* stateSet = node.getStateSet();
//        if (stateSet)
//        {
//            // 获取统一变量
//            osg::Uniform* uniform = stateSet->getUniform("osg_ModelViewProjectionMatrix");
//            if (uniform)
//            {
//                // 更新统一变量
//                uniform->set(modelViewProjectionMatrix);
//            }
//        }

//        // 继续遍历子节点
//        traverse(node);
//    }

//private:
//    osgViewer::Viewer* _viewer;
//};



float _earthSizeCoefficient = 2.0;
float earth_radius_base = 2.0;
float earth_radius = earth_radius_base*_earthSizeCoefficient;
osg::Vec3d earth_position(30,0,0);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->resize(1400,1200);


    m_pModelWidget = new QtOSGWidget(QtOSGWidget::E_OSG_SHOW_TYPE_MODEL, this);
    this->setCentralWidget(m_pModelWidget);

//    // 创建几何体对象
//    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();

//    // 创建顶点数组，包含一个点
//    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
//    vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
//    geom->setVertexArray(vertices);
//    geom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, 1));

//    // 创建着色器程序
//    osg::ref_ptr<osg::Program> program = new osg::Program;
//    //    osg::ref_ptr<osg::Shader> vertShader = new osg::Shader(osg::Shader::VERTEX);
//    //    osg::ref_ptr<osg::Shader> geomShader = new osg::Shader(osg::Shader::GEOMETRY);
//    //    osg::ref_ptr<osg::Shader> fragShader = new osg::Shader(osg::Shader::FRAGMENT);

//    //    // 加载着色器代码
//    //    vertShader->setShaderSource(
//    //        "#version 330 core\n"
//    //        "layout(location = 0) in vec4 osg_Vertex;\n"
//    //        "uniform mat4 osg_ModelViewProjectionMatrix;\n"
//    //        "void main() {\n"
//    //        "    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n"
//    //        "}"
//    //        );

//    //    geomShader->setShaderSource(
//    //        "#version 330 core\n"
//    //        "layout(points) in;\n"
//    //        "layout(triangles, max_vertices = 36) out;\n"
//    //        "uniform mat4 osg_ModelViewProjectionMatrix;\n"
//    //        "uniform float size = 1.0;\n"
//    //        "void main() {\n"
//    //        "    vec3[8] vertices = vec3[](\n"
//    //        "        vec3(-1.0, -1.0, -1.0), ...\n"
//    //        "    );\n"
//    //        "    int[6] faces = int[](\n"
//    //        "        0, 1, 2, 2, 3, 0, ...\n"
//    //        "    );\n"
//    //        "    for (int i = 0; i < 6; ++i) {\n"
//    //        "        for (int j = 0; j < 6; j += 2) {\n"
//    //        "            gl_Position = osg_ModelViewProjectionMatrix * vec4(vertices[faces[i * 6 + j]], 1.0);\n"
//    //        "            EmitVertex();\n"
//    //        "            gl_Position = osg_ModelViewProjectionMatrix * vec4(vertices[faces[i * 6 + j + 1]], 1.0);\n"
//    //        "            EmitVertex();\n"
//    //        "            gl_Position = osg_ModelViewProjectionMatrix * vec4(vertices[faces[i * 6 + j + 2]], 1.0);\n"
//    //        "            EmitVertex();\n"
//    //        "            EndPrimitive();\n"
//    //        "        }\n"
//    //        "    }\n"
//    //        "}"
//    //        );

//    //    fragShader->setShaderSource(
//    //        "#version 330 core\n"
//    //        "out vec4 FragColor;\n"
//    //        "void main() {\n"
//    //        "    FragColor = vec4(1.0, 0.5, 0.2, 1.0);\n"
//    //        "}"
//    //        );

//    auto _vertexShader = osgDB::readRefShaderFile(osg::Shader::VERTEX, "D:/project/WorkProject_20240402001152/gaeactor/gaeactor-display/shaders/pointexcube.vs");
//    program->addShader(_vertexShader.get());

//    auto _geometryShader = osgDB::readRefShaderFile(osg::Shader::GEOMETRY, "D:/project/WorkProject_20240402001152/gaeactor/gaeactor-display/shaders/pointexcube.gs");
//    program->addShader(_geometryShader.get());

//    auto _fragmentShader = osgDB::readRefShaderFile(osg::Shader::FRAGMENT, "D:/project/WorkProject_20240402001152/gaeactor/gaeactor-display/shaders/pointexcube.fs");
//    program->addShader(_fragmentShader.get());
//    // 将着色器添加到着色器程序
//    //    program->addShader(vertShader);
//    //    program->addShader(geomShader);
//    //    program->addShader(fragShader);

//    // 将着色器程序应用到几何体
//    geom->getOrCreateStateSet()->setAttributeAndModes(program);

//    // 创建场景并添加几何体
//    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
//    geode->addDrawable(geom);

//    m_pModelWidget->pOSGManager()->getModelSenceData()->addNode(geode);



    // load the specified model
//    osg::ref_ptr<osg::Node> root = createStockScene().get();


//    osgUtil::Optimizer optimizer;
//    optimizer.optimize( root );

//    m_pModelWidget->pOSGManager()->getModelSenceData()->addNode(root);


//    osg::ref_ptr<osg::Geode> planetGeode = new osg::Geode;

//    // 创建星球球体
//    osg::ref_ptr<osg::Geometry>  planetGeometry = DrawFunc::createSphereGeometry(osg::Vec3(0, 0, 0),180, 20, 40);


//    tagGeodeParams params;
//    params.lineWidth = 5;

//    QString path = QCoreApplication::applicationDirPath();
//    params.vspath = path.toStdString() + "/shaders/normaldisplay.vs";
//    params.gspath = path.toStdString() + "/shaders/normaldisplay.gs";
//    params.fspath = path.toStdString() + "/shaders/normaldisplay.fs";
//    DrawFunc::createGeometryShaderProgram(planetGeometry, params);


//    planetGeode->addChild(planetGeometry);
//    osgUtil::Optimizer optimizer;
//    optimizer.optimize( planetGeode );


//    //三句话
//    OSGHandler::getInstance().m_pCamera->getGraphicsContext()->getState()->resetVertexAttributeAlias(false);//1
//    OSGHandler::getInstance().m_pCamera->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);//2
//    OSGHandler::getInstance().m_pCamera->getGraphicsContext()->getState()->setUseVertexAttributeAliasing(true);//3
//    // 创建自定义NodeVisitor
//    UpdateUniformsVisitor updateUniformsVisitor(OSGHandler::getInstance().m_pViewer);

//    // 在每一帧中更新统一变量

//    planetGeode->setUpdateCallback(new osg::NodeCallback([&updateUniformsVisitor](osg::Node* node, osg::NodeVisitor* nv) {
//        updateUniformsVisitor.apply(*node);
//        return true;
//    }));



//    osg::ref_ptr<osg::Geometry> DrawFunc::createSphereGeometry(const osg::Vec3d& center, const double &rotateAngle, double dRadius /*= 1.0*/, int iHint /*= 18*/)
//    {
//        tagGeodeParams _tagGeodeParams;

//        osg::Matrixd Geometry_matrix_init;
//        Geometry_matrix_init.makeRotate(glm::radians(rotateAngle),osg::Z_AXIS);

//        for (int i = 0; i <= iHint; i++)//从上到下添加点，有等于号是为了之后贴纹理时可以和图片的点一一对应
//        {
//            for (int j = 0; j <= iHint * 2; j++)//逆时针添加点
//            {
//                //球面坐标公式
//                osg::Vec3 vec3VertexT(center.x() + sin(osg::PI*i / iHint)*cos(osg::PI*j / iHint),
//                                      center.y() + sin(osg::PI*i / iHint)*sin(osg::PI*j / iHint),
//                                      center.z() + cos(osg::PI*i / iHint));
//                //添加顶点
//                _tagGeodeParams.vertexes.push_back(vec3VertexT*Geometry_matrix_init * dRadius);
//                //添加法线
//                _tagGeodeParams.normals.push_back(vec3VertexT);
//                //添加纹理坐标
//                _tagGeodeParams.texcoord.push_back(osg::Vec2(double(j) / 2.0 / iHint, 1 - double(i) / iHint));
//            }
//        }

//        _tagGeodeParams.colors.push_back(osg::Vec4(1.0, 1.0, 1.0, 0.5));

//        tagGeodeParams::tagDrawIndicesParams _tagDrawIndicesParams;
//        _tagDrawIndicesParams.mode = osg::PrimitiveSet::TRIANGLE_STRIP;
//        //添加图元
//        osg::ref_ptr<osg::DrawElementsUInt> rpFace = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_STRIP);
//        for (int i = 0; i < iHint; i++)
//        {
//            for (int j = 0; j <= iHint * 2; j++)
//            {
//                _tagDrawIndicesParams.indices.push_back(i*(iHint * 2 + 1) + j);
//                _tagDrawIndicesParams.indices.push_back((i + 1)*(iHint * 2 + 1) + j);
//            }
//        }
//        _tagGeodeParams.drawindices.push_back(std::move(_tagDrawIndicesParams));
//        _tagGeodeParams.mode = osg::PrimitiveSet::TRIANGLE_STRIP;

//        osg::ref_ptr<osg::Geometry> pGeom = createGeometry(_tagGeodeParams);

//        return pGeom;
//    }

//    QString path = QCoreApplication::applicationDirPath();
//    std::string vspath = path.toStdString() + "/res/models/sphere.obj";
//    osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(vspath);
//    if (!node)
//    {
//        return;
//    }

//    struct Vertex {
//        glm::vec3 position;
//        glm::vec3 normal;
//        glm::vec2 textureCoords;
//        glm::vec3 tangent;
//        glm::vec3 bitangent;
//    };

//    struct Texture {
//        size_t id;
//        std::string type;
//        aiString path;
//    };

//    struct Mesh{
//        std::vector<Vertex> _vertices; // Вершины
//        std::vector<size_t> _indices; // Индексы
//        std::vector<Texture> _textures; // Текстуры
//    };

#if 1

    std::function<void (osg::ref_ptr<osg::Node> , const std::string &) > setTex=[](osg::ref_ptr<osg::Node> pNode, const std::string &path)
    {
        //    return;
        osg::ref_ptr<osg::Image> rpImage = osgDB::readImageFile(path);
        if (!rpImage) {
            std::cerr << "can not load DDS file" << std::endl;
                return -1;
        }

        osg::ref_ptr<osg::Texture2D> rpTexture = new osg::Texture2D();

        rpTexture->setImage(rpImage);
//        rpTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);//设置S方向的环绕模式
//        rpTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);//设置R方向的环绕模式

        rpTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);//设置S方向的环绕模式
        rpTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);//设置R方向的环绕模式
        rpTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);//设置R方向的环绕模式
        rpTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);//设置R方向的环绕模式

        osg::ref_ptr<osg::StateSet> pState = pNode->getOrCreateStateSet();
        pState->setTextureAttributeAndModes(0, rpTexture, osg::StateAttribute::ON);

        //关闭光照确保纹理可见
        pState->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

#if 0
    pState->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
    osg::ref_ptr<osg::Material> material = new osg::Material;
    material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 0.1f)); // 设置透明度为0.5
    material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 0.1f));
    material->setTransparency(osg::Material::FRONT_AND_BACK,25.5f);
    pState->setAttribute(material.get(), osg::StateAttribute::OVERRIDE);

    pState->setMode(GL_BLEND, osg::StateAttribute::ON);
    pState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    // 设置混合函数
    pState->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);
#endif
    };


    auto DcreateGeometryShaderProgram=[](osg::ref_ptr<osg::Geometry> pGeom,
                                           const std::string& vspath,
                                           const std::string& gspath,
                                           const std::string& fspath)
    {
        if(pGeom.valid() &&
            (!vspath.empty()||
             !gspath.empty()||
             !fspath.empty()))
        {
            // 设置几何着色器
            osg::ref_ptr<osg::Program> program = new osg::Program;

            if(!vspath.empty())
            {
                auto _vertexShader = osgDB::readRefShaderFile(osg::Shader::VERTEX, vspath);
                program->addShader(_vertexShader.get());
                //            std::cout<<"add vs shader "<<params.vspath<<std::endl;
            }
            if(!gspath.empty())
            {
                auto _geometryShader = osgDB::readRefShaderFile(osg::Shader::GEOMETRY, gspath);
                program->addShader(_geometryShader.get());
                //            std::cout<<"add gs shader "<<params.gspath<<std::endl;
            }
            if(!fspath.empty())
            {
                auto _fragmentShader = osgDB::readRefShaderFile(osg::Shader::FRAGMENT, fspath);
                program->addShader(_fragmentShader.get());
                //            std::cout<<"add fs shader "<<params.fspath<<std::endl;
            }
            // 创建Uniform并设置值
            pGeom->getOrCreateStateSet()->setAttributeAndModes(program);
        }
    };

#if 1

    enum E_GEODE_TYPE
    {
        E_GEODE_TYPE_PLANET,
        E_GEODE_TYPE_CLOUDS,
        E_GEODE_TYPE_ATMOSPHERE,

    };


    struct AtmosphereInfo {
        MeshHolder atmosphereModel;
        glm::vec3 atmosphereColor, mieTint;
        float scaleFactor, innerRadius, outerRadius;
        float _atmosphereOuterBoundary = 2.0;

        explicit AtmosphereInfo(MeshHolder model, float earthScaleFactor, const glm::vec3& atmosphereColor, float innerRadius, float outerRadius, const glm::vec3& mieTint = glm::vec3(1.0))
            : atmosphereModel(std::move(model)),
            atmosphereColor(atmosphereColor),
            mieTint(mieTint),
            scaleFactor(earthScaleFactor),
            innerRadius(innerRadius),
            outerRadius(outerRadius)
        {
            _atmosphereOuterBoundary *= scaleFactor;
        }
    };


    class UpdateAtmosSphereCallback : public osg::NodeCallback
    {
    public:
        UpdateAtmosSphereCallback(QtOSGWidget* _pModelWidget = nullptr){m_pModelWidget = _pModelWidget;}
        void operator()(osg::Node* node, osg::NodeVisitor* nv) override
        {
            osg::Vec3d sun_position(0,0,0);
            float camera_near = 0.001f;
            float camera_far = 20000.f;

            if(m_pModelWidget)
            {
                osg::Vec3d _eye;
                osg::Vec3d _center;
                osg::Vec3d _up;

                m_pModelWidget->pOSGManager()->getOsgHandler()->m_pCamera->getViewMatrixAsLookAt(_eye, _center, _up);

                osg::Matrix matrixTmp = osg::Matrix::identity();
                matrixTmp *= osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::Z_AXIS);


                osg::Matrix _cameraProjectionMatrix = m_pModelWidget->pOSGManager()->getOsgHandler()->m_pCamera->getProjectionMatrix();
                osg::Matrix _cameraViewMatrix = m_pModelWidget->pOSGManager()->getOsgHandler()->m_pCamera->getViewMatrix();

                DrawFunc::setMat4f(node,"projection", _cameraProjectionMatrix);
                DrawFunc::setMat4f(node,"view", _cameraViewMatrix);

                double fovy; double aspectRatio;
                double zNear; double zFar;

                m_pModelWidget->pOSGManager()->getOsgHandler()->m_pCamera->getProjectionMatrixAsPerspective(fovy,aspectRatio,zNear,zFar);
                static const double zCoef = /*earth_radius_base*/2.0 / glm::log2(zFar + 1.0);
                DrawFunc::setFloat(node,"farPlane", zFar);
                DrawFunc::setFloat(node,"zCoef", zCoef);
                DrawFunc::setFloat(node,"bias", 0.001);
                DrawFunc::setInt(node,"shadowMap", 11);

                osg::Vec3d _earth_position = earth_position * matrixTmp;
                osg::Vec3d _sun_position = sun_position * matrixTmp;

//                const osg::Matrix lightProjection = osg::Matrixd::ortho(-earth_radius * 3.0f, earth_radius * 3.0f, -earth_radius * 3.0f, earth_radius * 3.0f, camera_near, camera_far);
                const osg::Matrix lightProjection = osg::Matrixd::ortho(-earth_radius * 3.0f, earth_radius * 3.0f, -earth_radius * 3.0f, earth_radius * 3.0f, zNear, zFar);
                const osg::Matrix lightView = osg::Matrixd::lookAt(_sun_position, _earth_position - _sun_position, osg::Vec3d(0.0, 1.0, 0.0));
                const osg::Matrix _lightSpaceMatrix = lightProjection * lightView;

                DrawFunc::setMat4f(node,"lightSpaceMatrix", _lightSpaceMatrix);

                DrawFunc::setVec3f(node, "camPosition", (_eye - _earth_position));
                DrawFunc::setVec3f(node,"lightPos", (_sun_position - _earth_position));
                DrawFunc::setVec3f(node,"mieTint", osg::Vec3d(1.0,1.0,1.0));
                DrawFunc::setFloat(node,"SCALE_H_FACTOR", 6.0);
                DrawFunc::setFloat(node,"earthSizeCoefficient", 1.0*_earthSizeCoefficient);
                DrawFunc::setBool(node,"isUseToneMapping", false);
                DrawFunc::setBool(node,"isNearbyPlanetaryRing", false);

                if(glm::length(glm::vec3(_earth_position.x(), _earth_position.y(), _earth_position.z()) - glm::vec3(_eye.x(), _eye.y(), _eye.z())) <= earth_radius_base*1.1)
                {
                    node->getOrCreateStateSet()->setMode(GL_FRONT_FACE, osg::StateAttribute::ON);
                    node->getOrCreateStateSet()->setAttributeAndModes(new osg::FrontFace(osg::FrontFace::CLOCKWISE), osg::StateAttribute::ON);
                }

                DrawFunc::setVec3f(node,"C_R",  osg::Vec3d(0.3, 0.7, 1.0));
                DrawFunc::setFloat(node,"innerRadius", (earth_radius_base - 0.00007)*_earthSizeCoefficient);
                DrawFunc::setFloat(node,"outerRadius", (earth_radius_base + 0.1)*_earthSizeCoefficient);

                auto Transform = node->getParent(0)->getParent(0)->asTransform();
                if(Transform)
                {
                    auto matrixTransform = dynamic_cast<osg::MatrixTransform*>(Transform);
                    if(Transform)
                    {
                        DrawFunc::setMat4f(node, "model", matrixTransform->getMatrix() * matrixTmp);
                    }
                }

                node->getOrCreateStateSet()->setMode(GL_FRONT_FACE, osg::StateAttribute::ON);
                node->getOrCreateStateSet()->setAttributeAndModes(new osg::FrontFace(osg::FrontFace::COUNTER_CLOCKWISE), osg::StateAttribute::ON);

            }
            traverse(node, nv);
        }
    private:

        QtOSGWidget* m_pModelWidget;
    };

    auto createGeode=[&](E_GEODE_TYPE eType,const std::string & meshpath,
                                 const std::string & texturepath,
                                 const osg::Vec3d & position,
                                 float _scaleFactor = 1.0,
                                 const std::string& vspath = "",
                                 const std::string& gspath = "",
                                 const std::string& fspath = "")->osg::ref_ptr<osg::Group>
    {
        osg::ref_ptr<osg::Group> pGroup = new osg::Group;
        MeshHolder sphereModel(meshpath);

        auto meshs = sphereModel.getMeshes();
        auto meshs_itor = meshs.begin();
        while(meshs_itor != meshs.end())
        {
            auto &_Mesh = *meshs_itor;

            const std::vector<Vertex> &_vertices = _Mesh._vertices;
            const std::vector<size_t> &_indices = _Mesh._indices;

            int vsize = _vertices.size();
            tagGeodeParams _tagGeodeParams;
            _tagGeodeParams.vertexes.resize(vsize);
            _tagGeodeParams.normals.resize(vsize);
            _tagGeodeParams.texcoord.resize(vsize);
            _tagGeodeParams.colors.resize(vsize);

            for(int m = 0; m < vsize; m++)
            {

                _tagGeodeParams.vertexes[m] = osg::Vec3d(_vertices.at(m).position.x,
                                                         _vertices.at(m).position.y,
                                                         _vertices.at(m).position.z);
                _tagGeodeParams.normals[m] = osg::Vec3d(_vertices.at(m).normal.x,
                                                        _vertices.at(m).normal.y,
                                                        _vertices.at(m).normal.z);
                _tagGeodeParams.texcoord[m] = osg::Vec2d(_vertices.at(m).textureCoords.x,
                                                         _vertices.at(m).textureCoords.y);
                _tagGeodeParams.colors[m] = osg::Vec4(1.0, 1.0, 1.0, 0.5);
            }

            tagGeodeParams::tagDrawIndicesParams _tagDrawIndicesParams;
            _tagDrawIndicesParams.mode = osg::PrimitiveSet::TRIANGLES;
            _tagDrawIndicesParams.indices.resize(_indices.size());
            for(int m = 0; m < _indices.size(); m++)
            {
                _tagDrawIndicesParams.indices[m] = _indices.at(m);
            }
            _tagGeodeParams.drawindices.push_back(std::move(_tagDrawIndicesParams));
            _tagGeodeParams.mode = osg::PrimitiveSet::TRIANGLES;

            osg::ref_ptr<osg::Geometry> pGeom = DrawFunc::createGeometry(_tagGeodeParams);

            osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
            pGeode->addChild(pGeom);


            osg::Vec3d fscalef(_scaleFactor, _scaleFactor, _scaleFactor);
            osg::ref_ptr<osg::MatrixTransform> nodeTransform = new osg::MatrixTransform;
            osg::Matrix matrixTmp = osg::Matrix::identity();
            matrixTmp *= osg::Matrix::scale(fscalef);
            matrixTmp *= osg::Matrix::translate(position);

            osg::StateSet* stateset = pGeom->getOrCreateStateSet();
            switch (eType) {
            case E_GEODE_TYPE_PLANET:
            {
            }break;
            case E_GEODE_TYPE_CLOUDS:
            {

                {
//                    DcreateGeometryShaderProgram(pGeom, vspath, gspath, fspath);
//                    DrawFunc::setMat4f(pGeom,"projection", _cameraProjection);
//                    DrawFunc::setMat4f(pGeom,"view", _cameraView);
//                    DrawFunc::setVec3f(pGeom,"viewPos", camera_position);
//                    DrawFunc::setVec3f(pGeom,"lightPos", sun_position);
//                    DrawFunc::setFloat(pGeom,"farPlane", camera_far);
//                    DrawFunc::setFloat(pGeom,"zCoef", zCoef);
//                    DrawFunc::setFloat(pGeom,"bias", 0.001);
//                    DrawFunc::setInt(pGeom,"shadowMap", 8);

//                    DrawFunc::setMat4f(pGeom,"lightSpaceMatrix", lightSpaceMatrix);
                }
                {
                    stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
                    stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

//                    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

                    stateset->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);
                }

            }break;
            case E_GEODE_TYPE_ATMOSPHERE:
            {
                DcreateGeometryShaderProgram(pGeom, vspath, gspath, fspath);

                pGeom->setUpdateCallback(new UpdateAtmosSphereCallback(m_pModelWidget));
                {
                    stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
                    stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
                    stateset->setAttributeAndModes(new osg::BlendFunc(GL_ONE, GL_ONE), osg::StateAttribute::ON);
                }
            }break;
            default:
                break;
            }
            if(!texturepath.empty())
            {
                setTex(pGeode, texturepath);
            }

            nodeTransform->setMatrix(matrixTmp);
            nodeTransform->addChild(pGeode);
            ///////////////////////////////
            pGroup->addChild(nodeTransform);
            meshs_itor++;
        }
        return pGroup;
    };

    QString path = QCoreApplication::applicationDirPath();
    std::string vspath = path.toStdString() + "/res/models/sphere.obj";
    std::string earth_day_texturePath = path.toStdString() + "/res/osgearth/Star/Earth_Day.jpg";
//    std::string earth_clods_texturePath = path.toStdString() + "/res/osgearth/Star/Earth_Clouds.jpg";
    std::string earth_clods_texturePath = "D:/earth/SolarSystem-3D-win32/resource/textures/Earth_Clouds_Diffuse.dds";


    osg::ref_ptr<osg::Group> pGroup = new osg::Group;
    pGroup->addChild(createGeode(E_GEODE_TYPE_PLANET,vspath, earth_day_texturePath, earth_position,1.0*_earthSizeCoefficient));
    pGroup->addChild(createGeode(E_GEODE_TYPE_CLOUDS,vspath, earth_clods_texturePath, earth_position, 1.0055*_earthSizeCoefficient, path.toStdString() +"/shaders/planetLighting.vs","",path.toStdString() +"/shaders/cloudsLighting.fs"));
    pGroup->addChild(createGeode(E_GEODE_TYPE_ATMOSPHERE,vspath, "", earth_position, 1.1*_earthSizeCoefficient, path.toStdString() +"/shaders/atmosphere.vs","",path.toStdString() +"/shaders/atmosphere.fs"));
#else

    osg::ref_ptr<osg::Geometry> pGeom = DrawFunc::createSphereGeometry(osg::Vec3d(0,0,0));
    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pGeode->addChild(pGeom);

    ///////////////////////////////
    std::string texturePath = path.toStdString() + "/res/osgearth/Star/Earth_Clouds.jpg";
    setTex(pGeode, texturePath);

    pGroup->addChild(pGeode);
#endif
    m_pModelWidget->pOSGManager()->getModelSenceData()->HideGroundScene(false);
    m_pModelWidget->pOSGManager()->getOsgHandler()->m_pCamera->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 0.5f));
    m_pModelWidget->pOSGManager()->getModelSenceData()->addNode(pGroup);
#endif

}

MainWindow::~MainWindow()
{
    delete ui;
}

