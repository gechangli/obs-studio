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
#include "xl-source-app-item-widget.hpp"
#include "xl-source-popup-widget.hpp"
#include "window-basic-main.hpp"
#include "xl-add-window-dialog.hpp"
#include "xl-util.hpp"

XLSourceAppItemWidget::XLSourceAppItemWidget(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::XLAppItemWidget),
	m_index(0) {
	ui->setupUi(this);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-source-app-item-widget");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

XLSourceAppItemWidget::~XLSourceAppItemWidget() {
}

void XLSourceAppItemWidget::paintEvent(QPaintEvent* event) {
	// draw with style
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void XLSourceAppItemWidget::update() {
	// get window property
	obs_property_t* prop = getWindowProperty();

	// set name
	ui->nameLabel->setText(obs_property_list_item_name(prop, m_index));

	// get window id
	long long winId = obs_property_list_item_int(prop, m_index);
	QPixmap icon = XLUtil::getWindowIcon(winId);
	if(icon.isNull()) {
		ui->iconLabel->setPixmap(QPixmap(":/res/images/source_window.png"));
	} else {
		ui->iconLabel->setPixmap(icon);
	}
}

obs_property_t* XLSourceAppItemWidget::getWindowProperty() {
	QStandardItemModel* model = getModel();
	QStandardItem* item = model->item(m_index, 0);
	return (obs_property_t*)item->data().value<void*>();
}

QStandardItemModel* XLSourceAppItemWidget::getModel() {
	QListView* listView = dynamic_cast<QListView*>(parentWidget()->parentWidget());
	QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(listView->model());
	return model;
}

void XLSourceAppItemWidget::on_openButton_clicked() {
	// show properties window
	const char* id = "window_capture";
	OBSBasic* main = dynamic_cast<OBSBasic*>(App()->GetMainWindow());
	obs_source_t* source = main->addSourceById(id);
	XLAddSourceDialog* dialog = main->showPropertiesWindow(source, false, false);

	// preset window
	XLAddWindowDialog* md = dynamic_cast<XLAddWindowDialog*>(dialog);
	if(md) {
		md->presetWindow(m_index);
	}

	// start dialog
	dialog->exec();

	// release
	obs_source_release(source);
}

int XLSourceAppItemWidget::getIndex() {
	return m_index;
}

void XLSourceAppItemWidget::setIndex(int i) {
	m_index = i;
}