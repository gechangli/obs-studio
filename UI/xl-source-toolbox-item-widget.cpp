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
#include <QStandardItemModel>
#include <QPaintEvent>
#include "xl-source-toolbox-item-widget.hpp"
#include "window-basic-main.hpp"
#include "xl-util.hpp"
#include "obs.h"

XLSourceToolboxItemWidget::XLSourceToolboxItemWidget(QWidget* parent, XLSourcePopupWidget::Mode mode) :
	QWidget(parent),
	ui(new Ui::XLToolboxItemWidget),
	m_index(0),
	m_mode(mode) {
	// init ui
	ui->setupUi(this);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-source-toolbox-item-widget");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

XLSourceToolboxItemWidget::~XLSourceToolboxItemWidget() {
}

void XLSourceToolboxItemWidget::paintEvent(QPaintEvent* event) {
	// draw with style
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void XLSourceToolboxItemWidget::update() {
}

QStandardItemModel* XLSourceToolboxItemWidget::getModel() {
	XLSourceListView* listView = dynamic_cast<XLSourceListView*>(parentWidget()->parentWidget());
	QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(listView->model());
	return model;
}

void XLSourceToolboxItemWidget::on_openButton_clicked() {
	switch(m_mode) {
		case XLSourcePopupWidget::MODE_TOOLBOX:
			const char* id;
			switch(m_index) {
				case 0:
#ifdef Q_OS_OSX
					id = "text_ft2_source";
#else
					id = "text_gdiplus";
#endif
					break;
				case 1:
					id = "image_source";
					break;
			}

			// show properties window
			OBSBasic* main = dynamic_cast<OBSBasic*>(App()->GetMainWindow());
			obs_source_t* source = main->addSourceById(id);
			main->showPropertiesWindow(source, false);
			obs_source_release(source);
			break;
	}
}

int XLSourceToolboxItemWidget::getIndex() {
	return m_index;
}

void XLSourceToolboxItemWidget::setIndex(int i) {
	m_index = i;
}

void XLSourceToolboxItemWidget::setIcon(QPixmap icon) {
	ui->iconLabel->setPixmap(icon);
}

void XLSourceToolboxItemWidget::setName(QString name) {
	ui->nameLabel->setText(name);
}

void XLSourceToolboxItemWidget::setDesc(QString desc) {
	ui->descLabel->setText(desc);
}