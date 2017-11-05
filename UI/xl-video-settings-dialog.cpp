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

#include "xl-util.hpp"
#include "window-basic-main.hpp"
#include "xl-video-settings-dialog.hpp"
#include "xl-title-bar-sub.hpp"
#include "xl-frameless-window-util.hpp"

using namespace std;

XLVideoSettingsDialog::XLVideoSettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::XLVideoSettingsDialog) {
	ui->setupUi(this);

	// setup frameless ui
	XLFramelessWindowUtil::setupUI(this);

	// create title bar
	m_titleBar = new XLTitleBarSub(this);
	m_titleBar->init();
	m_titleBar->move(0, 0);
	connect(m_titleBar, &XLTitleBar::windowRequestClose, this, &QDialog::reject);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-video-settings-dialog");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

XLVideoSettingsDialog::~XLVideoSettingsDialog() {
}

void XLVideoSettingsDialog::on_yesButton_clicked() {
	accept();
}