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
#include "obs.hpp"

class XLTitleBarSub;
class QComboBox;
class OBSQTDisplay;

class XLAddSourceDialog : public QDialog {
	Q_OBJECT

	// pointer type for properties
	using properties_delete_t = decltype(&obs_properties_destroy);
	using properties_t = std::unique_ptr<obs_properties_t, properties_delete_t>;

protected:
	XLTitleBarSub* m_titleBar;
	OBSSource m_source;
	OBSData m_oldSettings;
	OBSData m_settings;
	properties_t m_properties;
	bool m_rollback;
	bool m_deferUpdate;
	bool m_editMode;

private:
	void cleanup();
	static void drawPreview(void *data, uint32_t cx, uint32_t cy);
	static void getScaleAndCenterPos(int baseCX, int baseCY, int windowCX, int windowCY, int &x, int &y, float &scale);
	void addComboItem(QComboBox *combo, obs_property_t *prop, obs_combo_format format, size_t idx);
	void bindListPropertyUI(obs_property_t *prop, QComboBox *combo, const char *slot);

protected:
	void bindPropertyUI(obs_property_t* prop, QWidget* widget, const char* slot);
	bool onListPropertyChanged(obs_property_t* prop, QComboBox* combo, int index);
	void populateListProperty(obs_property_t* prop, QComboBox* combo);

	// subclass implmeentation
	virtual void loadUI() = 0;
	virtual OBSQTDisplay* getDisplay() = 0;
	virtual void loadProperties() = 0;

public:
	XLAddSourceDialog(QWidget *parent, obs_source_t* source);
	virtual ~XLAddSourceDialog();

	// init must be called after constructor
	void init();

	// override
	void setWindowTitle(const QString& title);
	void reject();
	void accept();

	// getter/setter
	obs_source_t* getSource();
	void setEditMode(bool v);
};
