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
#include "xl-source-list-item-widget.hpp"
#include "xl-source-list-view.hpp"
#include "window-basic-main.hpp"
#include "xl-util.hpp"

Q_DECLARE_METATYPE(OBSSceneItem);

XLSourceListItemWidget::XLSourceListItemWidget(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::XLSourceListItemWidget),
	m_index(0) {
	// init ui
	ui->setupUi(this);
}

XLSourceListItemWidget::~XLSourceListItemWidget() {
}

void XLSourceListItemWidget::paintEvent(QPaintEvent* event) {
	// draw with style
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void XLSourceListItemWidget::update() {
	// get source
	obs_sceneitem_t* sceneItem = getSceneItem();
	obs_source_t* source = obs_sceneitem_get_source(sceneItem);

	// update name
	const char* name = obs_source_get_name(source);
	ui->nameLabel->setText(name);

	// visible
	ui->visibilityLabel->setPixmap(QPixmap(obs_sceneitem_visible(sceneItem) ? ":/res/images/visible.png" : ":/res/images/invisible.png"));

	// update icon
	if(obs_source_get_type(source) == OBS_SOURCE_TYPE_INPUT) {
		const char* id = obs_source_get_id(source);
		ui->iconLabel->setPixmap(XLUtil::getSourceIcon(id));
	} else {
		// should not go here
	}
}

obs_sceneitem_t* XLSourceListItemWidget::getSceneItem() {
	QStandardItemModel* model = getModel();
	QStandardItem* item = model->item(m_index, 0);
	return item->data(static_cast<int>(QtDataRole::OBSRef)).value<OBSSceneItem>();
}

obs_source_t* XLSourceListItemWidget::getSource() {
	obs_sceneitem_t* sceneItem = getSceneItem();
	return obs_sceneitem_get_source(sceneItem);
}

void XLSourceListItemWidget::on_visibilityLabel_clicked() {
	// get scene item and source
	obs_sceneitem_t* sceneItem = getSceneItem();
	obs_source_t* source = obs_sceneitem_get_source(sceneItem);

	// toggle visibility
	bool visible = !obs_sceneitem_visible(sceneItem);
	obs_sceneitem_set_visible(sceneItem, visible);

	// update ui
	ui->visibilityLabel->setPixmap(QPixmap(visible ? ":/res/images/visible.png" : ":/res/images/invisible.png"));
}

QStandardItemModel* XLSourceListItemWidget::getModel() {
	XLSourceListView* listView = dynamic_cast<XLSourceListView*>(parentWidget()->parentWidget());
	QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(listView->model());
	return model;
}

void XLSourceListItemWidget::on_deleteLabel_clicked() {
	remove();
}

void XLSourceListItemWidget::remove() {
	QStandardItemModel* model = getModel();
	obs_sceneitem_t* sceneItem = getSceneItem();
	obs_sceneitem_remove(sceneItem);
	model->removeRow(m_index);
}

void XLSourceListItemWidget::on_editLabel_clicked() {
	obs_source_t* source = getSource();
	OBSBasic* main = dynamic_cast<OBSBasic*>(App()->GetMainWindow());
	main->showPropertiesWindow(source, true);
}

int XLSourceListItemWidget::getIndex() {
	return m_index;
}

void XLSourceListItemWidget::setIndex(int i) {
	m_index = i;
}