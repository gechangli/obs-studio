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
#include <QCloseEvent>
#include <QFontDialog>
#include "xl-add-text-dialog.hpp"
#include "qt-wrappers.hpp"
#include "xl-util.hpp"
#include "window-basic-main.hpp"

using namespace std;

XLAddTextDialog::XLAddTextDialog(QWidget *parent, obs_source_t* source) :
	XLAddSourceDialog (parent, source),
	ui(new Ui::XLAddTextDialog) {
}

XLAddTextDialog::~XLAddTextDialog() {
}

void XLAddTextDialog::on_yesButton_clicked() {
	accept();
}

void XLAddTextDialog::on_noButton_clicked() {
	reject();
}

void XLAddTextDialog::loadUI() {
	// init ui
	ui->setupUi(this);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-add-text-dialog");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

OBSQTDisplay* XLAddTextDialog::getDisplay() {
	return ui->preview;
}

void XLAddTextDialog::loadProperties() {
	// load properties from source
	m_properties.reset(obs_source_properties(m_source));

	// find properties we want to set
#ifdef Q_OS_OSX
	const char* id1 = "font";
#else
	const char* id1 = "font";
#endif
#ifdef Q_OS_OSX
	const char* id2 = "preset";
#else
	const char* id2 = "resolution";
#endif

	// check defer update flag
	uint32_t flags = obs_properties_get_flags(m_properties.get());
	m_deferUpdate = (flags & OBS_PROPERTIES_DEFER_UPDATE) != 0;

	// property
	m_fontProperty = obs_properties_get(m_properties.get(), id1);

	// bind ui
	bindPropertyUI(m_fontProperty, ui->fontNameLabel, ui->selectFontButton, SLOT(onSelectFont()));
}

void XLAddTextDialog::onSelectFont() {
	// get font settings
	const char* name = obs_property_name(m_fontProperty);
	obs_data_t* font_obj = obs_data_get_obj(m_settings, name);

	// open font dialog to select font
	bool success;
	QFont font;
	if (!font_obj) {
		font = QFontDialog::getFont(&success, this);
	} else {
		makeQFont(font_obj, font);
		font = QFontDialog::getFont(&success, font, this);
		obs_data_release(font_obj);
	}

	// if failed, do nothing
	if (!success) {
		return;
	}

	// create font setting for selected font
	font_obj = obs_data_create();
	obs_data_set_string(font_obj, "face", QT_TO_UTF8(font.family()));
	obs_data_set_string(font_obj, "style", QT_TO_UTF8(font.styleName()));
	obs_data_set_int(font_obj, "size", font.pointSize());
	uint32_t flags = font.bold() ? OBS_FONT_BOLD : 0;
	flags |= font.italic() ? OBS_FONT_ITALIC : 0;
	flags |= font.underline() ? OBS_FONT_UNDERLINE : 0;
	flags |= font.strikeOut() ? OBS_FONT_STRIKEOUT : 0;
	obs_data_set_int(font_obj, "flags", flags);

	// update font name label
	QFont labelFont;
	makeQFont(font_obj, labelFont, true);
	ui->fontNameLabel->setFont(labelFont);
	ui->fontNameLabel->setText(QString("%1 %2").arg(font.family(), font.styleName()));

	// write font settings back
	obs_data_set_obj(m_settings, name, font_obj);
	obs_data_release(font_obj);
}