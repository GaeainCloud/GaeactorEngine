#ifndef GAEACTOR_INTERACTIONS_INTERFACE_H
#define GAEACTOR_INTERACTIONS_INTERFACE_H

#include "gaeactor_interactions_global.h"
#include <QObject>
#include "gaeactor_interactions_define.h"
#include <QHash>
#include <QList>
#include <QVector>

namespace gaeactorinteractions {
class GAEACTOR_INTERACTIONS_EXPORT GaeactorInteractions : public QObject
{
    Q_OBJECT
public:
    static GaeactorInteractions & getInstance();
    virtual ~GaeactorInteractions();

    void setCheckEnable(bool newBChecking);
    void refreshInteractions(int id);
    void registDisplayCallback(echowave_display_hexidx_update_callback func);
    void registDisplayListCallback(echowave_list_display_hexidx_update_callback func);
    void registHexidxDisplayCallback(display_hexidx_update_callback func);
private:
    explicit GaeactorInteractions(QObject *parent = nullptr);
};
}
#endif // GAEACTOR_INTERACTIONS_INTERFACE_H
