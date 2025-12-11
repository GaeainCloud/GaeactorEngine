#ifndef QTMATERIALTEXT_H
#define QTMATERIALTEXT_H



#include <QLabel>

enum TextFontSize                      // 文字大小，单位：像素
{
    FONT_SIZE_TITLE_HEAD = 32,         // 头部banner标题
    FONT_SIZE_TITLE_FIRST = 28,        // 一级标题
    FONT_SIZE_TITLE_SECOND = 18,       // 二级标题
    FONT_SIZE_TITLE_THIRD = 16,        // 三级标题
    FONT_SIZE_TITLE_FOURTH = 14,       // 四级标题
    FONT_SIZE_CONTENT = 12             // 内容文字
};

class QtMaterialTextPrivate;

class  QtMaterialText : public QLabel
{
    Q_OBJECT

    Q_PROPERTY(TextFontSize textFontSize WRITE setFontSize READ fontSize)
    Q_PROPERTY(QColor color WRITE setColor READ color)
    Q_PROPERTY(QFont font WRITE setFont READ font)

public:
    explicit QtMaterialText(QWidget *parent = nullptr);
    ~QtMaterialText();

    void setText(const QString &text);

    void setFontSize(const TextFontSize &fontSize);
    TextFontSize fontSize();

    void setColor(const QColor &color);
    QColor color();

    void setFont(const QFont &font);
    QFont font();

protected:
    const QScopedPointer<QtMaterialTextPrivate> d_ptr;

private:
    Q_DISABLE_COPY(QtMaterialText)
    Q_DECLARE_PRIVATE(QtMaterialText)
};

#endif // QTMATERIALTEXT_H
