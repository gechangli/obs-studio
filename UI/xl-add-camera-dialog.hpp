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
class QComboBox;

class XLAddCameraDialog : public QDialog {
	Q_OBJECT

	// pointer type for properties
	using properties_delete_t = decltype(&obs_properties_destroy);
	using properties_t = std::unique_ptr<obs_properties_t, properties_delete_t>;

private:
	std::unique_ptr<Ui::XLAddCameraDialog> ui;
	XLTitleBarSub* m_titleBar;
	OBSSource m_source;
	OBSData m_oldSettings;
	OBSData m_settings;
	properties_t m_properties;
	obs_property_t* m_deviceProperty;
	bool m_rollback;
	bool m_deferUpdate;

private slots:
	void on_yesButton_clicked();
	void onDeviceChanged(int index);

private:
	static void drawPreview(void *data, uint32_t cx, uint32_t cy);
	static void getScaleAndCenterPos(int baseCX, int baseCY, int windowCX, int windowCY, int &x, int &y, float &scale);
	void loadProperties();
	void bindPropertyUI(obs_property_t* prop, QWidget* widget);
	void bindComboBoxPropertyUI(obs_property_t* prop, QComboBox* combo);
	void addComboItem(QComboBox *combo, obs_property_t *prop, obs_combo_format format, size_t idx);

public:
	XLAddCameraDialog(QWidget *parent, obs_source_t* source);
	virtual ~XLAddCameraDialog();

	// override
	void setWindowTitle(const QString& title);
	void reject();

	// getter/setter
	obs_source_t* getSource();
};
