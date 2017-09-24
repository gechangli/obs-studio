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

class QNetworkAccessManager;

// helper to access xiguamei oa backend
class XgmOA : public QObject {
	Q_OBJECT

private:
	QNetworkAccessManager* m_netMgr;

private:
	void doPost(const char* url, const char* json);
	void doGet(const char* url);

public:
	XgmOA();
	virtual ~XgmOA();

public slots:
	void httpFinished();
	void httpReadyRead();
};