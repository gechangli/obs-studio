/******************************************************************************
    Copyright (C) 2014 by Luma <stubma@gmail.com>

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

#include "xl-progress-dialog.hpp"
#include <QKeyEvent>
#include <QPropertyAnimation>
#include "xl-util.hpp"

XLProgressDialog::XLProgressDialog(QWidget* parent) :
	QDialog (parent, Qt::FramelessWindowHint),
	ui(new Ui::XLProgressDialog)  {
	// init ui
	ui->setupUi(this);

	// create animation
	m_animation = new QPropertyAnimation(this, "pixmap");
	m_animation->setDuration(1000);
	m_animation->setLoopCount(-1);
	for (int i = 0; i < 13; ++i) {
		m_animation->setKeyValueAt(i / 12.0, i + 1);
	}
	m_animation->setStartValue(1);
	m_animation->setEndValue(12);

	// set style
	QString qssPath = XLUtil::getQssPathByName("xl-progress-dialog");
	QString qss = XLUtil::loadQss(qssPath);
	setStyleSheet(qss);
}

void XLProgressDialog::keyPressEvent(QKeyEvent *event) {
	switch (event->key()) {
		case Qt::Key_Escape:
			break;
		default:
			QDialog::keyPressEvent(event);
			break;
	}
}

void XLProgressDialog::showEvent(QShowEvent *event) {
	QDialog::showEvent(event);
	m_animation->start();
}

void XLProgressDialog::hideEvent(QHideEvent *event) {
	QDialog::hideEvent(event);
	m_animation->stop();
}

int XLProgressDialog::pixmap() const {
	return m_index;
}

void XLProgressDialog::setPixmap(const int index)  {
	// save index
	m_index = index;

	// rotate image
	QPixmap pixmap(QString(":/res/images/anims/loading_%1.png").arg(index));
	ui->loadingLabel->setPixmap(pixmap);
}