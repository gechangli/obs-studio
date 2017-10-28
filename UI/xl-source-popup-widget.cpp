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
#include "xl-source-popup-widget.hpp"
#include "window-basic-main.hpp"
#include "xl-util.hpp"

XLSourcePopupWidget::XLSourcePopupWidget(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::XLSourcePopupWidget),
	m_refLocWidget(Q_NULLPTR),
	m_mode(MODE_APP) {
	// init ui
	ui->setupUi(this);

	// make self transparent
	setAttribute(Qt::WA_TranslucentBackground);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-source-popup-widget");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

XLSourcePopupWidget::~XLSourcePopupWidget() {
}

void XLSourcePopupWidget::setMode(XLSourcePopupWidget::Mode m) {
	m_mode = m;
}

void XLSourcePopupWidget::hideEvent(QHideEvent *event) {
	// call super
	QWidget::hideEvent(event);

	// delete self
	deleteLater();
}

void XLSourcePopupWidget::showEvent(QShowEvent *event) {
	// call super
	QWidget::showEvent(event);

	// center of ref
	QWidget* ref = m_refLocWidget ? m_refLocWidget : parentWidget();
	QSize refSize = ref->size();
	QPoint refCenter(refSize.width() / 2, refSize.height());
	refCenter = ref->mapToGlobal(refCenter);

	// start x
	OBSBasic* main = dynamic_cast<OBSBasic*>(App()->GetMainWindow());
	QWidget* scenePanel = main->getScenePanel();
	QSize selfSize = size();
	int x = (scenePanel->size().width() - selfSize.width()) / 2;
	x = scenePanel->mapToGlobal(QPoint(x, 0)).x();

	// left margin of arrow
	QSize arrowSize = ui->arrowLabel->size();
	QMargins m = ui->arrowContainer->contentsMargins();
	m.setLeft(refCenter.x() - x - arrowSize.width() / 2);
	ui->arrowContainer->setContentsMargins(m);

	// move
	move(QPoint(x, refCenter.y()));
}

void XLSourcePopupWidget::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);

	// draw with style
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void XLSourcePopupWidget::setReferenceWidget(QWidget* w) {
	m_refLocWidget = w;
}