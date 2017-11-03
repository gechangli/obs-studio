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
#include "xl-add-picture-dialog.hpp"
#include "qt-wrappers.hpp"
#include "xl-util.hpp"
#include "window-basic-main.hpp"

using namespace std;

XLAddPictureDialog::XLAddPictureDialog(QWidget *parent, obs_source_t* source) :
	XLAddSourceDialog (parent, source),
	ui(new Ui::XLAddPictureDialog) {
}

XLAddPictureDialog::~XLAddPictureDialog() {
}

void XLAddPictureDialog::on_yesButton_clicked() {
	accept();
}

void XLAddPictureDialog::on_noButton_clicked() {
	reject();
}

void XLAddPictureDialog::loadUI() {
	// init ui
	ui->setupUi(this);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-add-picture-dialog");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

OBSQTDisplay* XLAddPictureDialog::getDisplay() {
	return ui->preview;
}

void XLAddPictureDialog::loadProperties() {
	// load properties from source
	m_properties.reset(obs_source_properties(m_source));

	// find properties we want to set
	const char* id1 = "file";

	// check defer update flag
	uint32_t flags = obs_properties_get_flags(m_properties.get());
	m_deferUpdate = (flags & OBS_PROPERTIES_DEFER_UPDATE) != 0;

	// property
	m_fileProperty = obs_properties_get(m_properties.get(), id1);

	// bind ui
	bindPropertyUI(m_fileProperty, ui->fileNameLabel, ui->selectFileButton, SLOT(onSelectFile()));
}

void XLAddPictureDialog::onSelectFile() {
	onPathPropertyChanged(m_fileProperty, ui->fileNameLabel);
}