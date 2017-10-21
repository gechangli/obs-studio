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

#include "xl-name-dialog.hpp"
#include "qt-wrappers.hpp"
#include "ui_NameDialog.h"
#include "obs-app.hpp"
#include "xl-util.hpp"

using namespace std;

XLNameDialog::XLNameDialog(QWidget *parent) :
	QDialog (parent),
	ui(new Ui::XLNameDialog) {
	// init ui
	ui->setupUi(this);

	// disable focus rect
	ui->nameEdit->setAttribute(Qt::WA_MacShowFocusRect, false);

	// title bar
#ifdef Q_OS_WIN
	// setup frameless ui
	XLFramelessWindowUtil::setupUI(this);

	// create title bar
	m_titleBar = new XLTitleBarSub(this);
	m_titleBar->init();
	m_titleBar->move(0, 0);
	connect(m_titleBar, &XLTitleBar::windowRequestClose, this, &QDialog::reject);
#endif

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-name-dialog");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

void XLNameDialog::on_yesButton_clicked() {
	accept();
}

void XLNameDialog::on_noButton_clicked() {
	reject();
}

void XLNameDialog::setWindowTitle(const QString& title) {
#ifdef Q_OS_WIN
	m_titleBar->setWindowTitle(title);
#else
	QDialog::setWindowTitle(title);
#endif
}

static bool IsWhitespace(char ch)  {
	return ch == ' ' || ch == '\t';
}

bool XLNameDialog::AskForName(QWidget *parent, const QString &title,
							  const QString &text, string &str, const QString &placeHolder) {
	XLNameDialog dialog(parent);
	dialog.setWindowTitle(title);
	dialog.ui->label->setText(text);
	dialog.ui->nameEdit->setText(placeHolder);
	dialog.ui->nameEdit->selectAll();

	bool accepted = (dialog.exec() == DialogCode::Accepted);
	if (accepted) {
		str = QT_TO_UTF8(dialog.ui->nameEdit->text());
		while (str.size() && IsWhitespace(str.back()))
			str.erase(str.end() - 1);
		while (str.size() && IsWhitespace(str.front()))
			str.erase(str.begin());
	}

	return accepted;
}
