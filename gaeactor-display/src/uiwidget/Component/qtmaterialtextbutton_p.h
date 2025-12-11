#ifndef QTMATERIALTEXTBUTTON_P_H
#define QTMATERIALTEXTBUTTON_P_H

class QtMaterialTextButton;

class QtMaterialTextButtonPrivate
{
    Q_DISABLE_COPY(QtMaterialTextButtonPrivate)
    Q_DECLARE_PUBLIC(QtMaterialTextButton)

public:
    explicit QtMaterialTextButtonPrivate(QtMaterialTextButton *q);
    ~QtMaterialTextButtonPrivate();

    void init();

    QtMaterialTextButton *const q_ptr;
    QColor hoverColor;
    QColor clickColor;
};

#endif // QTMATERIALTEXTBUTTON_P_H
