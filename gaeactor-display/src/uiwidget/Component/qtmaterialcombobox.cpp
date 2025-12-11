#include "qtmaterialcombobox.h"
#include "qtmaterialcombobox_p.h"

QtMaterialComboBoxPrivate::QtMaterialComboBoxPrivate(QtMaterialComboBox *q)
    : q_ptr(q)
{

}

QtMaterialComboBoxPrivate::~QtMaterialComboBoxPrivate()
{

}

void QtMaterialComboBoxPrivate::init()
{
    Q_Q(QtMaterialComboBox);

    q_ptr->setStyleSheet("QComboBox{color: white;background-color: transparent;    border: 1px solid white;font-size: 18px;font-family: Microsoft YaHei;font-weight: 400;    padding-left: 10px;    max-width: 420px;    height: 32px;line-height: 32px;opacity: 0.99;}QComboBox:pressed{color: white;border: 1px solid white;background-color: rgba(15, 255, 255, 0.5);font-size: 20px;font-family: Microsoft YaHei;font-weight: 400;line-height: 32px;opacity: 0.99;}QComboBox:hover{color: white;border: 1px solid #FFFFFF;background-color: rgba(15, 255, 255, 0.25);font-size: 20px;font-family: Microsoft YaHei;font-weight: 400;line-height: 32px;opacity: 0.99;}QComboBox::drop-down{    width: 20px;    padding-right: 10px;    image: url(./apps/appcontainers/PhugiaSettings/res/icon/common_panel/white/down_24x24.png);}QComboBox QAbstractItemView{border: 1px solid #0FFFFF;background-color: transparent;}QComboBox QAbstractItemView::item{height: 32px;color: white;background-color: transparent;}QComboBox QAbstractItemView::item:selected{background-color:rgb(22, 37, 124);}QComboBox QScrollBar::vertical{width:8px;background:rgba(0,0,0,0%);background-color: transparent;margin:0px,0px,0px,0px;}QComboBox QScrollBar::handle::vertical{width:8px;background:rgba(0,0,0,50%);min-height:20px;}QComboBox QScrollBar::handle::vertical::hover{width:8px;background:rgba(0,0,0,75%);min-height:20px;}QComboBox QScrollBar::add-line::vertical{border:none;}QComboBox QScrollBar::sub-line::vertical{border:none;}");
}

QtMaterialComboBox::QtMaterialComboBox(QWidget *parent)
    : QComboBox(parent)
    , d_ptr(new QtMaterialComboBoxPrivate(this))
{
    d_func()->init();
}

QtMaterialComboBox::~QtMaterialComboBox()
{

}
