#ifndef QTMATERIALTEXTBUTTON_H
#define QTMATERIALTEXTBUTTON_H

#include "qtmaterialtext.h"


#include <QtGlobal>

class QtMaterialTextButtonPrivate;

class  QtMaterialTextButton : public QtMaterialText
{
    Q_OBJECT

    Q_PROPERTY(QColor hoverColor WRITE setHoverColor READ hoverColor)
    Q_PROPERTY(QColor clickColor WRITE setClickColor READ clickColor)

public:
    explicit QtMaterialTextButton(QWidget *parent = nullptr);
    ~QtMaterialTextButton();

    void setText(const QString &text);

    void setHoverColor(const QColor &color);
    QColor hoverColor();

    void setClickColor(const QColor &color);
    QColor clickColor();

signals:
    void clicked();

protected:
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

protected:
    const QScopedPointer<QtMaterialTextButtonPrivate> d_ptr;

private:
    Q_DISABLE_COPY(QtMaterialTextButton)
    Q_DECLARE_PRIVATE(QtMaterialTextButton)
};

#endif // QTMATERIALTEXTBUTTON_H
