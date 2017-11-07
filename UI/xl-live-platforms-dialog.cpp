/******************************************************************************
    Copyright (C) 2014 by Luma <stubma@gmail.com>

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

#include "window-basic-main.hpp"
#include "xl-util.hpp"
#include "xl-live-platforms-dialog.hpp"
#include "xl-frameless-window-util.hpp"
#include "xl-title-bar-sub.hpp"

using namespace std;

XLLivePlatformsDialog::XLLivePlatformsDialog(QWidget* parent) :
	QDialog (parent),
	ui(new Ui::XLLivePlatformsDialog) {
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
	QString qssPath = XLUtil::getQssPathByName("xl-live-platforms-dialog");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);

	// set title
	m_titleBar->setWindowTitle(L("Live.Platforms.Title"));

	// remove focus rect
	ui->listView->setAttribute(Qt::WA_MacShowFocusRect, false);
}

XLLivePlatformsDialog::~XLLivePlatformsDialog() {

}

void XLLivePlatformsDialog::on_yesButton_clicked() {
	accept();
}