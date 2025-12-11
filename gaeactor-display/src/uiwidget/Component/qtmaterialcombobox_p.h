#ifndef QTMATERIALCOMBOBOX_P_H
#define QTMATERIALCOMBOBOX_P_H

#include <QtGlobal>

class QtMaterialComboBox;

class QtMaterialComboBoxPrivate
{
    Q_DISABLE_COPY(QtMaterialComboBoxPrivate)
    Q_DECLARE_PUBLIC(QtMaterialComboBox)

public:
    QtMaterialComboBoxPrivate(QtMaterialComboBox *q);
    virtual ~QtMaterialComboBoxPrivate();

    void init();

    QtMaterialComboBox *const q_ptr;
};

#endif // QTMATERIALCOMBOBOX_P_H
