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

#include "xl-register-dialog.hpp"
#include "window-basic-main.hpp"
#include "xl-login-dialog.hpp"
#include <QKeyEvent>

using namespace std;

XLRegisterDialog::XLRegisterDialog(OBSBasic *parent) :
	QDialog (parent),
	ui(new Ui::XLRegisterDialog),
	m_main(parent)
{
	// init ui
	ui->setupUi(this);

	// set title
	setWindowTitle(L("XL.Register.Title"));

	// update button text
	ui->buttonBox->button(QDialogButtonBox::Ok)->setText(L("Register"));
	ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(L("XL.Register.Already.Has.Account"));
}

void XLRegisterDialog::accept() {

}

void XLRegisterDialog::reject() {
	// close self
	QDialog::reject();

	// show login dialog
	XLLoginDialog login(m_main);
	login.exec();
}

void XLRegisterDialog::keyPressEvent(QKeyEvent *event) {
	switch (event->key()) {
		case Qt::Key_Escape:
			break;
		default:
			QDialog::keyPressEvent(event);
			break;
	}
}