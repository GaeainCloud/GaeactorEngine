#pragma execution_character_set("utf-8")
#include "QGVWidgetTools.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QImage>
#include <QPainter>
#include <QVBoxLayout>
#include <QtMath>
#include <QStyleOption>
namespace {
QSize iconSize = QSize(64, 64);
double zoomExponentDown = qPow(2, 1.0 / 5.0);
double zoomExponentUp = 1.0 / qPow(2, 1.0 / 5.0);
}

#define DECLAR_BTN(NAME) \
mButton##NAME.reset(new QToolButton());\
    mButton##NAME->setAutoRepeat(false);\
    mButton##NAME->setIconSize(iconSize);\
    mButton##NAME->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);\
    mButton##NAME->setCheckable(true);\
    mButton##NAME->setIcon(QIcon(createPixmap(iconSize, #NAME)));\
    connect(mButton##NAME.data(), &QToolButton::clicked, this, &QGVWidgetTools::select##NAME);

QGVWidgetTools::QGVWidgetTools()
    :m_eSelectType(E_SELECT_TYPE_NULL)
{

    this->setStyleSheet("QToolButton{\
                        background-color:transparent;\
                        color:rgba(255,255,255, 60%);\
                        font-size: 20px;\
                        }\
                        QToolButton::checked{\
                        background-color:#2F8DED;\
                        font-size: 20px;\
                        }\
                        QToolButton::pressed{\
                        background-color:#2F8DED;\
                        font-size: 20px;\
                        }");
    DECLAR_BTN(Ellipse)
    mButtonEllipse->setIcon(QIcon("./res/AWACS_1.png"));
    DECLAR_BTN(Pie)
    mButtonPie->setIcon(QIcon("./res/radar_3.png"));
    DECLAR_BTN(Rectangle)
    mButtonRectangle->setIcon(QIcon("./res/satellite_2.png"));
    DECLAR_BTN(Polygon)
    mButtonPolygon->setIcon(QIcon("./res/data_link.png"));
    DECLAR_BTN(Line)
    mButtonLine->setIcon(QIcon("./res/waypoints_line.png"));
    DECLAR_BTN(Tracking)
    mButtonTracking->setIcon(QIcon("./res/PathFollow.png"));

    setOrientation(Qt::Horizontal);
    setAnchor(QPoint(0, 0), { Qt::BottomEdge });
}

void QGVWidgetTools::setOrientation(Qt::Orientation orientation)
{
    if (layout() != nullptr) {
        delete layout();
    }
    mOrientation = orientation;
    if (mOrientation == Qt::Horizontal) {
        setLayout(new QHBoxLayout(this));
    } else {
        setLayout(new QVBoxLayout(this));
    }
    layout()->setSpacing(0);
    layout()->setSizeConstraint(QLayout::SetMinimumSize);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(mButtonEllipse.data());
    layout()->addWidget(mButtonPie.data());
    layout()->addWidget(mButtonRectangle.data());
    layout()->addWidget(mButtonPolygon.data());
    layout()->addWidget(mButtonLine.data());
    layout()->addWidget(mButtonTracking.data());
}

Qt::Orientation QGVWidgetTools::getOrientation() const
{
    return mOrientation;
}

QToolButton* QGVWidgetTools::btnEllipse()
{
    return mButtonEllipse.data();
}

QToolButton* QGVWidgetTools::btnPie()
{
    return mButtonPie.data();
}

QToolButton* QGVWidgetTools::btnRectangle()
{
    return mButtonRectangle.data();
}

QToolButton* QGVWidgetTools::btnPolygon()
{
    return mButtonPolygon.data();
}

QToolButton *QGVWidgetTools::btnLine()
{
    return mButtonLine.data();
}

QToolButton *QGVWidgetTools::btnTracking()
{
    return mButtonTracking.data();
}

QPixmap QGVWidgetTools::createPixmap(const QSize& size, const QString& text)
{
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(qRgba(0, 0, 0, 0));
    QPixmap pixmap = QPixmap::fromImage(image, Qt::NoFormatConversion);
    QPainter painter(&pixmap);
    QPen pen = QPen(Qt::black);
    pen.setWidth(1);
    pen.setCosmetic(true);
    painter.setPen(pen);
    QBrush brush = QBrush(Qt::blue);
    painter.setBrush(brush);
    const auto path = QGV::createTextPath(QRect(QPoint(0, 0), size), text, font(), pen.width());
    painter.drawPath(path);
    return pixmap;
}

void QGVWidgetTools::selectEllipse(bool checked)
{
    if(checked)
    {
        mButtonLine->setChecked(false);
        mButtonEllipse->setChecked(true);
        mButtonPie->setChecked(false);
        mButtonRectangle->setChecked(false);
        mButtonPolygon->setChecked(false);
        mButtonTracking->setChecked(false);
        m_eSelectType = E_SELECT_TYPE_ELLIPSE;
    }
    else
    {
        m_eSelectType = E_SELECT_TYPE_NULL;
    }
}

void QGVWidgetTools::selectPie(bool checked)
{
    if(checked)
    {
        mButtonLine->setChecked(false);
        mButtonEllipse->setChecked(false);
        mButtonPie->setChecked(true);
        mButtonRectangle->setChecked(false);
        mButtonPolygon->setChecked(false);
        mButtonTracking->setChecked(false);
        m_eSelectType = E_SELECT_TYPE_PIE;
    }
    else
    {
        m_eSelectType = E_SELECT_TYPE_NULL;
    }
}

void QGVWidgetTools::selectRectangle(bool checked)
{
    if(checked)
    {
        mButtonLine->setChecked(false);
        mButtonEllipse->setChecked(false);
        mButtonPie->setChecked(false);
        mButtonRectangle->setChecked(true);
        mButtonPolygon->setChecked(false);
        mButtonTracking->setChecked(false);
        m_eSelectType = E_SELECT_TYPE_RECTANGLE;
    }
    else
    {
        m_eSelectType = E_SELECT_TYPE_NULL;
    }
}

void QGVWidgetTools::selectPolygon(bool checked)
{
    if(checked)
    {
        mButtonLine->setChecked(false);
        mButtonEllipse->setChecked(false);
        mButtonPie->setChecked(false);
        mButtonRectangle->setChecked(false);
        mButtonPolygon->setChecked(true);
        mButtonTracking->setChecked(false);
        m_eSelectType = E_SELECT_TYPE_POLYGON;
    }
    else
    {
        m_eSelectType = E_SELECT_TYPE_NULL;
    }
}

void QGVWidgetTools::selectLine(bool checked)
{
    if(checked)
    {
        mButtonLine->setChecked(true);
        mButtonEllipse->setChecked(false);
        mButtonPie->setChecked(false);
        mButtonRectangle->setChecked(false);
        mButtonPolygon->setChecked(false);
        mButtonTracking->setChecked(false);
        m_eSelectType = E_SELECT_TYPE_LINE;
    }
    else
    {
        m_eSelectType = E_SELECT_TYPE_NULL;
    }
}

void QGVWidgetTools::selectTracking(bool checked)
{
    if(checked)
    {
        mButtonTracking->setChecked(true);
        mButtonLine->setChecked(false);
        mButtonEllipse->setChecked(false);
        mButtonPie->setChecked(false);
        mButtonRectangle->setChecked(false);
        mButtonPolygon->setChecked(false);
        m_eSelectType = E_SELECT_TYPE_MOVE_TRACKING_LINE;
    }
    else
    {
        m_eSelectType = E_SELECT_TYPE_NULL;
    }
}


void QGVWidgetTools::paintEvent(QPaintEvent *event)
{
    {
        QStyleOption opt;
        opt.init(this);
        QPainter p(this);
        this->style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }

    return QGVWidget::paintEvent(event);
}

QGVWidgetTools::E_SELECT_TYPE QGVWidgetTools::getSelectType() const
{
    return m_eSelectType;
}

