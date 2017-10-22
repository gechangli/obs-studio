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
#include "obs.hpp"

class XLTitleBarSub;

class XLAddCameraDialog : public QDialog {
	Q_OBJECT

private:
	std::unique_ptr<Ui::XLAddCameraDialog> ui;
	XLTitleBarSub* m_titleBar;
	OBSSource m_source;

private slots:
	void on_yesButton_clicked();

private:
	static void drawPreview(void *data, uint32_t cx, uint32_t cy);
	static void getScaleAndCenterPos(int baseCX, int baseCY, int windowCX, int windowCY, int &x, int &y, float &scale);

public:
	XLAddCameraDialog(QWidget *parent);

	// override
	void setWindowTitle(const QString& title);

	// getter/setter
	obs_source_t* getSource();
};
