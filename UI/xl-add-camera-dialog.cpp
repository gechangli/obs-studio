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
#include <QLineEdit>
#include <QComboBox>
#include "xl-add-camera-dialog.hpp"
#include "qt-wrappers.hpp"
#include "xl-util.hpp"
#include "xl-title-bar-sub.hpp"
#include "xl-frameless-window-util.hpp"
#include "window-basic-main.hpp"

using namespace std;

template <long long get_int(obs_data_t*, const char*),
	double get_double(obs_data_t*, const char*),
	const char *get_string(obs_data_t*, const char*)>
static string from_obs_data(obs_data_t *data, const char *name, obs_combo_format format) {
	switch (format) {
		case OBS_COMBO_FORMAT_INT:
			return to_string(get_int(data, name));
		case OBS_COMBO_FORMAT_FLOAT:
			return to_string(get_double(data, name));
		case OBS_COMBO_FORMAT_STRING:
			return get_string(data, name);
		default:
			return "";
	}
}

static string from_obs_data(obs_data_t *data, const char *name, obs_combo_format format) {
	return from_obs_data<obs_data_get_int, obs_data_get_double, obs_data_get_string>(data, name, format);
}

XLAddCameraDialog::XLAddCameraDialog(QWidget *parent, obs_source_t* source) :
	QDialog (parent),
	ui(new Ui::XLAddCameraDialog),
	m_source(OBSSource(source)),
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

	// make a copy of settings
	m_oldSettings = obs_data_create();
	m_settings = obs_source_get_settings(source);
	obs_data_apply(m_oldSettings, m_settings);
	obs_data_release(m_oldSettings);
	obs_data_release(m_settings);

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
	loadProperties();
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

void XLAddCameraDialog::onDeviceChanged(int index) {

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

void XLAddCameraDialog::loadProperties() {
	// load properties from source
	m_properties.reset(obs_source_properties(m_source));

	// find properties we want to set
	m_deviceProperty = obs_properties_get(m_properties.get(), "device");

	// check defer update flag
	uint32_t flags = obs_properties_get_flags(m_properties.get());
	m_deferUpdate = (flags & OBS_PROPERTIES_DEFER_UPDATE) != 0;

	// bind ui
	bindPropertyUI(m_deviceProperty, ui->cameraComboBox);
}

void XLAddCameraDialog::bindPropertyUI(obs_property_t* prop, QWidget* widget) {
	// null checking
	if (!prop) {
		return;
	}

	// get property type
	obs_property_type type = obs_property_get_type(prop);
	switch(type) {
		case OBS_PROPERTY_LIST:
			bindComboBoxPropertyUI(prop, dynamic_cast<QComboBox*>(widget));
			break;
	}
}

void XLAddCameraDialog::bindComboBoxPropertyUI(obs_property_t* prop, QComboBox* combo) {
	// add items
	const char* name = obs_property_name(prop);
	obs_combo_type type = obs_property_list_type(prop);
	obs_combo_format format = obs_property_list_format(prop);
	size_t count = obs_property_list_item_count(prop);
	for (size_t i = 0; i < count; i++) {
		addComboItem(combo, prop, format, i);
	}

	// set editable or not
	if (type == OBS_COMBO_TYPE_EDITABLE) {
		combo->setEditable(true);
	}

	// max visible items and tooltip
	combo->setMaxVisibleItems(40);
	combo->setToolTip(QT_UTF8(obs_property_long_description(prop)));

	// get current property value
	// if string value, set to combo box
	// if a list value, find value index and set as current
	int idx = -1;
	string value = from_obs_data(m_settings, name, format);
	if (format == OBS_COMBO_FORMAT_STRING &&
		type == OBS_COMBO_TYPE_EDITABLE) {
		combo->lineEdit()->setText(QT_UTF8(value.c_str()));
	} else {
		idx = combo->findData(QByteArray(value.c_str()));
		if (idx != -1) {
			combo->setCurrentIndex(idx);
		}
	}

	// connect event
	connect(combo, &QComboBox::currentIndexChanged, this, &XLAddCameraDialog::onDeviceChanged);

	// trigger device changed if current value is not found
	if(idx == -1) {
		onDeviceChanged(-1);
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