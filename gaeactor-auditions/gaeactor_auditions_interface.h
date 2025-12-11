#ifndef GAEACTOR_AUDITIONS_INTERFACE_H
#define GAEACTOR_AUDITIONS_INTERFACE_H

#include "gaeactor_auditions_global.h"
#include <QObject>
#include "gaeactor_auditions_define.h"
#include <QHash>
#include <QList>
#include <QVector>

namespace gaeactorauditions {

class GAEACTOR_AUDITIONS_EXPORT GaeactorAuditions : public QObject
{
    Q_OBJECT
public:
    static GaeactorAuditions & getInstance();
    virtual ~GaeactorAuditions();

    void setCheckEnable(bool newBChecking);
    void refreshAuditions();
    void registDisplayCallback(echowave_display_hexidx_update_callback func);

    void registHexidxDisplayCallback(display_hexidx_update_callback func);
private:
    explicit GaeactorAuditions(QObject *parent = nullptr);


};
}
#endif // GAEACTOR_AUDITIONS_INTERFACE_H
