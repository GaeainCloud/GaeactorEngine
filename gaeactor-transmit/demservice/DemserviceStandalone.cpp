#include "DemserviceStandalone.h"
#include <QDir>
#include <iostream>
extern "C" {
#include "srtmHgtReader.h"
}

DemserviceStandalone &DemserviceStandalone::getInstance()
{
    static DemserviceStandalone instance;
    {
        if(instance.elevationPath() == "srtmsss"){
#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            instance.initializeElevationPath("C:/Workplace/GaeaWorkshop/HGT");
#else
            instance.initializeElevationPath("/home/lavic/core/serviceData/data/HGT");
#endif
        }
    }
    return instance;
}

void DemserviceStandalone::initializeElevationPath(QString fileFolder)
{
    // auto dpath = "./HGT";
    // if(QDir(dpath).entryInfoList().count() > 0)
    // {
    //     fileFolder = dpath;
    // }
    setFolder(fileFolder.toStdString().c_str(),fileFolder.size());
}

QString DemserviceStandalone::elevationPath()
{
    return getFolder();
}

float DemserviceStandalone::getElevation(float lo, float la)
{
    return  srtmGetElevation(la,lo);
}

double DemserviceStandalone::getElevation(double lo, double la)
{
    auto lof = static_cast<float>(lo);
    auto laf = static_cast<float>(la);

    // if both 0 return 0
    if(fabs(lof) < 1e-3 && fabs(laf) < 1e-3){
        return 0;
    }
    auto elv = getElevation(lof, laf);
    if(fabs(elv) > 1e4){
        elv = 0;
    }
    return static_cast<double>(elv);
}

float DemserviceStandalone::getElevation(QPointF lola)
{
    return srtmGetElevation(lola.y(), lola.x());
}

QList<float> DemserviceStandalone::getElevations(QList<QPointF> geoPonts)
{
    QList<float> result;
    for(auto p:geoPonts)
    {
        result.push_back(srtmGetElevation(p.y(),p.x()));
    }
    return result;
}


const float DemserviceStandalone::invalid_value = INVALID_VALUE;

DemserviceStandalone::DemserviceStandalone()
{

}
