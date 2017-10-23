/******************************************************************************
    Copyright (C) 2013 by Hugh Bailey <obs.jim@gmail.com>

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

#include <QStandardItemModel>
#include "xl-add-camera-dialog.hpp"
#include "qt-wrappers.hpp"
#include "xl-util.hpp"
#include "xl-title-bar-sub.hpp"
#include "xl-frameless-window-util.hpp"
#include "window-basic-main.hpp"

using namespace std;

XLAddCameraDialog::XLAddCameraDialog(QWidget *parent, obs_source_t* source) :
	QDialog (parent),
	ui(new Ui::XLAddCameraDialog),
	m_source(OBSSource(source)),
	m_oldSettings(obs_data_create()),
	m_properties(Q_NULLPTR, obs_properties_destroy),
	m_rollback(false) {
	// init ui
	ui->setupUi(this);

	// setup frameless ui
	XLFramelessWindowUtil::setupUI(this);

	// create title bar
	m_titleBar = new XLTitleBarSub(this);
	m_titleBar->init();
	m_titleBar->move(0, 0);
	connect(m_titleBar, &XLTitleBar::windowRequestClose, this, &QDialog::reject);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-add-camera-dialog");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);

	// The OBSData constructor increments the reference once so we need release here
	// then copy default settings from source
	obs_data_release(m_oldSettings);
	OBSData settings = obs_source_get_settings(source);
	obs_data_apply(m_oldSettings, settings);
	obs_data_release(settings);

	// add preview callback
	obs_source_inc_showing(m_source);
	auto addDrawCallback = [this]() {
		obs_display_add_draw_callback(ui->preview->GetDisplay(), XLAddCameraDialog::drawPreview, this);
	};
	enum obs_source_type type = obs_source_get_type(m_source);
	uint32_t caps = obs_source_get_output_flags(m_source);
	bool drawable_type = type == OBS_SOURCE_TYPE_INPUT || type == OBS_SOURCE_TYPE_SCENE;
	if (drawable_type && (caps & OBS_SOURCE_VIDEO) != 0) {
		connect(ui->preview, &OBSQTDisplay::DisplayCreated, addDrawCallback);
	}

	// init ui for properties
	reloadProperties();
}

XLAddCameraDialog::~XLAddCameraDialog() {
	obs_source_dec_showing(m_source);
	if(m_rollback) {

	}
}

void XLAddCameraDialog::reject() {
	m_rollback = true;
	QDialog::reject();
}

obs_source_t* XLAddCameraDialog::getSource() {
	return m_source;
}

void XLAddCameraDialog::on_yesButton_clicked() {
	accept();
}

void XLAddCameraDialog::setWindowTitle(const QString& title) {
	m_titleBar->setWindowTitle(title);
}

void XLAddCameraDialog::getScaleAndCenterPos(
	int baseCX, int baseCY, int windowCX, int windowCY,
	int &x, int &y, float &scale) {
	double windowAspect, baseAspect;
	int newCX, newCY;

	windowAspect = double(windowCX) / double(windowCY);
	baseAspect   = double(baseCX)   / double(baseCY);

	if (windowAspect > baseAspect) {
		scale = float(windowCY) / float(baseCY);
		newCX = int(double(windowCY) * baseAspect);
		newCY = windowCY;
	} else {
		scale = float(windowCX) / float(baseCX);
		newCX = windowCX;
		newCY = int(float(windowCX) / baseAspect);
	}

	x = windowCX/2 - newCX/2;
	y = windowCY/2 - newCY/2;
}

void XLAddCameraDialog::drawPreview(void *data, uint32_t cx, uint32_t cy) {
	XLAddCameraDialog *window = static_cast<XLAddCameraDialog*>(data);
	if (!window->getSource())
		return;

	uint32_t sourceCX = max(obs_source_get_width(window->getSource()), 1u);
	uint32_t sourceCY = max(obs_source_get_height(window->getSource()), 1u);

	int   x, y;
	int   newCX, newCY;
	float scale;

	getScaleAndCenterPos(sourceCX, sourceCY, cx, cy, x, y, scale);

	newCX = int(scale * float(sourceCX));
	newCY = int(scale * float(sourceCY));

	gs_viewport_push();
	gs_projection_push();
	gs_ortho(0.0f, float(sourceCX), 0.0f, float(sourceCY),
			 -100.0f, 100.0f);
	gs_set_viewport(x, y, newCX, newCY);

	obs_source_video_render(window->getSource());

	gs_projection_pop();
	gs_viewport_pop();
}

void XLAddCameraDialog::reloadProperties() {
	// load properties from source
	m_properties.reset(obs_source_properties(m_source));

	// check defer update flag
	uint32_t flags = obs_properties_get_flags(m_properties.get());
	m_deferUpdate = (flags & OBS_PROPERTIES_DEFER_UPDATE) != 0;

	// init ui
	reloadPropertiesUI();
}

void XLAddCameraDialog::reloadPropertiesUI() {
	// fill device combo
	obs_property_t *property = obs_properties_get(m_properties.get(), "device");
	if (property) {
		// add items
		obs_combo_type type   = obs_property_list_type(property);
		obs_combo_format format = obs_property_list_format(property);
		size_t count  = obs_property_list_item_count(property);
		for (size_t i = 0; i < count; i++) {
			addComboItem(ui->cameraComboBox, property, format, i);
		}

		// set editable or not
		if (type == OBS_COMBO_TYPE_EDITABLE) {
			ui->cameraComboBox->setEditable(true);
		}

		// max visible items and tooltip
		ui->cameraComboBox->setMaxVisibleItems(40);
		ui->cameraComboBox->setToolTip(QT_UTF8(obs_property_long_description(property)));
	}
}

void XLAddCameraDialog::addComboItem(QComboBox *combo, obs_property_t *prop, obs_combo_format format, size_t idx) {
	// get property name and value
	const char *name = obs_property_list_item_name(prop, idx);
	QVariant var;
	if (format == OBS_COMBO_FORMAT_INT) {
		long long val = obs_property_list_item_int(prop, idx);
		var = QVariant::fromValue<long long>(val);
	} else if (format == OBS_COMBO_FORMAT_FLOAT) {
		double val = obs_property_list_item_float(prop, idx);
		var = QVariant::fromValue<double>(val);
	} else if (format == OBS_COMBO_FORMAT_STRING) {
		var = QByteArray(obs_property_list_item_string(prop, idx));
	}

	// add item to combo
	combo->addItem(QT_UTF8(name), var);

	// if item is disabled, set it
	if (obs_property_list_item_disabled(prop, idx)) {
		int index = combo->findText(QT_UTF8(name));
		if (index < 0) {
			return;
		}
		QStandardItemModel *model = dynamic_cast<QStandardItemModel*>(combo->model());
		if (!model) {
			return;
		}
		QStandardItem *item = model->item(index);
		item->setFlags(Qt::NoItemFlags);
	}
}