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

#include "xl-web-dialog.hpp"
#include <QKeyEvent>
#include "xl-progress-dialog.hpp"
#include "window-basic-main.hpp"
#include <QDesktopWidget>

XLWebDialog::XLWebDialog(QWidget* parent) :
	QDialog (parent),
	ui(new Ui::XLWebDialog),
	m_progressDialog(Q_NULLPTR) {
	// init ui
	ui->setupUi(this);
}

XLWebDialog::~XLWebDialog() {
	if(m_progressDialog) {
		delete m_progressDialog;
		m_progressDialog = Q_NULLPTR;
	}
}

void XLWebDialog::keyPressEvent(QKeyEvent *event) {
	switch (event->key()) {
		case Qt::Key_Escape:
			break;
		default:
			QDialog::keyPressEvent(event);
			break;
	}
}

void XLWebDialog::showProgressDialog() {
	// create progress dialog
	if(!m_progressDialog) {
		m_progressDialog = new XLProgressDialog();
	}
	m_progressDialog->show();
}

void XLWebDialog::hideProgressDialog() {
	if(m_progressDialog) {
		m_progressDialog->hide();
	}
}

void XLWebDialog::openNormal(QUrl initUrl, QString title, QSize winSize) {
	// set it hidden when page is loading
	m_autoSize = winSize.isEmpty();
	connect(ui->webView, &QWebEngineView::loadStarted, [=]() {
		hide();
		showProgressDialog();
	});
	connect(ui->webView, &QWebEngineView::loadFinished, [=](bool ok) {
		QString url = ui->webView->page()->url().toString();
		if(url != "about:blank") {
			hideProgressDialog();
			if(m_autoSize) {
				ui->webView->page()->runJavaScript("document.documentElement.scrollWidth;", [=](QVariant result){
					int newWidth = result.toInt()+10;
					QRect r = geometry();
					r.setWidth(newWidth);
					QRect screenGeometry = QApplication::desktop()->screenGeometry();
					r.setX((screenGeometry.width() - r.width()) / 2);
					setGeometry(r);
				});
				ui->webView->page()->runJavaScript("document.documentElement.scrollHeight;", [=](QVariant result){
					int newHeight = result.toInt();
					QRect r = geometry();
					r.setHeight(newHeight);
					setGeometry(r);
				});
			}
			show();
		}
	});

	// resize
	if(!m_autoSize) {
		resize(winSize);
	}

	// load
	ui->webView->load(initUrl);

	// show
	exec();
}