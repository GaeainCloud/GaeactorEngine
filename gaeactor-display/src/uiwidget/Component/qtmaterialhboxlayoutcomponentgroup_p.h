#ifndef QTMATERIALHBOXLAYOUTCOMPONENTGROUP_P_H
#define QTMATERIALHBOXLAYOUTCOMPONENTGROUP_P_H

class QtMaterialHBoxLayoutComponentGroupPrivate
{
    Q_DISABLE_COPY(QtMaterialHBoxLayoutComponentGroupPrivate)
    Q_DECLARE_PUBLIC(QtMaterialHBoxLayoutComponentGroup)

public:
    QtMaterialHBoxLayoutComponentGroupPrivate(QtMaterialHBoxLayoutComponentGroup *q);
    ~QtMaterialHBoxLayoutComponentGroupPrivate();

    void init();

    QtMaterialHBoxLayoutComponentGroup *const q_ptr;
};

#endif // QTMATERIALHBOXLAYOUTCOMPONENTGROUP_P_H
