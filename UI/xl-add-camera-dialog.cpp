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
#include "xl-add-camera-dialog.hpp"
#include "qt-wrappers.hpp"
#include "xl-util.hpp"
#include "window-basic-main.hpp"

using namespace std;

XLAddCameraDialog::XLAddCameraDialog(QWidget *parent, obs_source_t* source) :
	XLAddSourceDialog (parent, source),
	ui(new Ui::XLAddCameraDialog) {
}

XLAddCameraDialog::~XLAddCameraDialog() {
}

void XLAddCameraDialog::on_yesButton_clicked() {
	accept();
}

void XLAddCameraDialog::loadUI() {
	// init ui
	ui->setupUi(this);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-add-camera-dialog");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

OBSQTDisplay* XLAddCameraDialog::getDisplay() {
	return ui->preview;
}

void XLAddCameraDialog::onPresetResolutionChanged(int index) {
	onListPropertyChanged(m_presetProperty, ui->resolutionComboBox, index);
}

void XLAddCameraDialog::onDeviceChanged(int index) {
	if(onListPropertyChanged(m_deviceProperty, ui->cameraComboBox, index)) {
		populateListProperty(m_presetProperty, ui->resolutionComboBox);
	}
}

void XLAddCameraDialog::loadProperties() {
	// find properties we want to set
#ifdef Q_OS_OSX
	const char* id1 = "device";
#else
	const char* id1 = "video_device_id";
#endif
#ifdef Q_OS_OSX
	const char* id2 = "preset";
#else
	const char* id2 = "resolution";
#endif
	m_deviceProperty = obs_properties_get(m_properties.get(), id1);
	m_presetProperty = obs_properties_get(m_properties.get(), id2);

	// bind ui
	bindPropertyUI(m_deviceProperty, ui->cameraComboBox, Q_NULLPTR, SLOT(onDeviceChanged(int)));
	bindPropertyUI(m_presetProperty, ui->resolutionComboBox, Q_NULLPTR, SLOT(onPresetResolutionChanged(int)));
}