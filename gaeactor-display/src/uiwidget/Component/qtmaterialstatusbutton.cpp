#include "qtmaterialstatusbutton.h"
#include "qtmaterialstatusbutton_p.h"
#include <QPainter>

QtMaterialStatusButtonPrivate::QtMaterialStatusButtonPrivate(QtMaterialStatusButton *q)
    : q_ptr(q)
{
}

QtMaterialStatusButtonPrivate::~QtMaterialStatusButtonPrivate()
{
}

void QtMaterialStatusButtonPrivate::init()
{
    status = false;
}

QtMaterialStatusButton::QtMaterialStatusButton(QWidget *parent)
    : QtMaterialIconButton(QIcon(), parent)
    , d_ptr(new QtMaterialStatusButtonPrivate(this))
{
    d_func()->init();

    setIcon(QIcon(":/icons/exception_16x16.png"));
}

QtMaterialStatusButton::~QtMaterialStatusButton()
{

}

void QtMaterialStatusButton::setStatus(const bool &status)
{
    Q_D(QtMaterialStatusButton);

    d->status = status;
    update();
}

bool QtMaterialStatusButton::status()
{
    Q_D(const QtMaterialStatusButton);

    return d->status;
}

void QtMaterialStatusButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);

    QPixmap pixmap;
    if (status() == true)
    {
        pixmap = QIcon(":/icons/normal_16x16.png").pixmap(iconSize());
    }
    else
    {
        pixmap = QIcon(":/icons/exception_16x16.png").pixmap(iconSize());
    }

    QPainter icon(&pixmap);
    icon.setCompositionMode(QPainter::CompositionMode_SourceIn);
    icon.fillRect(pixmap.rect(), isEnabled() ? color() : disabledColor());

    QRect r(rect());
    const qreal w = pixmap.width();
    const qreal h = pixmap.height();
    painter.drawPixmap(QRect((r.width()-w)/2, (r.height()-h)/2, w, h), pixmap);
}
