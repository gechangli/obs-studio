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

#include "xl-message-box.hpp"
#include "qt-wrappers.hpp"
#include "obs-app.hpp"
#include "xl-util.hpp"
#include "xl-title-bar-sub.hpp"
#include "xl-frameless-window-util.hpp"
#include "window-basic-main.hpp"

using namespace std;

QDialog::DialogCode XLMessageBox::question(
	QWidget *parent,
	const QString &title,
	const QString &text,
	const QString defaultButtonText,
	const QString otherButtonText) {
	XLMessageBox* box = new XLMessageBox(parent);
	box->setWindowTitle(title);
	box->setMessage(text);
	box->setPositiveButton(defaultButtonText);
	box->setNegativeButton(otherButtonText);
	return (QDialog::DialogCode)box->exec();
}

XLMessageBox::XLMessageBox(QWidget *parent) :
	QDialog (parent),
	ui(new Ui::XLMessageBox) {
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
	QString qssPath = XLUtil::getQssPathByName("xl-message-box");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

void XLMessageBox::setPositiveButton(const QString& btnText) {
	ui->yesButton->setText(btnText);
}

void XLMessageBox::setNegativeButton(const QString& btnText) {
	ui->noButton->setText(btnText);
}

void XLMessageBox::on_yesButton_clicked() {
	accept();
}

void XLMessageBox::on_noButton_clicked() {
	reject();
}

void XLMessageBox::setWindowTitle(const QString& title) {
	m_titleBar->setWindowTitle(title);
}

void XLMessageBox::setMessage(const QString& msg) {
	ui->msgLabel->setText(msg);
}