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
#include <QCloseEvent>
#include "xl-add-video-dialog.hpp"
#include "qt-wrappers.hpp"
#include "xl-util.hpp"
#include "window-basic-main.hpp"

using namespace std;

XLAddVideoDialog::XLAddVideoDialog(QWidget *parent, obs_source_t* source) :
	XLAddSourceDialog (parent, source),
	ui(new Ui::XLAddVideoDialog) {
}

XLAddVideoDialog::~XLAddVideoDialog() {
}

void XLAddVideoDialog::on_yesButton_clicked() {
	accept();
}

void XLAddVideoDialog::on_noButton_clicked() {
	reject();
}

void XLAddVideoDialog::loadUI() {
	// init ui
	ui->setupUi(this);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-add-video-dialog");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

OBSQTDisplay* XLAddVideoDialog::getDisplay() {
	return Q_NULLPTR;
}

void XLAddVideoDialog::loadProperties() {
	// find properties we want to set
	const char* id1 = "local_file";
	const char* id2 = "looping";

	// property
	m_fileProperty = obs_properties_get(m_properties.get(), id1);
	m_loopProperty = obs_properties_get(m_properties.get(), id2);

	// bind ui
	bindPropertyUI(m_fileProperty, ui->fileNameLabel, ui->selectFileButton, SLOT(onSelectFile()));
	bindPropertyUI(m_loopProperty, ui->loopCheckBox, Q_NULLPTR, SLOT(onLoopChanged(int)));
}

void XLAddVideoDialog::onSelectFile() {
	onPathPropertyChanged(m_fileProperty, ui->fileNameLabel);
}

void XLAddVideoDialog::onLoopChanged(int state) {
	onBoolPropertyChanged(m_loopProperty, ui->loopCheckBox);
}