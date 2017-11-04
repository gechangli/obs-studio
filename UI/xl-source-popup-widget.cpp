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

#include <QPainter>
#include <QStandardItemModel>
#include "xl-source-popup-widget.hpp"
#include "window-basic-main.hpp"
#include "xl-util.hpp"
#include "xl-source-toolbox-item-widget.hpp"
#include "xl-source-app-item-widget.hpp"

XLSourcePopupWidget::XLSourcePopupWidget(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::XLSourcePopupWidget),
	m_refLocWidget(Q_NULLPTR),
	m_mode(MODE_APP) {
	// init ui
	ui->setupUi(this);

	// make self transparent
	setAttribute(Qt::WA_TranslucentBackground);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-source-popup-widget");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

XLSourcePopupWidget::~XLSourcePopupWidget() {
}

void XLSourcePopupWidget::setMode(XLSourcePopupWidget::Mode m) {
	// set model
	QStandardItemModel* model = new QStandardItemModel();
	ui->listView->setItemDelegate(new XLSourcePopupWidgetListDelegate(ui->listView, m));
	ui->listView->setModel(model);

	// check model, load cells
	m_mode = m;
	switch(m_mode) {
		case MODE_TOOLBOX: {
			// set title
			ui->titleLabel->setText(L("Toolbox.Title"));

			// init rows
			model->appendRow(new QStandardItem());
			model->appendRow(new QStandardItem());
			model->appendRow(new QStandardItem());
			QPixmap icons[] = {
				QPixmap(":/res/images/source_text_pic.png"),
				QPixmap(":/res/images/source_picture_pic.png"),
				QPixmap(":/res/images/source_video_pic.png")
			};
			QString names[] = {
				L("Text"),
				L("Picture"),
				L("Video")
			};
			QString descs[] = {
				L("Text.Desc"),
				L("Picture.Desc"),
				L("Video.Desc")
			};
			for (int i = 0; i < 3; i++) {
				XLSourceToolboxItemWidget *widget = new XLSourceToolboxItemWidget(this,
																				  XLSourcePopupWidget::MODE_TOOLBOX);
				widget->setIndex(i);
				widget->setIcon(icons[i]);
				widget->setName(names[i]);
				widget->setDesc(descs[i]);
				ui->listView->setIndexWidget(model->index(i, 0), widget);
			}
			break;
		}
		case XLSourcePopupWidget::MODE_MONITOR:
			// title
			ui->titleLabel->setText(L("Monitor.Title"));

			// init rows
			for (int i = 0; i < XLUtil::getMonitorCount(); i++) {
				model->appendRow(new QStandardItem());
				XLSourceToolboxItemWidget *widget = new XLSourceToolboxItemWidget(this, XLSourcePopupWidget::MODE_MONITOR);
				widget->setIndex(i);
				widget->setIcon(QPixmap(":/res/images/source_monitor_pic.png"));
				widget->setName(QString("%1%2").arg(L("Monitor")).arg(i + 1));
				widget->setDesc(L("Monitor.Desc").arg(i + 1));
				ui->listView->setIndexWidget(model->index(i, 0), widget);
			}
			break;
	}
}

void XLSourcePopupWidget::hideEvent(QHideEvent *event) {
	// call super
	QWidget::hideEvent(event);

	// delete self
	deleteLater();
}

void XLSourcePopupWidget::showEvent(QShowEvent *event) {
	// call super
	QWidget::showEvent(event);

	// center of ref
	QWidget* ref = m_refLocWidget ? m_refLocWidget : parentWidget();
	QSize refSize = ref->size();
	QPoint refCenter(refSize.width() / 2, refSize.height());
	refCenter = ref->mapToGlobal(refCenter);

	// start x
	OBSBasic* main = dynamic_cast<OBSBasic*>(App()->GetMainWindow());
	QWidget* scenePanel = main->getScenePanel();
	QSize selfSize = size();
	int x = (scenePanel->size().width() - selfSize.width()) / 2;
	x = scenePanel->mapToGlobal(QPoint(x, 0)).x();

	// left margin of arrow
	QSize arrowSize = ui->arrowLabel->size();
	QMargins m = ui->arrowContainer->contentsMargins();
	m.setLeft(refCenter.x() - x - arrowSize.width() / 2);
	ui->arrowContainer->setContentsMargins(m);

	// move
	move(QPoint(x, refCenter.y()));
}

void XLSourcePopupWidget::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);

	// draw with style
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void XLSourcePopupWidget::setReferenceWidget(QWidget* w) {
	m_refLocWidget = w;
}

XLSourcePopupWidgetListDelegate::XLSourcePopupWidgetListDelegate(QObject* parent, XLSourcePopupWidget::Mode mode) :
	QStyledItemDelegate(parent),
	m_mode(mode) {
}

XLSourcePopupWidgetListDelegate::~XLSourcePopupWidgetListDelegate() {
}

QSize XLSourcePopupWidgetListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
	if(m_mode == XLSourcePopupWidget::MODE_APP) {
		return QSize(0, 48);
	} else {
		return QSize(0, 78);
	}
}