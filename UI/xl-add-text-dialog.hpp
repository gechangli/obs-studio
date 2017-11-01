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
#include "ui_XLAddTextDialog.h"
#include "obs.hpp"
#include "xl-add-source-dialog.hpp"

class XLTitleBarSub;
class QComboBox;

class XLAddTextDialog : public XLAddSourceDialog {
	Q_OBJECT

private:
	std::unique_ptr<Ui::XLAddTextDialog> ui;
	obs_property_t* m_fontProperty;
	obs_property_t* m_textProperty;
	obs_property_t* m_color1Property;

private slots:
	void on_yesButton_clicked();
	void on_noButton_clicked();
	void onSelectFont();
	void onTextChanged();
	void onColorButtonClicked();

private:
	void initOtherUI();
	void selectColorButton(QPushButton* btn);

protected:
	void loadUI() Q_DECL_OVERRIDE;
	OBSQTDisplay* getDisplay() Q_DECL_OVERRIDE;
	void loadProperties() Q_DECL_OVERRIDE;

public:
	XLAddTextDialog(QWidget *parent, obs_source_t* source);
	virtual ~XLAddTextDialog();
};
