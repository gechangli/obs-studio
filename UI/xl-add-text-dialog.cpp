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
	const char* id2 = "text";
#else
	const char* id2 = "text";
#endif
#ifdef Q_OS_OSX
	const char* id3 = "color1";
#else
	const char* id3 = "color1";
#endif
#ifdef Q_OS_OSX
	const char* id4 = "color2";
#else
	const char* id4 = "color2";
#endif

	// check defer update flag
	uint32_t flags = obs_properties_get_flags(m_properties.get());
	m_deferUpdate = (flags & OBS_PROPERTIES_DEFER_UPDATE) != 0;

	// property
	m_fontProperty = obs_properties_get(m_properties.get(), id1);
	m_textProperty = obs_properties_get(m_properties.get(), id2);
	m_color1Property = obs_properties_get(m_properties.get(), id3);
	m_color2Property = obs_properties_get(m_properties.get(), id4);

	// bind ui
	bindPropertyUI(m_fontProperty, ui->fontNameLabel, ui->selectFontButton, SLOT(onSelectFont()));
	bindPropertyUI(m_textProperty, ui->textEdit, Q_NULLPTR, SLOT(onTextChanged()));
	bindPropertyUI(m_color1Property, ui->customizeButton, Q_NULLPTR, SLOT(onColorChanged()));

	// custom init
	initOtherUI();
}

void XLAddTextDialog::onTextChanged() {
	onTextPropertyChanged(m_textProperty, ui->textEdit);
}

void XLAddTextDialog::onSelectFont() {
	onFontPropertyChanged(m_fontProperty, ui->fontNameLabel);
}

void XLAddTextDialog::onColorButtonClicked() {
	// update checked state of all color buttons
	QPushButton* btn = dynamic_cast<QPushButton*>(sender());
	selectColorButton(btn);

	// get color
	QColor color = btn->palette().button().color();

	// update color1
	const char* name1 = obs_property_name(m_color1Property);
	obs_data_set_int(m_settings, name1, XLUtil::color2Int(color));
	postPropertyChanged(m_color1Property);

	// update color2 also
	const char* name2 = obs_property_name(m_color2Property);
	obs_data_set_int(m_settings, name2, XLUtil::color2Int(color));
	postPropertyChanged(m_color2Property);
}

void XLAddTextDialog::onColorChanged() {
	// get changed color
	QColor color = onColorPropertyChanged(m_color1Property);

	// update color2 also
	const char* name = obs_property_name(m_color2Property);
	obs_data_set_int(m_settings, name, XLUtil::color2Int(color));
	postPropertyChanged(m_color2Property);

	// update custom color button
	ui->customColorButton->setStyleSheet(QString("background-color: %1;").arg(color.name(QColor::HexArgb)));
	selectColorButton(ui->customColorButton);
}

void XLAddTextDialog::initOtherUI() {
	// color event
	connect(ui->colorButton1, SIGNAL(clicked()), this, SLOT(onColorButtonClicked()));
	connect(ui->colorButton2, SIGNAL(clicked()), this, SLOT(onColorButtonClicked()));
	connect(ui->colorButton3, SIGNAL(clicked()), this, SLOT(onColorButtonClicked()));
	connect(ui->colorButton4, SIGNAL(clicked()), this, SLOT(onColorButtonClicked()));
	connect(ui->colorButton5, SIGNAL(clicked()), this, SLOT(onColorButtonClicked()));
	connect(ui->colorButton6, SIGNAL(clicked()), this, SLOT(onColorButtonClicked()));
	connect(ui->colorButton7, SIGNAL(clicked()), this, SLOT(onColorButtonClicked()));
	connect(ui->colorButton8, SIGNAL(clicked()), this, SLOT(onColorButtonClicked()));
	connect(ui->colorButton9, SIGNAL(clicked()), this, SLOT(onColorButtonClicked()));
	connect(ui->colorButton10, SIGNAL(clicked()), this, SLOT(onColorButtonClicked()));
	connect(ui->customColorButton, SIGNAL(clicked()), this, SLOT(onColorButtonClicked()));

	// get current color
	const char* name = obs_property_name(m_color1Property);
	long long val = obs_data_get_int(m_settings, name);
	QColor color = XLUtil::int2Color(val);

	// update custom color button
	ui->customColorButton->setStyleSheet(QString("background-color: %1;").arg(color.name(QColor::HexArgb)));
	selectColorButton(ui->customColorButton);
}

void XLAddTextDialog::selectColorButton(QPushButton* btn) {
	ui->colorButton1->setChecked(false);
	ui->colorButton2->setChecked(false);
	ui->colorButton3->setChecked(false);
	ui->colorButton4->setChecked(false);
	ui->colorButton5->setChecked(false);
	ui->colorButton6->setChecked(false);
	ui->colorButton7->setChecked(false);
	ui->colorButton8->setChecked(false);
	ui->colorButton9->setChecked(false);
	ui->colorButton10->setChecked(false);
	ui->customColorButton->setChecked(false);
	btn->setChecked(true);
}