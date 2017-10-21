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
#include "ui_XLAddCameraDialog.h"

class XLTitleBarSub;

class XLAddCameraDialog : public QDialog {
	Q_OBJECT

private:
	std::unique_ptr<Ui::XLAddCameraDialog> ui;
	XLTitleBarSub* m_titleBar;

private slots:
	void on_yesButton_clicked();

public:
	XLAddCameraDialog(QWidget *parent);

	// override
	void setWindowTitle(const QString& title);
};
