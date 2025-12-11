#ifndef QTMATERIALLINEEDIT_P_H
#define QTMATERIALLINEEDIT_P_H

#include <QtGlobal>

class QtMaterialLineEdit;

class QtMaterialLineEditPrivate
{
    Q_DISABLE_COPY(QtMaterialLineEditPrivate)
    Q_DECLARE_PUBLIC(QtMaterialLineEdit)

public:
    explicit QtMaterialLineEditPrivate(QtMaterialLineEdit *q);
    virtual ~QtMaterialLineEditPrivate();

    void init();

    QtMaterialLineEdit *const q_ptr;
    QColor borderColor;
    QColor backgroundColor;
    QColor textColor;
    int width;
};

#endif // QTMATERIALLINEEDIT_P_H
