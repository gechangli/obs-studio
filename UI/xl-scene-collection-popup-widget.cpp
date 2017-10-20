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
#include "xl-scene-collection-popup-widget.hpp"
#include "window-basic-main.hpp"
#include "xl-util.hpp"

XLSceneCollectionPopupWidget::XLSceneCollectionPopupWidget(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::XLSceneCollectionPopupWidget),
	m_refLocWidget(Q_NULLPTR) {
	// init ui
	ui->setupUi(this);

	// make self transparent
	setAttribute(Qt::WA_TranslucentBackground);

	// event
	OBSBasic* main = static_cast<OBSBasic*>(App()->GetMainWindow());

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-scene-collection-popup-widget");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

XLSceneCollectionPopupWidget::~XLSceneCollectionPopupWidget() {
}

XLSceneCollectionPopupWidget* XLSceneCollectionPopupWidget::instance() {
	static XLSceneCollectionPopupWidget* inst = Q_NULLPTR;
	if (inst == Q_NULLPTR) {
		inst = new XLSceneCollectionPopupWidget();
	}
	return inst;
}

void XLSceneCollectionPopupWidget::showEvent(QShowEvent *event) {
	// call super
	QWidget::showEvent(event);

	// popup at bottom center of parent
	QWidget* ref = m_refLocWidget ? m_refLocWidget : parentWidget();
	QSize refSize = ref->size();
	QSize selfSize = size();
	QPoint pos(refSize.width() - selfSize.width(), refSize.height());
	pos = ref->mapToGlobal(pos);
	move(pos);
}

void XLSceneCollectionPopupWidget::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);

	// draw with style
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void XLSceneCollectionPopupWidget::setReferenceWidget(QWidget* w) {
	m_refLocWidget = w;
}