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

#include <QListView>

class QDrag;
class XLSourceListItemWidget;

class XLSourceListView : public QListView {
	Q_OBJECT

private:
	QModelIndex m_dragIndex;
	int m_insertIndex;
	QPoint m_dragPos;
	QPixmap m_dragImg;
	XLSourceListItemWidget* m_lastHoverWidget;

protected:
	// overrides
	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
	void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
	void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
	void startDrag(Qt::DropActions supportedActions) Q_DECL_OVERRIDE;

private slots:
	void onRowsInserted(const QModelIndex &parent, int first, int last);
	void onRowsRemoved(const QModelIndex &parent, int first, int last);
	void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
	void onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);

public:
	XLSourceListView(QWidget* parent = Q_NULLPTR);
	virtual ~XLSourceListView();

	// overrides
	void setModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;
};