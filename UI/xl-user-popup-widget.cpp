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

#include <QPainter>
#include "xl-user-popup-widget.hpp"
#include "window-basic-main.hpp"
#include "xl-util.hpp"

XLUserPopupWidget::XLUserPopupWidget(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::XLUserPopupWidget),
	m_refLocWidget(Q_NULLPTR) {
	// init ui
	ui->setupUi(this);

	// make self transparent
	setAttribute(Qt::WA_TranslucentBackground);

	// event
	OBSBasic* main = static_cast<OBSBasic*>(App()->GetMainWindow());
	connect(ui->exitButton, &QPushButton::clicked, main, &OBSBasic::close);
	connect(ui->settingsButton, &QPushButton::clicked, main, &OBSBasic::on_profileButton_clicked);
	connect(ui->switchUserButton, &QPushButton::clicked, main, &OBSBasic::logout);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-user-popup-widget");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

XLUserPopupWidget::~XLUserPopupWidget() {
}

XLUserPopupWidget* XLUserPopupWidget::instance() {
	static XLUserPopupWidget* inst = Q_NULLPTR;
	if (inst == Q_NULLPTR) {
		inst = new XLUserPopupWidget();
	}
	return inst;
}

void XLUserPopupWidget::showEvent(QShowEvent *event) {
	// call super
	QWidget::showEvent(event);

	// popup at bottom center of parent
	QWidget* ref = m_refLocWidget ? m_refLocWidget : parentWidget();
	QSize refSize = ref->size();
	QSize selfSize = size();
	QPoint pos(refSize.width() / 2, refSize.height());
	pos = ref->mapToGlobal(pos);
	pos.setX(pos.x() - selfSize.width() / 2);
	move(pos);
}

void XLUserPopupWidget::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);

	// draw with style
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void XLUserPopupWidget::setReferenceWidget(QWidget* w) {
	m_refLocWidget = w;
}