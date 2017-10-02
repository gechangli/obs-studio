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
	m_windowBorderWidth(0),
	m_pressed(false) {
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

	// set button ui
	m_minButton->setFlat(true);
	m_restoreButton->setFlat(true);
	m_maxButton->setFlat(true);
	m_closeButton->setFlat(true);
	m_minButton->setIcon(QIcon(QPixmap(QString(":/res/images/minimize.png"))));
	m_restoreButton->setIcon(QIcon(QPixmap(QString(":/res/images/restore.png"))));
	m_maxButton->setIcon(QIcon(QPixmap(QString(":/res/images/maximize.png"))));
	m_closeButton->setIcon(QIcon(QPixmap(QString(":/res/images/close.png"))));

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
	layout->setContentsMargins(5, 0, 5, 0);
	layout->setSpacing(0);
	m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setFixedHeight(TITLE_HEIGHT);
	setWindowFlags(Qt::FramelessWindowHint);

	// connect slots
	connect(m_minButton, &QPushButton::clicked, this, &XLTitleBar::onMinClicked);
	connect(m_restoreButton, &QPushButton::clicked, this, &XLTitleBar::onRestoreClicked);
	connect(m_maxButton, &QPushButton::clicked, this, &XLTitleBar::onMaxClicked);
	connect(m_closeButton, &QPushButton::clicked, this, &XLTitleBar::onCloseClicked);

	// button visibility
	m_maxButton->setVisible(!parent->isMaximized() && hasMaxButton());
	m_restoreButton->setVisible(parent->isMaximized() && hasMaxButton());
	m_minButton->setVisible(hasMinButton());
}

XLTitleBar::~XLTitleBar() {
}

bool XLTitleBar::hasMaxButton() {
	Qt::WindowFlags flags = parentWidget()->windowFlags();
	return (flags & Qt::WindowMaximizeButtonHint) != 0;
}

bool XLTitleBar::hasMinButton() {
	Qt::WindowFlags flags = parentWidget()->windowFlags();
	return (flags & Qt::WindowMinimizeButtonHint) != 0;
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

void XLTitleBar::mouseDoubleClickEvent(QMouseEvent *event) {
	if (hasMaxButton()) {
		if (m_maxButton->isVisible()) {
			onMaxClicked();
		} else {
			onRestoreClicked();
		}
	}

	return QWidget::mouseDoubleClickEvent(event);
}

void XLTitleBar::mousePressEvent(QMouseEvent *event)  {
	if (hasMaxButton()) {
		// disable drag when it is maximized
		if (m_maxButton->isVisible()) {
			m_pressed = true;
			m_startMovePos = event->globalPos();
		}
	} else {
		m_pressed = true;
		m_startMovePos = event->globalPos();
	}

	return QWidget::mousePressEvent(event);
}

void XLTitleBar::mouseMoveEvent(QMouseEvent *event)  {
	if (m_pressed) {
		QPoint movePoint = event->globalPos() - m_startMovePos;
		QPoint widgetPos = this->parentWidget()->pos();
		m_startMovePos = event->globalPos();
		parentWidget()->move(widgetPos.x() + movePoint.x(), widgetPos.y() + movePoint.y());
	}
	return QWidget::mouseMoveEvent(event);
}

void XLTitleBar::mouseReleaseEvent(QMouseEvent *event)  {
	m_pressed = false;
	return QWidget::mouseReleaseEvent(event);
}

void XLTitleBar::onMinClicked() {
	emit windowRequestMinimize();
}

void XLTitleBar::onRestoreClicked() {
	m_restoreButton->setVisible(false);
	m_maxButton->setVisible(true);
	emit windowRequestRestore();
}

void XLTitleBar::onMaxClicked() {
	m_maxButton->setVisible(false);
	m_restoreButton->setVisible(true);
	emit windowRequestMaximize();
}

void XLTitleBar::onCloseClicked() {
	emit windowRequestClose();
}

void XLTitleBar::saveRestoreInfo(const QPoint point, const QSize size)  {
	m_restorePos = point;
	m_restoreSize = size;
}

void XLTitleBar::getRestoreInfo(QPoint& point, QSize& size)  {
	point = m_restorePos;
	size = m_restoreSize;
}

void XLTitleBar::setWindowTitle(QString title) {
	m_titleLabel->setText(title);
}