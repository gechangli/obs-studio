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
#include "ui_XLAddCameraDialog.h"
#include "obs.hpp"
#include "xl-add-source-dialog.hpp"

class XLTitleBarSub;
class QComboBox;

class XLAddCameraDialog : public XLAddSourceDialog {
	Q_OBJECT

private:
	std::unique_ptr<Ui::XLAddCameraDialog> ui;
	obs_property_t* m_deviceProperty;
	obs_property_t* m_presetProperty;

private slots:
	void on_yesButton_clicked();
	void onDeviceChanged(int index);
	void onPresetResolutionChanged(int index);

protected:
	void loadUI() Q_DECL_OVERRIDE;
	OBSQTDisplay* getDisplay() Q_DECL_OVERRIDE;
	void loadProperties() Q_DECL_OVERRIDE;

public:
	XLAddCameraDialog(QWidget *parent, obs_source_t* source);
	virtual ~XLAddCameraDialog();
};
