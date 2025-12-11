#ifndef QTMATERIALSTATUSBUTTON_H
#define QTMATERIALSTATUSBUTTON_H


#include "qtmaterialiconbutton.h"

class QtMaterialStatusButtonPrivate;

class  QtMaterialStatusButton : public QtMaterialIconButton
{
    Q_OBJECT

public:
    explicit QtMaterialStatusButton(QWidget *parent = nullptr);
    ~QtMaterialStatusButton();

    Q_PROPERTY(bool status WRITE setStatus READ status)

    void setStatus(const bool &status);
    bool status();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

    const QScopedPointer<QtMaterialStatusButtonPrivate> d_ptr;

private:
    Q_DISABLE_COPY(QtMaterialStatusButton)
    Q_DECLARE_PRIVATE(QtMaterialStatusButton)
};

#endif // QTMATERIALSTATUSBUTTON_H
