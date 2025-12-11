#ifndef QTMATERIALTEXTPRIVATE_H
#define QTMATERIALTEXTPRIVATE_H

#include <QtGlobal>
#include <QWidget>

#include "qtmaterialtext.h"

class QtMaterialText;

class QtMaterialTextPrivate
{
    Q_DISABLE_COPY(QtMaterialTextPrivate)
    Q_DECLARE_PUBLIC(QtMaterialText)

public:
    QtMaterialTextPrivate(QtMaterialText *q);
    ~QtMaterialTextPrivate();

    void init();

    QtMaterialText *const q_ptr;
    TextFontSize fontSize;
    QColor color;
    QFont font;
};

#endif // QTMATERIALTEXTPRIVATE_H
