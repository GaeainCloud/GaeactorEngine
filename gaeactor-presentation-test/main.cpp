#if 1
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
#else



#include <Windows.h>
#include <iostream>
#include <string>

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>

#include <osgEarth/MapNode>
#include <osgEarth/ImageLayer>
#include <osgEarth/GLUtils>

#include <osgEarthDrivers/gdal/GDALOptions>
#include <osgEarthDrivers/cache_filesystem/FileSystemCache>
#include <osgEarthDrivers/feature_ogr/OGRFeatureOptions>

#include <osgEarthFeatures/FeatureSourceLayer>
#include <osgEarthFeatures/FeatureModelLayer>

#include <osgEarthUtil/EarthManipulator>

using namespace std;

void AddAnno(std::string filePath, osg::ref_ptr<osgEarth::Map> map)
{
    osgEarth::Symbology::Style labelStyle;

    osgEarth::Symbology::TextSymbol* text = labelStyle.getOrCreateSymbol<osgEarth::Symbology::TextSymbol>();
    string name = "[CNTRY_NAME]";       //如果需要显示汉字，则需要转换成UTF-8编码
    text->content() = osgEarth::Symbology::StringExpression(name);
    text->priority() = 1.0;//osgEarth::NumericExpression( "[pop_cntry]" );
    text->size() = 16.0f;
    text->alignment() = osgEarth::Symbology::TextSymbol::ALIGN_CENTER_CENTER;
    text->fill()->color() = osgEarth::Symbology::Color::White;
    text->halo()->color() = osgEarth::Symbology::Color::Red;
    text->encoding() = osgEarth::Symbology::TextSymbol::ENCODING_UTF8;
    //string fontFile = PathRef::GetAppDir() + "/fonts/SourceHanSansCN-Regular.ttf";
    //text->font() = fontFile;          //如果显示汉字，需要支持中文字库的字体

    // and configure a model layer:
    osgEarth::Features::FeatureModelLayerOptions fmlOpt;
    fmlOpt.name() = filePath + "_labels";
    fmlOpt.featureSourceLayer() = filePath + "_source";
    fmlOpt.styles() = new osgEarth::Symbology::StyleSheet();
    fmlOpt.styles()->addStyle(labelStyle);

    osg::ref_ptr<osgEarth::Features::FeatureModelLayer> fml = new osgEarth::Features::FeatureModelLayer(fmlOpt);
    map->addLayer(fml);
}

void AddVector(osg::ref_ptr<osgEarth::Map> map)
{
    //

//    std::string filePath = "D:/earth/shp/world.shp";
    std::string filePath = "D:/earth/gzsczt_fwjz_pro_cgcs2000/gz_50/gz_50.shp";
//    std::string filePath = "D:/earth/1/1.shp";

    osgEarth::Drivers::OGRFeatureOptions featureData;
    featureData.url() = filePath;

    //     如果缺少空间参考，可以手动指定
    //    ifstream infile("C:/Data/vector/hs/23.prj");
    //    string line;
    //    getline(infile, line);
    //    featureData.profile()->srsString() = line;

    // Make a feature source layer and add it to the Map:
    osgEarth::Features::FeatureSourceLayerOptions ogrLayer;
    ogrLayer.name() = filePath + "_source";
    ogrLayer.featureSource() = featureData;
    osgEarth::Features::FeatureSourceLayer*  featureSourceLayer = new osgEarth::Features::FeatureSourceLayer(ogrLayer);
    map->addLayer(featureSourceLayer);
    osgEarth::Features::FeatureSource *features = featureSourceLayer->getFeatureSource();
    if (!features)
    {
        printf(("无法打开该矢量文件！"));
        return;
    }

    //设置样式
    osgEarth::Symbology::Style style;

    //可见性
    osgEarth::Symbology::RenderSymbol* rs = style.getOrCreate<osgEarth::Symbology::RenderSymbol>();
    rs->depthTest() = false;

    //贴地设置
    osgEarth::Symbology::AltitudeSymbol* alt = style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
    alt->clamping() = alt->CLAMP_TO_TERRAIN;
    alt->technique() = alt->TECHNIQUE_DRAPE;

    //设置矢量面样式（包括边界线）
    osgEarth::Symbology::LineSymbol* ls = style.getOrCreateSymbol<osgEarth::Symbology::LineSymbol>();
    ls->stroke()->color() = osgEarth::Symbology::Color("#FA8072");
    ls->stroke()->width() = 1.0;
    ls->tessellationSize()->set(100, osgEarth::Units::KILOMETERS);

    osgEarth::Symbology::PolygonSymbol *polygonSymbol = style.getOrCreateSymbol<osgEarth::Symbology::PolygonSymbol>();
    polygonSymbol->fill()->color() = osgEarth::Symbology::Color(152.0f / 255, 251.0f / 255, 152.0f / 255, 0.8f); //238 230 133
    polygonSymbol->outline() = true;

    //
    osgEarth::Features::FeatureModelLayerOptions fmlOpt;
    fmlOpt.name() = filePath;
    fmlOpt.featureSourceLayer() = filePath + "_source";
    fmlOpt.enableLighting() = false;
    fmlOpt.styles() = new osgEarth::Symbology::StyleSheet();
    fmlOpt.styles()->addStyle(style);

    osg::ref_ptr<osgEarth::Features::FeatureModelLayer> fml = new osgEarth::Features::FeatureModelLayer(fmlOpt);
    map->addLayer(fml);

//    AddAnno(filePath, map);
}

int main()
{
    osgEarth::ProfileOptions profileOpts;

    //地图配置：设置缓存目录
    osgEarth::Drivers::FileSystemCacheOptions cacheOpts;
    string cacheDir = "D:/Work/OSGNewBuild/tmp";
    cacheOpts.rootPath() = cacheDir;

    //
    osgEarth::MapOptions mapOpts;
    mapOpts.cache() = cacheOpts;
    mapOpts.profile() = profileOpts;

    //创建地图节点
    osg::ref_ptr<osgEarth::Map> map = new osgEarth::Map(mapOpts);
    osg::ref_ptr<osgEarth::MapNode> mapNode = new osgEarth::MapNode(map);

    osgEarth::Drivers::GDALOptions gdal;
    gdal.url() = "D:/earth/world.tif";
    osg::ref_ptr<osgEarth::ImageLayer> layer = new osgEarth::ImageLayer("BlueMarble", gdal);
    map->addLayer(layer);

    AddVector(map);

    osgViewer::Viewer viewer;
    viewer.setSceneData(mapNode);

    osg::ref_ptr< osgEarth::Util::EarthManipulator> mainManipulator = new osgEarth::Util::EarthManipulator;
    viewer.setCameraManipulator(mainManipulator);

    //解决Lines or Annotations (FeatureNode, etc.) 不被渲染的问题
    osgEarth::GLUtils::setGlobalDefaults(viewer.getCamera()->getOrCreateStateSet());

    viewer.setUpViewInWindow(100, 100, 800, 600);
    return viewer.run();
}
#endif
