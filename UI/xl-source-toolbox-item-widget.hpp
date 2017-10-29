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
#include "ui_XLToolboxItemWidget.h"
#include "obs.h"
#include "xl-source-popup-widget.hpp"

class QStandardItemModel;

class XLSourceToolboxItemWidget : public QWidget {
	Q_OBJECT

private:
	std::unique_ptr<Ui::XLToolboxItemWidget> ui;
	Q_PROPERTY(int m_index READ getIndex WRITE setIndex);
	int m_index;
	XLSourcePopupWidget::Mode m_mode;

protected:
	void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private slots:
	void on_openButton_clicked();

public:
	XLSourceToolboxItemWidget(QWidget* parent = Q_NULLPTR, XLSourcePopupWidget::Mode mode = XLSourcePopupWidget::MODE_APP);
	virtual ~XLSourceToolboxItemWidget();

	// update item
	void update();

	// setter/getter
	int getIndex();
	void setIndex(int i);
	QStandardItemModel* getModel();
	void setIcon(QPixmap icon);
	void setName(QString name);
	void setDesc(QString desc);
};