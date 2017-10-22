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
#include "ui_XLSceneCollectionPopupWidget.h"

class XLSceneCollectionPopupWidget : public QWidget {
	Q_OBJECT

private:
	std::unique_ptr<Ui::XLSceneCollectionPopupWidget> ui;
	QWidget* m_refLocWidget;

protected:
	void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

public:
	XLSceneCollectionPopupWidget(QWidget* parent = Q_NULLPTR);
	virtual ~XLSceneCollectionPopupWidget();
	static XLSceneCollectionPopupWidget* instance();

	// reference widget
	void setReferenceWidget(QWidget* w);
};