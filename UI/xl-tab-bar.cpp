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

#include "xl-tab-bar.hpp"
#include "xl-title-bar.hpp"
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStyle>
#include <QStyleOption>

XLTabBar::XLTabBar(QWidget *parent) :
	QWidget(parent),
	m_titleBar(Q_NULLPTR) {
}

XLTabBar::~XLTabBar() {
}

void XLTabBar::setTitleBar(XLTitleBar* titleBar) {
	m_titleBar = titleBar;
}

void XLTabBar::paintEvent(QPaintEvent* event) {
	// draw with style
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void XLTabBar::mouseDoubleClickEvent(QMouseEvent *event) {
	if (m_titleBar->hasMaxButton()) {
		if (m_titleBar->m_maxButton->isVisible()) {
			m_titleBar->onMaxClicked();
		} else {
			m_titleBar->onRestoreClicked();
		}
	}

	return QWidget::mouseDoubleClickEvent(event);
}

void XLTabBar::mousePressEvent(QMouseEvent *event)  {
	if (m_titleBar->hasMaxButton()) {
		// disable drag when it is maximized
		if (m_titleBar->m_maxButton->isVisible()) {
			m_titleBar->m_pressed = true;
			m_titleBar->m_startMovePos = event->globalPos();
		}
	} else {
		m_titleBar->m_pressed = true;
		m_titleBar->m_startMovePos = event->globalPos();
	}

	return QWidget::mousePressEvent(event);
}

void XLTabBar::mouseMoveEvent(QMouseEvent *event)  {
	if (m_titleBar->m_pressed) {
		QPoint movePoint = event->globalPos() - m_titleBar->m_startMovePos;
		QPoint widgetPos = m_titleBar->parentWidget()->pos();
		m_titleBar->m_startMovePos = event->globalPos();
		m_titleBar->parentWidget()->move(widgetPos.x() + movePoint.x(), widgetPos.y() + movePoint.y());
	}
	return QWidget::mouseMoveEvent(event);
}

void XLTabBar::mouseReleaseEvent(QMouseEvent *event)  {
	m_titleBar->m_pressed = false;
	return QWidget::mouseReleaseEvent(event);
}