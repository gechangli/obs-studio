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
#include "xl-add-window-dialog.hpp"
#include "qt-wrappers.hpp"
#include "xl-util.hpp"
#include "window-basic-main.hpp"

using namespace std;

XLAddWindowDialog::XLAddWindowDialog(QWidget *parent, obs_source_t* source) :
	XLAddSourceDialog (parent, source),
	ui(new Ui::XLAddWindowDialog) {
}

XLAddWindowDialog::~XLAddWindowDialog() {
}

void XLAddWindowDialog::on_yesButton_clicked() {
	accept();
}

void XLAddWindowDialog::on_noButton_clicked() {
	reject();
}

void XLAddWindowDialog::loadUI() {
	// init ui
	ui->setupUi(this);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-add-window-dialog");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

OBSQTDisplay* XLAddWindowDialog::getDisplay() {
	return ui->preview;
}

void XLAddWindowDialog::onWindowChanged(int index) {
	QComboBox* combo = static_cast<QComboBox*>(sender());
	onListPropertyChanged(m_windowProperty, combo, index);
}

void XLAddWindowDialog::loadProperties() {
	// find properties we want to set
	const char* id1 = "window";
	m_windowProperty = obs_properties_get(m_properties.get(), id1);

	// bind ui
	bindPropertyUI(m_windowProperty, ui->windowComboBox, Q_NULLPTR, SLOT(onWindowChanged(int)));
}

void XLAddWindowDialog::presetWindow(int windowIndex) {
	ui->windowComboBox->setCurrentIndex(windowIndex);
}