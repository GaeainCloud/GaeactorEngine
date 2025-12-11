#include "qtmaterialtextbutton.h"
#include "qtmaterialtextbutton_p.h"
#include <QMouseEvent>

QtMaterialTextButtonPrivate::QtMaterialTextButtonPrivate(QtMaterialTextButton *q)
    : q_ptr(q)
{

}

QtMaterialTextButtonPrivate::~QtMaterialTextButtonPrivate()
{

}

void QtMaterialTextButtonPrivate::init()
{
    Q_Q(QtMaterialTextButton);

    q_ptr->setHoverColor(QColor(125, 251, 253));
}

QtMaterialTextButton::QtMaterialTextButton(QWidget *parent)
    : QtMaterialText(parent)
    , d_ptr(new QtMaterialTextButtonPrivate(this))
{
    d_func()->init();
}

QtMaterialTextButton::~QtMaterialTextButton()
{

}

void QtMaterialTextButton::setText(const QString &text)
{
    QLabel::setText(text);

    int length = text.length();

    QFont font = this->font();
    int size = font.pointSize();

    QFontMetrics fontMerics(font);
    QRect rect = fontMerics.boundingRect(text);
    int textWidth = rect.width();   // 字符串所占像素宽度
    int textHeight = rect.height();

    setFixedSize(textWidth, textHeight);       // 设置文字宽度，以相应鼠标进入和离开文字区域时的颜色变化事件
}

void QtMaterialTextButton::setHoverColor(const QColor &color)
{
    Q_D(QtMaterialTextButton);

    d_ptr->hoverColor = color;
    update();
}

QColor QtMaterialTextButton::hoverColor()
{
    Q_D(QtMaterialTextButton);

    return d_ptr->hoverColor;
}

void QtMaterialTextButton::setClickColor(const QColor &color)
{
    Q_D(QtMaterialTextButton);

    d_ptr->clickColor = color;
}

QColor QtMaterialTextButton::clickColor()
{
    Q_D(QtMaterialTextButton);

    return d_ptr->clickColor;
}

void QtMaterialTextButton::enterEvent(QEvent *event)
{
    setColor(QColor(Qt::white));
}

void QtMaterialTextButton::leaveEvent(QEvent *event)
{
    setColor(QColor(Qt::green));
}

void QtMaterialTextButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked();
    }
}

