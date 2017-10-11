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
#include <QStandardItemModel>
#include <QMimeData>
#include <QDrag>
#include "xl-source-list-view.hpp"
#include "xl-source-list-item-widget.hpp"

XLSourceListView::XLSourceListView(QWidget* parent) :
	QListView(parent) {
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
	// save this position for later dragging
	m_dragPos = event->pos();

	// call super
	QListView::mousePressEvent(event);
}

void XLSourceListView::dragEnterEvent(QDragEnterEvent *event) {
	// check mime type
	if (event->mimeData()->hasFormat("application/x-XLSourceListView-MoveRow")) {
		event->accept();
	} else {
		event->ignore();
	}
}

void XLSourceListView::dragMoveEvent(QDragMoveEvent *event) {
	QModelIndex index = indexAt(event->pos());
	XLSourceListItemWidget* item = dynamic_cast<XLSourceListItemWidget*>(indexWidget(index));
	if(!item) {
		return;
	}

	// check mime type
	if (event->mimeData()->hasFormat("application/x-XLSourceListView-MoveRow")) {
		event->setDropAction(Qt::MoveAction);
		event->accept();
	} else {
		event->ignore();
	}
}

void XLSourceListView::dropEvent(QDropEvent *event) {
	// drop based on mime type
	if (event->mimeData()->hasFormat("application/x-XLSourceListView-MoveRow")) {
		// get source and destination
		QStandardItemModel* m = dynamic_cast<QStandardItemModel*>(model());
		QModelIndex dstIndex = indexAt(event->pos());
		QStandardItem* srcItem = m->itemFromIndex(m_dragIndex);
		srcItem = new QStandardItem(srcItem->text());

		// manipulate model to move row
		m->removeRow(m_dragIndex.row());
		if(m_dragIndex.row() < dstIndex.row()) {
			m->insertRow(dstIndex.row() - 1, srcItem);
			setIndexWidget(m->index(dstIndex.row() - 1, 0), new XLSourceListItemWidget());
		} else if(dstIndex.row() == -1) {
			m->appendRow(srcItem);
			setIndexWidget(m->index(m->rowCount() - 1, 0), new XLSourceListItemWidget());
		} else {
			m->insertRow(dstIndex.row(), srcItem);
			setIndexWidget(dstIndex, new XLSourceListItemWidget());
		}

		// accept this drop
		event->setDropAction(Qt::MoveAction);
		event->accept();
	} else {
		event->ignore();
	}
}

void XLSourceListView::startDrag(Qt::DropActions supportedActions) {
	// get dragged item index from drag position
	m_dragIndex = indexAt(m_dragPos);
	XLSourceListItemWidget* item = dynamic_cast<XLSourceListItemWidget*>(indexWidget(m_dragIndex));
	if (!item) {
		return;
	}

	// get a widget snapshot for dragging indicator
	m_dragImg = item->grab();

	// set a mime data for this dragging
	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);
	dataStream << m_dragIndex.row();
	QMimeData *mimeData = new QMimeData;
	mimeData->setData("application/x-XLSourceListView-MoveRow", itemData);

	// create drag and execute
	QDrag* drag = new QDrag(this);
	drag->setMimeData(mimeData);
	drag->setHotSpot(QPoint(m_dragPos.x() - item->pos().x(), m_dragPos.y() - item->pos().y()));
	drag->setPixmap(m_dragImg);
	drag->exec(supportedActions);
}