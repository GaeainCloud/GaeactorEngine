#ifndef DEMSERVICESTANDALONE_H
#define DEMSERVICESTANDALONE_H

#include <QString>
#include <QtCore>
//
class DemserviceStandalone
{
public:
    static const float invalid_value;      //无效值 无效点的时候返回
    static DemserviceStandalone& getInstance();
    void   initializeElevationPath(QString fileFolder);
    QString elevationPath();
    float  getElevation(float lo, float la);
    double  getElevation(double lo, double la);
    float  getElevation(QPointF lola);
    QList<float>  getElevations(QList<QPointF> geoPonts);

private:
    DemserviceStandalone();
};

#endif // DEMSERVICESTANDALONE_H
