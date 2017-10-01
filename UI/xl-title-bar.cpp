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

#include "xl-title-bar.hpp"
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

#define BUTTON_HEIGHT 30
#define BUTTON_WIDTH 30
#define TITLE_HEIGHT 36

XLTitleBar::XLTitleBar(QWidget *parent) :
	QWidget(parent),
	m_windowBorderWidth(0) {
	// create controls
	m_icon = new QLabel;
	m_titleLabel = new QLabel;
	m_minButton = new QPushButton;
	m_restoreButton = new QPushButton;
	m_maxButton = new QPushButton;
	m_closeButton = new QPushButton;

	// set button size
	m_minButton->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));
	m_restoreButton->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));
	m_maxButton->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));
	m_closeButton->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));

	// set widget name
	m_titleLabel->setObjectName("titleLabel");
	m_minButton->setObjectName("minButton");
	m_restoreButton->setObjectName("restoreButton");
	m_maxButton->setObjectName("maxButton");
	m_closeButton->setObjectName("closeButton");

	// add widget to a horizontal layout
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addWidget(m_icon);
	layout->addWidget(m_titleLabel);
	layout->addWidget(m_minButton);
	layout->addWidget(m_restoreButton);
	layout->addWidget(m_maxButton);
	layout->addWidget(m_closeButton);
	layout->setContentsMargins(5, 0, 0, 0);
	layout->setSpacing(0);
	m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setFixedHeight(TITLE_HEIGHT);
	setWindowFlags(Qt::FramelessWindowHint);
}

XLTitleBar::~XLTitleBar() {

}

void XLTitleBar::paintEvent(QPaintEvent *event) {
	// draw background
	QPainter painter(this);
	QPainterPath pathBack;
	pathBack.setFillRule(Qt::WindingFill);
	pathBack.addRoundedRect(QRect(0, 0, width(), height()), 3, 3);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	painter.fillPath(pathBack, QBrush(QColor::fromRgb(49, 50, 65)));

	// adjust width
	if (width() != (parentWidget()->width() - m_windowBorderWidth)) {
		setFixedWidth(parentWidget()->width() - m_windowBorderWidth);
	}

	// super
	QWidget::paintEvent(event);
}