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
#include "ui_XLVideoSettingsDialog.h"
#include "obs.hpp"

class XLTitleBarSub;

class XLVideoSettingsDialog : public QDialog {
	Q_OBJECT

private:
	std::unique_ptr<Ui::XLVideoSettingsDialog> ui;
	XLTitleBarSub* m_titleBar;

private slots:
	void on_yesButton_clicked();
	void on_superRadioButton_clicked(bool checked);
	void on_highRadioButton_clicked(bool checked);
	void on_normalRadioButton_clicked(bool checked);
	void on_customRadioButton_clicked(bool checked);

public:
	XLVideoSettingsDialog(QWidget *parent);
	virtual ~XLVideoSettingsDialog();

	// override
	void accept();
};