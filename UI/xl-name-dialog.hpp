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
#include "ui_XLNameDialog.h"

class XLTitleBarSub;

class XLNameDialog : public QDialog {
	Q_OBJECT

private:
	std::unique_ptr<Ui::XLNameDialog> ui;

	// because this dialog in macOS is displayed as sheet
	// so we only use custom title in windows
#ifdef Q_OS_WIN
	XLTitleBarSub* m_titleBar;
#endif

private slots:
	void on_yesButton_clicked();
	void on_noButton_clicked();

public:
	XLNameDialog(QWidget *parent);

	// override
	void setWindowTitle(const QString& title);

	static bool AskForName(QWidget *parent, const QString &title,
			const QString &text, std::string &str,
			const QString &placeHolder = QString(""));
};
