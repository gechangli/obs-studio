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

#include "xiguamei-oa.hpp"
#include <QNetworkAccessManager>
#include <QNetworkReply>

XgmOA::XgmOA() {
	m_netMgr = new QNetworkAccessManager(this);
}

XgmOA::~XgmOA() {
	delete m_netMgr;
}

void XgmOA::doPost(const char* url, const char* json) {
	QNetworkRequest req;
	req.setUrl(QUrl(url));
	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	QNetworkReply* reply = m_netMgr->post(req, QByteArray(json));
	connect(reply, &QNetworkReply::finished, this, &XgmOA::httpFinished);
	connect(reply, &QIODevice::readyRead, this, &XgmOA::httpReadyRead);
}

void XgmOA::doGet(const char* url) {
	QNetworkRequest req;
	req.setUrl(QUrl(url));
	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	QNetworkReply* reply = m_netMgr->get(req);
	connect(reply, &QNetworkReply::finished, this, &XgmOA::httpFinished);
	connect(reply, &QIODevice::readyRead, this, &XgmOA::httpReadyRead);
}

void XgmOA::httpFinished() {

}

void XgmOA::httpReadyRead() {

}