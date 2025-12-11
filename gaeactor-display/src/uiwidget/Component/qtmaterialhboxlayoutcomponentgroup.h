#ifndef QTMATERIALHBOXLAYOUTCOMPONENTGROUP_H
#define QTMATERIALHBOXLAYOUTCOMPONENTGROUP_H


#include <QHBoxLayout>
#include <QtGlobal>
#include <QWidget>

class QtMaterialHBoxLayoutComponentGroupPrivate;

class  QtMaterialHBoxLayoutComponentGroup : public QWidget
{
    Q_OBJECT

public:
    explicit QtMaterialHBoxLayoutComponentGroup(QWidget *parent = nullptr);
    ~QtMaterialHBoxLayoutComponentGroup();

    void addComponent(QWidget *pComponent);
    void removeComponent(QWidget *pComponent);

protected:
    const QScopedPointer<QtMaterialHBoxLayoutComponentGroupPrivate> d_ptr;
    QHBoxLayout *m_pHBoxLayout;

private:
    Q_DISABLE_COPY(QtMaterialHBoxLayoutComponentGroup)
    Q_DECLARE_PRIVATE(QtMaterialHBoxLayoutComponentGroup)
};

#endif // QTMATERIALHBOXLAYOUTCOMPONENTGROUP_H
