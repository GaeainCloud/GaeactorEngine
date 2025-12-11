#ifndef QTMATERIALLINEEDIT_H
#define QTMATERIALLINEEDIT_H



#include <QtGlobal>
#include <QLineEdit>

class QtMaterialLineEditPrivate;

class  QtMaterialLineEdit : public QLineEdit
{
    Q_OBJECT

    Q_PROPERTY(QColor borderColor WRITE setBorderColor READ borderColor)
    Q_PROPERTY(QColor backgroundColor WRITE setBackgroundColor READ backgroundColor)
    Q_PROPERTY(QColor textColor WRITE setTextColor READ textColor)
    Q_PROPERTY(int width WRITE setWidth READ width)

public:
    explicit QtMaterialLineEdit(QWidget *parent = nullptr);
    virtual ~QtMaterialLineEdit();

    void setBorderColor(const QColor &borderColor);
    QColor borderColor();

    void setBackgroundColor(const QColor &backgroundColor);
    QColor backgroundColor();

    void setTextColor(const QColor &textColor);
    QColor textColor();

    void setWidth(const int &width);
    int width();

protected:
    const QScopedPointer<QtMaterialLineEditPrivate> d_ptr;

private:
    Q_DISABLE_COPY(QtMaterialLineEdit)
    Q_DECLARE_PRIVATE(QtMaterialLineEdit)
};

#endif // QTMATERIALLINEEDIT_H
