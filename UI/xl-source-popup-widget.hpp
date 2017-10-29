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
#include <QStyledItemDelegate>
#include "ui_XLSourcePopupWidget.h"

class XLSourcePopupWidget : public QWidget {
	Q_OBJECT

public:
	typedef enum {
		MODE_APP,
		MODE_MONITOR,
		MODE_TOOLBOX
	} Mode;

private:
	std::unique_ptr<Ui::XLSourcePopupWidget> ui;
	QWidget* m_refLocWidget;
	Mode m_mode;

protected:
	void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
	void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

public:
	XLSourcePopupWidget(QWidget* parent = Q_NULLPTR);
	virtual ~XLSourcePopupWidget();

	// reference location, if set, place it at center bottom of this widget
	void setReferenceWidget(QWidget* w);

	// set mode
	void setMode(Mode m);
};

class XLSourcePopupWidgetListDelegate : public QStyledItemDelegate {
	Q_OBJECT

private:
	XLSourcePopupWidget::Mode m_mode;

public:
	XLSourcePopupWidgetListDelegate(QObject* parent = Q_NULLPTR, XLSourcePopupWidget::Mode mode = XLSourcePopupWidget::MODE_APP);
	virtual ~XLSourcePopupWidgetListDelegate();

	// implementation
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
};