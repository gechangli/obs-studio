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

#include <obs.hpp>
#include <map>
#include <QObject>
#include <QVariant>
#include <QSize>
#include <string>
#include <QJsonDocument>
#include <QNetworkReply>

class QNetworkAccessManager;

// helper to access xiguamei oa backend
class XgmOA : public QObject {
	Q_OBJECT

public:
	// rest api id
	typedef enum {
		OP_INVALID,
		OP_GET_AUTO_CODE,
		OP_REGISTER,
		OP_LOGIN_BY_PASSWORD,
		OP_LOGIN_BY_AUTHCODE,
		OP_LOGOUT,
		OP_GET_LIVE_PLATFORM_ACCOUNTS,
		OP_ADD_LIVE_PLATFORM_ACCOUNT
	} XgmRestOp;

private:
	QNetworkAccessManager* m_netMgr;
	QMap<QNetworkReply*, QJsonDocument> m_respMap;

public:
	static XgmRestOp urlToOp(QUrl& url);

private:
	void doPost(std::string url, QByteArray json);
	void doGet(std::string url);

public:
	XgmOA();
	virtual ~XgmOA();

	// rest api
	void registerUser(std::string acc, std::string pwd, std::string authCode);
	void getAuthCode(std::string phone);
	void loginByPassword(std::string acc, std::string pwd);
	void loginByAuthCode(std::string acc, std::string authCode);
	void logout();
	void getLivePlatformUsers();
	void addLivePlatformUser(std::string pltName, std::string acc);

public slots:
	void httpFinished();
	void httpReadyRead();

signals:
	void restOpDone(XgmRestOp op, QJsonDocument doc);
	void restOpFailed(XgmRestOp op, QNetworkReply::NetworkError errNo, QString errMsg);
};