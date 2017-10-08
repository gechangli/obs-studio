/******************************************************************************
    Copyright (C) 2013 by luma <stubma@gmail.com>

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

#include <QWidget>
#include <memory>
#include "ui_XLUserPopupWidget.h"

class XLUserPopupWidget : public QWidget {
	Q_OBJECT

private:
	std::unique_ptr<Ui::UserPopupWidget> ui;
	QWidget* m_refLocWidget;

protected:
	void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

private slots:
	void onModifyPasswordClicked();

public:
	XLUserPopupWidget(QWidget* parent = Q_NULLPTR);
	virtual ~XLUserPopupWidget();
	static XLUserPopupWidget* instance();

	// reference location, if set, place it at center bottom of this widget
	void setReferenceLocation(QWidget* w);
};