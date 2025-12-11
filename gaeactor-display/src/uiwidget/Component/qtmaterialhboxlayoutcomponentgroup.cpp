#include "qtmaterialhboxlayoutcomponentgroup.h"
#include "qtmaterialhboxlayoutcomponentgroup_p.h"

QtMaterialHBoxLayoutComponentGroupPrivate::QtMaterialHBoxLayoutComponentGroupPrivate(QtMaterialHBoxLayoutComponentGroup *q)
    : q_ptr(q)
{

}

QtMaterialHBoxLayoutComponentGroupPrivate::~QtMaterialHBoxLayoutComponentGroupPrivate()
{

}

void QtMaterialHBoxLayoutComponentGroupPrivate::init()
{
    Q_Q(QtMaterialHBoxLayoutComponentGroup);
}

QtMaterialHBoxLayoutComponentGroup::QtMaterialHBoxLayoutComponentGroup(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new QtMaterialHBoxLayoutComponentGroupPrivate(this))
    , m_pHBoxLayout(new QHBoxLayout(this))
{
    d_func()->init();

    m_pHBoxLayout->setSpacing(10);
    m_pHBoxLayout->setMargin(0);
}

QtMaterialHBoxLayoutComponentGroup::~QtMaterialHBoxLayoutComponentGroup()
{

}

void QtMaterialHBoxLayoutComponentGroup::addComponent(QWidget *pComponent)
{
    m_pHBoxLayout->addWidget(pComponent);
}

void QtMaterialHBoxLayoutComponentGroup::removeComponent(QWidget *pComponent)
{
    m_pHBoxLayout->removeWidget(pComponent);
}

