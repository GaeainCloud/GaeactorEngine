#include "qtmaterialtext.h"
#include "qtmaterialtext_p.h"

QtMaterialTextPrivate::QtMaterialTextPrivate(QtMaterialText *q)
    : q_ptr(q)
{

}

QtMaterialTextPrivate::~QtMaterialTextPrivate()
{

}

void QtMaterialTextPrivate::init()
{
    Q_Q(QtMaterialText);

    QFont font;
    font.setFamily("microsoft YaHei");
    q->setFont(font);
}

QtMaterialText::QtMaterialText(QWidget *parent)
    : QLabel(parent)
    , d_ptr(new QtMaterialTextPrivate(this))
{
    d_func()->init();
}

QtMaterialText::~QtMaterialText()
{

}

void QtMaterialText::setText(const QString &text)
{
    QLabel::setText(text);
}

void QtMaterialText::setFontSize(const TextFontSize &fontSize)
{
    Q_D(QtMaterialText);

    d->fontSize = fontSize;
    update();

    QFont font;
    switch (fontSize)
    {
    case FONT_SIZE_TITLE_HEAD:
        {
            font.setPointSize(FONT_SIZE_TITLE_HEAD);
            font.setBold(true);
            QLabel::setFont(font);
            setFixedHeight(140);
            setContentsMargins(0, 64, 0, 22);
            setStyleSheet("background-color: blue;");
            break;
        }
    case FONT_SIZE_TITLE_FIRST:        // 一级标题
        {
            font.setPointSize(FONT_SIZE_TITLE_FIRST);
            font.setBold(true);
            QLabel::setFont(font);
            setFixedHeight(112);
            setContentsMargins(0, 45, 0, 22);
            setStyleSheet("background-color: red;");
            break;
        }
    case FONT_SIZE_TITLE_SECOND:       // 二级标题
        {
            font.setPointSize(FONT_SIZE_TITLE_SECOND);
            font.setBold(true);
            QLabel::setFont(font);
            setFixedHeight(88);
            setContentsMargins(0, 38, 0, 22);
            setStyleSheet("background-color: orange;");
            break;
        }
    case FONT_SIZE_TITLE_THIRD:        // 三级标题
        {
            font.setPointSize(FONT_SIZE_TITLE_THIRD);
            font.setBold(true);
            QLabel::setFont(font);
            setFixedHeight(70);
            setContentsMargins(0, 22, 0, 22);
            setStyleSheet("background-color: red;");
            break;
        }
    case FONT_SIZE_TITLE_FOURTH:       // 四级标题
        {
            font.setPointSize(FONT_SIZE_TITLE_FOURTH);
            font.setBold(true);
            QLabel::setFont(font);
            setFixedHeight(64);
            setContentsMargins(0, 20, 0, 22);
            setStyleSheet("background-color: blue;");
            break;
        }
    case FONT_SIZE_CONTENT:            // 内容文字
        {
            font.setPointSize(FONT_SIZE_CONTENT);
            font.setBold(true);
            QLabel::setFont(font);
            setFixedHeight(200);
            setWordWrap(true);         // 根据单词断行
            setStyleSheet("background-color: orange;");
            break;
        }
    }
}

TextFontSize QtMaterialText::fontSize()
{
    Q_D(QtMaterialText);

    return d->fontSize;
}

void QtMaterialText::setColor(const QColor &color)
{
    Q_D(QtMaterialText);

    d->color = color;

    QPalette palette;
    palette.setColor(QPalette::WindowText, color);
    setPalette(palette);

    update();
}

QColor QtMaterialText::color()
{
    Q_D(QtMaterialText);

    return d->color;
}

void QtMaterialText::setFont(const QFont &font)
{
    Q_D(QtMaterialText);

    d->font = font;
    update();
}

QFont QtMaterialText::font()
{
    Q_D(QtMaterialText);

    return d->font;
}
