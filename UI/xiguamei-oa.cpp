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

using namespace std;

// endpoints
#define TEST_URL string("http://101.201.253.175:8000")
#define PRODUCT_URL string("http://oa.xiguamei.com")
#define SERVER_URL TEST_URL
#define PATH_GET_AUTH_CODE "/v1/user/live/authcode"
#define PATH_REGISTER "/v1/user/live/register"
#define PATH_LOGIN_BY_PASSWORD "/v1/user/live/login/passwd"
#define PATH_LOGIN_BY_AUTHCODE "/v1/user/live/login/authcode"
#define PATH_LOGOUT "/v1/user/live/logout"
#define PATH_GET_LIVE_PLATFORM_ACCOUNTS "/v1/user/live/account/list"
#define PATH_ADD_LIVE_PLATFORM_ACCOUNT "/v1/user/live/account/upload"

XgmOA::XgmOA() {
	m_netMgr = new QNetworkAccessManager(this);
}

XgmOA::~XgmOA() {
	delete m_netMgr;
}

XgmOA::XgmRestOp XgmOA::urlToOp(QUrl& url) {
	QString path = url.path();
	if(path == PATH_GET_AUTH_CODE) {
		return OP_GET_AUTO_CODE;
	} else if(path == PATH_REGISTER) {
		return OP_REGISTER;
	} else if(path == PATH_LOGIN_BY_PASSWORD) {
		return OP_LOGIN_BY_PASSWORD;
	} else if(path == PATH_LOGIN_BY_AUTHCODE) {
		return OP_LOGIN_BY_AUTHCODE;
	} else if(path == PATH_LOGOUT) {
		return OP_LOGOUT;
	} else if(path == PATH_GET_LIVE_PLATFORM_ACCOUNTS) {
		return OP_GET_LIVE_PLATFORM_ACCOUNTS;
	} else if(path == PATH_ADD_LIVE_PLATFORM_ACCOUNT) {
		return OP_ADD_LIVE_PLATFORM_ACCOUNT;
	} else {
		return OP_INVALID;
	}
}

void XgmOA::doPost(string url, QByteArray json) {
	QNetworkRequest req;
	req.setUrl(QUrl(url.c_str()));
	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	QNetworkReply* reply = m_netMgr->post(req, json);
	connect(reply, &QNetworkReply::finished, this, &XgmOA::httpFinished);
	connect(reply, &QIODevice::readyRead, this, &XgmOA::httpReadyRead);
}

void XgmOA::doGet(string url) {
	QNetworkRequest req;
	req.setUrl(QUrl(url.c_str()));
	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	QNetworkReply* reply = m_netMgr->get(req);
	connect(reply, &QNetworkReply::finished, this, &XgmOA::httpFinished);
	connect(reply, &QIODevice::readyRead, this, &XgmOA::httpReadyRead);
}

void XgmOA::httpFinished() {
	// get reply and operation id
	QNetworkReply* reply = (QNetworkReply*)QObject::sender();
	QNetworkRequest req = reply->request();
	QUrl url = req.url();
	XgmRestOp op = urlToOp(url);

	// check error, trigger
	if(reply->error() == QNetworkReply::NoError) {
		if(m_respMap.find(reply) != m_respMap.end()) {
			emit restOpDone(op, m_respMap[reply]);
		} else {
			emit restOpDone(op, QJsonDocument());
		}
	} else {
		emit restOpFailed(op, reply->error(), reply->errorString());
	}
}

void XgmOA::httpReadyRead() {
	// read response body
	QNetworkReply* reply = (QNetworkReply*)QObject::sender();
	QByteArray body = reply->readAll();

	// create json from body and put to map
	QJsonDocument doc = QJsonDocument::fromJson(body);
	m_respMap.insert(reply, doc);
}

void XgmOA::GetAuthCode(string phone) {
	// create json data
	QVariantMap map;
	map["telephone"] = QVariant(phone.c_str());
	QJsonDocument doc = QJsonDocument::fromVariant(QVariant(map));
	QByteArray json = doc.toJson(QJsonDocument::Compact);

	// post it
	doPost(SERVER_URL + PATH_GET_AUTH_CODE, json);
}

void XgmOA::Register(string acc, string pwd, string authCode) {
	// create json data
	QVariantMap map;
	map["account"] = QVariant(acc.c_str());
	map["passwd"] = QVariant(pwd.c_str());
	map["authcode"] = QVariant(authCode.c_str());
	QJsonDocument doc = QJsonDocument::fromVariant(QVariant(map));
	QByteArray json = doc.toJson(QJsonDocument::Compact);

	// post it
	doPost(SERVER_URL + PATH_REGISTER, json);
}