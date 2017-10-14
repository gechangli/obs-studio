/******************************************************************************
    Copyright (C) 2013 by luma <stubma@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "xl-clickable-widget.hpp"
#include <QStyleOption>
#include <QPainter>

XLClickableWidget::XLClickableWidget(QWidget* parent, Qt::WindowFlags f) :
	QWidget(parent, f) {
}

XLClickableWidget::~XLClickableWidget() {
}

void XLClickableWidget::mousePressEvent(QMouseEvent* event) {
	emit clicked();
}

void XLClickableWidget::paintEvent(QPaintEvent* event) {
	// draw with style
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}