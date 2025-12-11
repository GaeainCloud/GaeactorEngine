#include "qtmateriallineedit.h"
#include "qtmateriallineedit_p.h"

QtMaterialLineEditPrivate::QtMaterialLineEditPrivate(QtMaterialLineEdit *q)
    : q_ptr(q)
{

}

QtMaterialLineEditPrivate::~QtMaterialLineEditPrivate()
{

}

void QtMaterialLineEditPrivate::init()
{
    Q_Q(QtMaterialLineEdit);

    q_ptr->setStyleSheet("border: 1px solid white; color: white; padding-left: 10px;");
    q_ptr->setFixedSize(420, 32);
}

QtMaterialLineEdit::QtMaterialLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , d_ptr(new QtMaterialLineEditPrivate(this))
{
    d_func()->init();
}

QtMaterialLineEdit::~QtMaterialLineEdit()
{

}

void QtMaterialLineEdit::setBorderColor(const QColor &borderColor)
{
    Q_D(QtMaterialLineEdit);

    d_ptr->borderColor = borderColor;
    update();
}

QColor QtMaterialLineEdit::borderColor()
{
    Q_D(const QtMaterialLineEdit);

    return d->borderColor;
}

void QtMaterialLineEdit::setBackgroundColor(const QColor &backgroundColor)
{
    Q_D(QtMaterialLineEdit);

    d_ptr->backgroundColor = backgroundColor;
    update();
}

QColor QtMaterialLineEdit::backgroundColor()
{
    Q_D(const QtMaterialLineEdit);

    return d_ptr->backgroundColor;
}

void QtMaterialLineEdit::setTextColor(const QColor &textColor)
{
    Q_D(QtMaterialLineEdit);

    d_ptr->textColor = textColor;
}

QColor QtMaterialLineEdit::textColor()
{
    Q_D(const QtMaterialLineEdit);

    return d_ptr->textColor;
}

void QtMaterialLineEdit::setWidth(const int &width)
{
    Q_D(QtMaterialLineEdit);

    d_ptr->width = width;
}

int QtMaterialLineEdit::width()
{
    Q_D(const QtMaterialLineEdit);

    return d_ptr->width;
}
