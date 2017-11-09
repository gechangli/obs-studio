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
#include "window-basic-main.hpp"

Q_DECLARE_METATYPE(OBSSceneItem);

XLSourceListView::XLSourceListView(QWidget* parent) :
	QListView(parent),
	m_lastHoverWidget(Q_NULLPTR) {
	setSelectionMode(QAbstractItemView::SingleSelection);
}

XLSourceListView::~XLSourceListView() {
}

void XLSourceListView::setup() {
	QStandardItemModel* model = new QStandardItemModel();
	setItemDelegate(new XLSourceListDelegate());
	setModel(model);
}

void XLSourceListView::setModel(QAbstractItemModel *model) {
	QListView::setModel(model);

	// create widget for every row
	for (int i = 0; i < model->rowCount(); i++) {
		QModelIndex index = model->index(i, 0);
		XLSourceListItemWidget* widget = new XLSourceListItemWidget;
		setIndexWidget(index, widget);
		widget->setIndex(i);
		widget->update();
	}

	// connect
	connect(model, &QAbstractItemModel::dataChanged, this, &XLSourceListView::onDataChanged);
	connect(model, &QAbstractItemModel::rowsInserted, this, &XLSourceListView::onRowsInserted);
	connect(model, &QAbstractItemModel::rowsRemoved, this, &XLSourceListView::onRowsRemoved);
	connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, &XLSourceListView::onCurrentRowChanged);
}

void XLSourceListView::onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous) {
	// deselect previous
	XLSourceListItemWidget* widget = dynamic_cast<XLSourceListItemWidget*>(indexWidget(previous));
	if(widget) {
		obs_sceneitem_select(widget->getSceneItem(), false);
	}

	// select current
	widget = dynamic_cast<XLSourceListItemWidget*>(indexWidget(current));
	if(widget) {
		obs_sceneitem_select(widget->getSceneItem(), true);
	}
}

void XLSourceListView::onRowsRemoved(const QModelIndex &parent, int first, int last) {
	QStandardItemModel* model = static_cast<QStandardItemModel*>(this->model());
	int rc = model->rowCount();
	for(int i = first; i < rc; i++) {
		QModelIndex index = model->index(i, 0);
		XLSourceListItemWidget* widget = dynamic_cast<XLSourceListItemWidget*>(indexWidget(index));
		widget->setIndex(i);
		widget->update();
	}
}

void XLSourceListView::onRowsInserted(const QModelIndex &parent, int first, int last) {
	QStandardItemModel* model = static_cast<QStandardItemModel*>(this->model());
	int rc = model->rowCount();
	for(int i = last + 1; i < rc; i++) {
		QModelIndex index = model->index(i, 0);
		XLSourceListItemWidget* widget = dynamic_cast<XLSourceListItemWidget*>(indexWidget(index));
		widget->setIndex(i);
		widget->update();
	}
	for(int i = first; i <= last; i++) {
		QModelIndex index = model->index(i, 0);
		XLSourceListItemWidget* widget = new XLSourceListItemWidget;
		setIndexWidget(index, widget);
		widget->setIndex(i);
		widget->update();
	}
}

void XLSourceListView::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
	QStandardItemModel* model = static_cast<QStandardItemModel*>(this->model());
	for(int i = topLeft.row(); i <= bottomRight.row(); i++) {
		QModelIndex index = model->index(i, 0);
		XLSourceListItemWidget* widget = dynamic_cast<XLSourceListItemWidget*>(indexWidget(index));
		widget->setIndex(i);
		widget->update();
	}
}

void XLSourceListView::mousePressEvent(QMouseEvent* event) {
	// save this position for later dragging
	m_dragPos = event->pos();

	// if not valid index, clear selection
	QModelIndex index = indexAt(m_dragPos);
	if(!index.isValid()) {
		QStandardItemModel* model = static_cast<QStandardItemModel*>(this->model());
		int rc = model->rowCount();
		for(int i = 0; i < rc; i++) {
			XLSourceListItemWidget* widget = dynamic_cast<XLSourceListItemWidget*>(indexWidget(model->index(i, 0)));
			OBSSceneItem item = widget->getSceneItem();
			if(obs_sceneitem_selected(item)) {
				obs_sceneitem_select(item, false);
			}
		}
	}

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
	// check mime type
	if (event->mimeData()->hasFormat("application/x-XLSourceListView-MoveRow")) {
		event->setDropAction(Qt::MoveAction);
		event->accept();
	} else {
		event->ignore();
	}

	// clear last widget style
	if(m_lastHoverWidget) {
		m_lastHoverWidget->setStyleSheet("");
	}

	// check hover position, get insert index
	// also set style sheet to show insert position
	QModelIndex index = indexAt(event->pos());
	m_insertIndex = index.row();
	XLSourceListItemWidget* item = dynamic_cast<XLSourceListItemWidget*>(indexWidget(index));
	if(item) {
		// if mouse position is in the bottom half of item, increase insert position
		QSize itemSize = item->size();
		int y = event->pos().y() - item->pos().y();
		if(y > itemSize.height() / 2) {
			m_insertIndex = index.row() + 1;
		}

		// set style
		if(m_insertIndex > index.row()) {
			item->setStyleSheet("border-bottom: 1px solid red;");
		} else {
			item->setStyleSheet("border-top: 1px solid red;");
		}

		// save
		m_lastHoverWidget = item;
	} else {
		// if no widget at position, we think it is at the last position
		QAbstractItemModel* m = model();
		item = dynamic_cast<XLSourceListItemWidget*>(indexWidget(m->index(m->rowCount() - 1, 0)));
		item->setStyleSheet("border-bottom: 1px solid red;");
		m_lastHoverWidget = item;
	}
}

void XLSourceListView::dropEvent(QDropEvent *event) {
	// drop based on mime type
	if (event->mimeData()->hasFormat("application/x-XLSourceListView-MoveRow")) {
		// get row count
		QStandardItemModel* m = dynamic_cast<QStandardItemModel*>(model());
		int rc = m->rowCount();

		// check if insertion will change anything
		bool same = false;
		if((m_insertIndex >= rc || m_insertIndex == -1) && m_dragIndex.row() == rc - 1) {
			same = true;
		} else if(m_dragIndex.row() < m_insertIndex && m_dragIndex.row() == m_insertIndex - 1) {
			same = true;
		} else if(m_dragIndex.row() == m_insertIndex) {
			same = true;
		}

		// if will, accept drop
		if(!same) {
			// make a copy of source item
			QStandardItem* srcItem = m->itemFromIndex(m_dragIndex);
			QStandardItem* newItem = new QStandardItem();
			OBSSceneItem sceneItem = srcItem->data(static_cast<int>(QtDataRole::OBSRef)).value<OBSSceneItem>();
			newItem->setData(QVariant::fromValue(sceneItem), static_cast<int>(QtDataRole::OBSRef));

			// manipulate model to remove row and insert again
			// it will trigger signal to update widget
			// meanwhile, we need re-order source position
			m->removeRow(m_dragIndex.row());
			if(m_insertIndex >= rc || m_insertIndex == -1) {
				m->appendRow(newItem);
				obs_sceneitem_set_order_position(sceneItem, rc - 1);
			} else if(m_dragIndex.row() < m_insertIndex) {
				m->insertRow(m_insertIndex - 1, newItem);
				obs_sceneitem_set_order_position(sceneItem, m_insertIndex - 1);
			} else {
				m->insertRow(m_insertIndex, newItem);
				obs_sceneitem_set_order_position(sceneItem, m_insertIndex);
			}

			// accept this drop
			event->setDropAction(Qt::MoveAction);
			event->accept();
		} else {
			event->ignore();
		}
	} else {
		event->ignore();
	}

	// clear style
	if(m_lastHoverWidget) {
		m_lastHoverWidget->setStyleSheet("");
	}
}

void XLSourceListView::startDrag(Qt::DropActions supportedActions) {
	// get dragged item index from drag position
	m_dragIndex = indexAt(m_dragPos);
	m_insertIndex = m_dragIndex.row();
	m_lastHoverWidget = Q_NULLPTR;
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

XLSourceListDelegate::XLSourceListDelegate(QObject* parent) :
	QStyledItemDelegate(parent) {

}

XLSourceListDelegate::~XLSourceListDelegate() {
}

QSize XLSourceListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
	return QSize(0, 50);
}