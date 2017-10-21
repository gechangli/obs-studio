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

#pragma once

#include <QDialog>
#include <string>
#include <memory>
#include <QDialog>
#include "ui_XLMessageBox.h"
#include "window-basic-main.hpp"

class XLTitleBarSub;

class XLMessageBox : public QDialog {
	Q_OBJECT

private:
	std::unique_ptr<Ui::XLMessageBox> ui;
	XLTitleBarSub* m_titleBar;

private slots:
	void on_yesButton_clicked();
	void on_noButton_clicked();

public:
	XLMessageBox(QWidget *parent);

	// override
	void setWindowTitle(const QString& title);

	// set message
	void setMessage(const QString& msg);

	// set button text
	void setPositiveButton(const QString& btnText);
	void setNegativeButton(const QString& btnText);

	// show message box
	static QDialog::DialogCode question(
		QWidget *parent,
		const QString &title,
		const QString &text,
		const QString defaultButtonText = L("OK"),
		const QString otherButtonText = L("Cancel"));
};
