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

#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDrag>
#include "xl-source-list-view.hpp"
#include "xl-source-list-item-widget.hpp"

XLSourceListView::XLSourceListView(QWidget* parent) :
	QListView(parent),
	m_drag(Q_NULLPTR) {
}

XLSourceListView::~XLSourceListView() {

}

void XLSourceListView::setModel(QAbstractItemModel *model) {
	QListView::setModel(model);

	// create widget for every row
	for (int i = 0; i < model->rowCount(); i++) {
		QModelIndex index = model->index(i, 0);
		XLSourceListItemWidget* widget = new XLSourceListItemWidget;
		setIndexWidget(index, widget);
	}
}

void XLSourceListView::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		m_dragIndex = indexAt(event->pos());
		XLSourceListItemWidget* item = dynamic_cast<XLSourceListItemWidget*>(indexWidget(m_dragIndex));
		if (!item) {
			return;
		}

		m_dragImg = item->grab();
		m_hotspotY = event->pos().y() - item->pos().y();
		startDrag(Qt::MoveAction);
	} else {
		QListView::mousePressEvent(event);
	}
}

void XLSourceListView::dragEnterEvent(QDragEnterEvent *event) {
	QListView::dragEnterEvent(event);
}

void XLSourceListView::dragMoveEvent(QDragMoveEvent *event) {
	QListView::dragMoveEvent(event);
}

void XLSourceListView::dropEvent(QDropEvent *event) {
	QListView::dropEvent(event);
}

void XLSourceListView::startDrag(Qt::DropActions supportedActions) {
	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);

	XLSourceListItemWidget *item = dynamic_cast<XLSourceListItemWidget*>(indexWidget(m_dragIndex));

	dataStream << m_dragIndex.row();

	QMimeData *mimeData = new QMimeData;
	mimeData->setData("application/x-QListView-DragAndDrop", itemData);

	m_drag = new QDrag(this);
	m_drag->setMimeData(mimeData);
	m_drag->setHotSpot(mapToParent(QPoint(item->x(), m_hotspotY)));
	m_drag->setPixmap(m_dragImg);

	if (m_drag->exec(Qt::MoveAction | Qt::CopyAction) == Qt::MoveAction) {
		model()->removeRow(m_dragIndex.row());
	}
}