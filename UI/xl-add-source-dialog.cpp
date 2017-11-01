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
#include <QTextEdit>
#include "xl-add-source-dialog.hpp"
#include "qt-wrappers.hpp"
#include "xl-util.hpp"
#include "xl-title-bar-sub.hpp"
#include "xl-frameless-window-util.hpp"
#include "window-basic-main.hpp"
#include "xl-source-list-item-widget.hpp"

using namespace std;

XLAddSourceDialog::XLAddSourceDialog(QWidget *parent, obs_source_t* source) :
	QDialog (parent),
	m_source(OBSSource(source)),
	m_properties(Q_NULLPTR, obs_properties_destroy),
	m_rollback(false),
	m_editMode(false) {
}

XLAddSourceDialog::~XLAddSourceDialog() {
	obs_source_dec_showing(m_source);

	// if user click X and not in edit mode, we need rollback the change
	// so, get list view item widget and call remove
	if(m_rollback) {
		if(m_editMode) {
			obs_data_t *settings = obs_source_get_settings(m_source);
			obs_data_clear(settings);
			obs_data_release(settings);
			if(m_deferUpdate) {
				obs_data_apply(settings, m_oldSettings);
			} else {
				obs_source_update(m_source, m_oldSettings);
			}
		} else {
			OBSBasic *main = dynamic_cast<OBSBasic *>(App()->GetMainWindow());
			XLSourceListView *listView = main->getSourceList();
			QAbstractItemModel *model = listView->model();
			QModelIndex index = model->index(model->rowCount() - 1, 0);
			XLSourceListItemWidget *widget = dynamic_cast<XLSourceListItemWidget *>(listView->indexWidget(index));
			widget->remove();
		}
	}
}

void XLAddSourceDialog::init() {
	// subclass load ui
	loadUI();

	// setup frameless ui
	XLFramelessWindowUtil::setupUI(this);

	// create title bar
	m_titleBar = new XLTitleBarSub(this);
	m_titleBar->init();
	m_titleBar->move(0, 0);
	connect(m_titleBar, &XLTitleBar::windowRequestClose, this, &QDialog::reject);

	// make a copy of settings
	m_oldSettings = obs_data_create();
	m_settings = obs_source_get_settings(m_source);
	obs_data_apply(m_oldSettings, m_settings);
	obs_data_release(m_oldSettings);
	obs_data_release(m_settings);

	// add preview callback
	obs_source_inc_showing(m_source);
	auto addDrawCallback = [this]() {
		obs_display_add_draw_callback(getDisplay()->GetDisplay(), XLAddSourceDialog::drawPreview, this);
	};
	enum obs_source_type type = obs_source_get_type(m_source);
	uint32_t caps = obs_source_get_output_flags(m_source);
	bool drawable_type = type == OBS_SOURCE_TYPE_INPUT || type == OBS_SOURCE_TYPE_SCENE;
	if (drawable_type && (caps & OBS_SOURCE_VIDEO) != 0) {
		connect(getDisplay(), &OBSQTDisplay::DisplayCreated, addDrawCallback);
	}

	// init ui for properties
	loadProperties();
}

void XLAddSourceDialog::setEditMode(bool v) {
	m_editMode = v;
}

void XLAddSourceDialog::reject() {
	m_rollback = true;
	cleanup();
	QDialog::reject();
}

void XLAddSourceDialog::accept() {
	cleanup();
	QDialog::accept();
}

void XLAddSourceDialog::cleanup() {
	obs_display_remove_draw_callback(getDisplay()->GetDisplay(), XLAddSourceDialog::drawPreview, this);
}

obs_source_t* XLAddSourceDialog::getSource() {
	return m_source;
}

bool XLAddSourceDialog::onListPropertyChanged(obs_property_t* prop, QComboBox* combo, int index) {
	// get combo, data format and type
	const char* name = obs_property_name(prop);
	obs_combo_format format = obs_property_list_format(prop);
	obs_combo_type type = obs_property_list_type(prop);
	QVariant data;

	// get selected data
	if (type == OBS_COMBO_TYPE_EDITABLE) {
		data = combo->currentText().toUtf8();
	} else {
		if (index != -1)
			data = combo->itemData(index);
		else
			return true;
	}

	// save new value to settings
	switch (format) {
		case OBS_COMBO_FORMAT_INVALID:
			return true;
		case OBS_COMBO_FORMAT_INT:
			obs_data_set_int(m_settings, name, data.value<long long>());
			break;
		case OBS_COMBO_FORMAT_FLOAT:
			obs_data_set_double(m_settings, name, data.value<double>());
			break;
		case OBS_COMBO_FORMAT_STRING:
			obs_data_set_string(m_settings, name, data.toByteArray().constData());
			break;
	}

	// update
	if(!m_deferUpdate) {
		obs_source_update(m_source, m_settings);
	}

	// notify property changed
	return obs_property_modified(prop, m_settings);
}

void XLAddSourceDialog::setWindowTitle(const QString& title) {
	m_titleBar->setWindowTitle(title);
}

void XLAddSourceDialog::getScaleAndCenterPos(
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

void XLAddSourceDialog::drawPreview(void *data, uint32_t cx, uint32_t cy) {
	XLAddSourceDialog *window = static_cast<XLAddSourceDialog*>(data);
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

void XLAddSourceDialog::bindPropertyUI(obs_property_t* prop, QWidget* widget, QWidget* actionWidget, const char* slot) {
	// if has this property, bind UI
	if (prop) {
		// get property type
		obs_property_type type = obs_property_get_type(prop);
		switch(type) {
			case OBS_PROPERTY_LIST:
				bindListPropertyUI(prop, dynamic_cast<QComboBox *>(widget), slot);
				break;
			case OBS_PROPERTY_FONT:
				bindFontPropertyUI(prop, dynamic_cast<QLabel*>(widget), dynamic_cast<QPushButton*>(actionWidget), slot);
				break;
			case OBS_PROPERTY_TEXT: {
				obs_text_type textType = obs_proprety_text_type(prop);
				switch(textType) {
					case OBS_TEXT_MULTILINE:
						bindMultilineTextPropertyUI(prop, dynamic_cast<QTextEdit*>(widget), slot);
						break;
				}
				break;
			}
			default:
				break;
		}
	}
}

void XLAddSourceDialog::populateListProperty(obs_property_t* prop, QComboBox* combo, const char *slot) {
	// clear list first
	QStandardItemModel *model = dynamic_cast<QStandardItemModel*>(combo->model());
	model->clear();

	// add items
	obs_combo_type type = obs_property_list_type(prop);
	obs_combo_format format = obs_property_list_format(prop);
	size_t count = obs_property_list_item_count(prop);
	for (size_t i = 0; i < count; i++) {
		addComboItem(combo, prop, format, i);
	}

	// set editable or not
	combo->setEditable(type == OBS_COMBO_TYPE_EDITABLE);

	// max visible items and tooltip
	combo->setMaxVisibleItems(40);
	combo->setToolTip(QT_UTF8(obs_property_long_description(prop)));

	// get current property value
	// if string value, set to combo box
	// if a list value, find value index and set as current
	int idx = -1;
	const char* name = obs_property_name(prop);
	QString value = XLUtil::getData(m_settings, name, format);
	if (format == OBS_COMBO_FORMAT_STRING &&
		type == OBS_COMBO_TYPE_EDITABLE) {
		combo->lineEdit()->setText(value);
	} else {
		idx = combo->findData(QByteArray(value.toStdString().c_str()));
		if (idx != -1) {
			combo->setCurrentIndex(idx);
		}
	}

	// trigger change event if index is -1
	if(idx == -1 && slot) {
		QMetaObject::invokeMethod(this, slot, Q_ARG(int, 0));
	}
}

void XLAddSourceDialog::bindListPropertyUI(obs_property_t *prop, QComboBox *combo, const char *slot) {
	// populate list items
	populateListProperty(prop, combo, slot);

	// connect event
	connect(combo, SIGNAL(currentIndexChanged(int)), this, slot);
}

void XLAddSourceDialog::bindFontPropertyUI(obs_property_t* prop, QLabel* fontNameLabel, QPushButton* selectFontButton, const char* slot) {
	// get font settings
	const char* name = obs_property_name(prop);
	obs_data_t* font_obj = obs_data_get_obj(m_settings, name);
	const char* face = obs_data_get_string(font_obj, "face");
	const char* style = obs_data_get_string(font_obj, "style");

	// if font property is not enabled, disable widget
	if (!obs_property_enabled(prop)) {
		selectFontButton->setEnabled(false);
		fontNameLabel->setEnabled(false);
	}

	// create font based on current settings
	QFont font = fontNameLabel->font();
	makeQFont(font_obj, font, true);

	// set tooltip
	selectFontButton->setToolTip(QT_UTF8(obs_property_long_description(prop)));

	// set font name
	fontNameLabel->setFont(font);
	fontNameLabel->setText(QString("%1 %2").arg(face, style));
	fontNameLabel->setToolTip(QT_UTF8(obs_property_long_description(prop)));

	// event
	connect(selectFontButton, SIGNAL(clicked()), this, slot);

	// release
	obs_data_release(font_obj);
}

void XLAddSourceDialog::bindMultilineTextPropertyUI(obs_property_t* prop, QTextEdit* textEdit, const char* slot) {

}

void XLAddSourceDialog::makeQFont(obs_data_t *font_obj, QFont &font, bool limit) {
	// get font settings
	const char *face  = obs_data_get_string(font_obj, "face");
	const char *style = obs_data_get_string(font_obj, "style");
	int size = (int)obs_data_get_int(font_obj, "size");
	uint32_t flags = (uint32_t)obs_data_get_int(font_obj, "flags");

	// set face
	if (face) {
		font.setFamily(face);
		font.setStyleName(style);
	}

	// set size
	if (size) {
		if (limit) {
			int max_size = font.pointSize();
			if (max_size < 28) max_size = 28;
			if (size > max_size) size = max_size;
		}
		font.setPointSize(size);
	}

	// set style
	font.setBold(flags & OBS_FONT_BOLD);
	font.setItalic(flags & OBS_FONT_ITALIC);
	font.setUnderline(flags & OBS_FONT_UNDERLINE);
	font.setStrikeOut(flags & OBS_FONT_STRIKEOUT);
}

void XLAddSourceDialog::addComboItem(QComboBox *combo, obs_property_t *prop, obs_combo_format format, size_t idx) {
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