#ifndef QTMATERIALSTATUSBUTTON_P_H
#define QTMATERIALSTATUSBUTTON_P_H

#include <QtGlobal>

class QtMaterialStatusButton;
class QtMaterialIconButton;

class QtMaterialStatusButtonPrivate
{
    Q_DISABLE_COPY(QtMaterialStatusButtonPrivate)
    Q_DECLARE_PUBLIC(QtMaterialStatusButton)

public:
    QtMaterialStatusButtonPrivate(QtMaterialStatusButton *q);
    virtual ~QtMaterialStatusButtonPrivate();

    void init();

    QtMaterialStatusButton *const q_ptr;
    bool status;
};

#endif // QTMATERIALSTATUSBUTTON_P_H
