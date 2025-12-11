/***************************************************************************
 * QGeoView is a Qt / C ++ widget for visualizing geographic data.
 * Copyright (C) 2018-2020 Andrey Yaroshenko.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see https://www.gnu.org/licenses.
 ****************************************************************************/

#pragma once

#include <QToolButton>

#include "QGVWidget.h"

class QGVWidgetTools : public QGVWidget
{
    Q_OBJECT

public:
    enum E_SELECT_TYPE
    {
        E_SELECT_TYPE_NULL,
        E_SELECT_TYPE_LINE,
        E_SELECT_TYPE_ELLIPSE,
        E_SELECT_TYPE_PIE,
        E_SELECT_TYPE_RECTANGLE,
        E_SELECT_TYPE_POLYGON,
        E_SELECT_TYPE_MOVE_TRACKING_LINE
    };
    QGVWidgetTools();

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation getOrientation() const;

    QToolButton* btnEllipse();
    QToolButton* btnPie();
    QToolButton* btnRectangle();
    QToolButton* btnPolygon();
    QToolButton* btnLine();
    QToolButton* btnTracking();

    E_SELECT_TYPE getSelectType() const;

private:
    QPixmap createPixmap(const QSize& size, const QString& text);
    void selectEllipse(bool checked);
    void selectPie(bool checked);
    void selectRectangle(bool checked);
    void selectPolygon(bool checked);
    void selectLine(bool checked);
    void selectTracking(bool checked);


protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    Qt::Orientation mOrientation;
    QScopedPointer<QToolButton> mButtonEllipse;
    QScopedPointer<QToolButton> mButtonPie;
    QScopedPointer<QToolButton> mButtonRectangle;
    QScopedPointer<QToolButton> mButtonPolygon;
    QScopedPointer<QToolButton> mButtonLine;
    QScopedPointer<QToolButton> mButtonTracking;

    E_SELECT_TYPE m_eSelectType;
};
