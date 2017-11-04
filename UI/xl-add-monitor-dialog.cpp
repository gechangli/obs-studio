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
#include "xl-add-monitor-dialog.hpp"
#include "qt-wrappers.hpp"
#include "xl-util.hpp"
#include "window-basic-main.hpp"

using namespace std;

XLAddMonitorDialog::XLAddMonitorDialog(QWidget *parent, obs_source_t* source) :
	XLAddSourceDialog (parent, source),
	ui(new Ui::XLAddMonitorDialog) {
}

XLAddMonitorDialog::~XLAddMonitorDialog() {
}

void XLAddMonitorDialog::on_yesButton_clicked() {
	accept();
}

void XLAddMonitorDialog::on_noButton_clicked() {
	reject();
}

void XLAddMonitorDialog::loadUI() {
	// init ui
	ui->setupUi(this);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-add-monitor-dialog");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

OBSQTDisplay* XLAddMonitorDialog::getDisplay() {
	return ui->preview;
}

void XLAddMonitorDialog::loadProperties() {
	// find properties we want to set
#ifdef Q_OS_OSX
	const char* id1 = "display";
#elif defined(Q_OS_WIN)
	const char* id1 = "monitor";
#endif
#ifdef Q_OS_OSX
	const char* id2 = "show_cursor";
#elif defined(Q_OS_WIN)
	const char* id2 = "capture_cursor";
#endif

	// property
	m_monitorProperty = obs_properties_get(m_properties.get(), id1);
	m_showCursorProperty = obs_properties_get(m_properties.get(), id2);

	// bind ui
	bindPropertyUI(m_showCursorProperty, ui->showCursorCheckBox, Q_NULLPTR, SLOT(onShowCursorChanged(int)));
}

void XLAddMonitorDialog::onShowCursorChanged(int state) {
	onBoolPropertyChanged(m_showCursorProperty, ui->showCursorCheckBox);
}

void XLAddMonitorDialog::presetMonitor(int monitorId) {
	const char* name = obs_property_name(m_monitorProperty);
	obs_data_set_int(m_settings, name, monitorId);
	postPropertyChanged(m_monitorProperty);
}