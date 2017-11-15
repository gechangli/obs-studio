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

#include "xl-live-platform.hpp"
#include <QtWebEngineWidgets/QtWebEngineWidgets>
#include "platform.hpp"
#include "window-basic-main.hpp"
#include "xl-progress-dialog.hpp"
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include "xl-util.hpp"
#include "xl-web-dialog.hpp"
#include "xl-progress-dialog.hpp"

using namespace std;

// home page of live platform
static const char* s_homeUrls[] = {
    "https://www.douyu.com/room/my",
	"http://i.huya.com/index.php?m=ProfileSetting",
    "https://www.panda.tv/setting",
//    "http://www.zhanqi.tv/user/follow",
//    "http://i.cc.163.com/",
//    "http://www.huajiao.com/user"
};

// string id of live platform
static const char* s_livePlatformIds[] = {
	"douyu",
	"huya",
	"xiongmao",
	"zhanqi",
	"wangyi",
	"huajiao"
};

// domain keyword for live platform
static const char* s_livePlatformDomains[] = {
	"douyu",
	"huya",
	"panda"
};

LivePlatformWeb::LivePlatformWeb() :
m_curPlatform(LIVE_PLATFORM_DOUYU),
m_webDialog(Q_NULLPTR),
m_delayOpenTimer(Q_NULLPTR),
m_progressDialog(Q_NULLPTR),
m_cookieStore(Q_NULLPTR) {
	// load info from config
	loadLivePlatformInfos();
}

LivePlatformWeb::~LivePlatformWeb() {
}

void LivePlatformWeb::loadLivePlatformInfos() {
	// load platform info, use platform name as section name
	config_t* globalConfig = GetGlobalConfig();
	const char* xgmUser = config_get_string(globalConfig, "XiaomeiLive", "Username");
	if(xgmUser != Q_NULLPTR && strlen(xgmUser) > 0) {
		for(int i = LIVE_PLATFORM_DOUYU; i <= LIVE_PLATFORM_LAST; i++) {
			const char* pltName = s_livePlatformIds[i];
			QString key = QString("%1.%2").arg(xgmUser, pltName);
			const char* pltUser = config_get_string(globalConfig, key.toStdString().c_str(), "Username");
			if(pltUser != Q_NULLPTR) {
				live_platform_info_t& info = getPlatformInfo((LivePlatform)i);
				memcpy(info.username, pltUser, strlen(pltUser));
			}
		}
	}
}

void LivePlatformWeb::saveLivePlatformInfo(LivePlatform plt) {
	live_platform_info_t& info = getPlatformInfo(plt);
	if(strlen(info.username) > 0) {
		config_t* globalConfig = GetGlobalConfig();
		const char* xgmUser = config_get_string(globalConfig, "XiaomeiLive", "Username");
		if(xgmUser != Q_NULLPTR && strlen(xgmUser) > 0) {
			const char *pltName = s_livePlatformIds[plt];
			QString key = QString("%1.%2").arg(xgmUser, pltName);
			config_set_string(globalConfig, key.toStdString().c_str(), "Username", info.username);
			config_save_safe(globalConfig, "tmp", Q_NULLPTR);
		}
	}
}

void LivePlatformWeb::saveLivePlatformInfo(QString pltName) {
	saveLivePlatformInfo(id2Type(pltName));
}

void LivePlatformWeb::hideWeb() {
	if(m_webDialog) {
		m_webDialog->hideWeb();
	}
}

void LivePlatformWeb::showWeb() {
	if(m_webDialog) {
		m_webDialog->showWeb();
	}
}

void LivePlatformWeb::openWeb(bool clearSession) {
	// create web dialog
	m_webDialog = new XLWebDialog();

	// listen cookie event
	if(!m_cookieStore) {
		m_cookieStore = m_webDialog->webView()->page()->profile()->cookieStore();
		connect(m_cookieStore, &QWebEngineCookieStore::cookieAdded, this, &LivePlatformWeb::onCookieAdded);
	}

	// if clear session is set, we need delay to open web because
	// delete cookie is asynchronous
	if(clearSession) {
		clearCookiesAsync();
	} else {
		doOpenWeb();
	}
}

void LivePlatformWeb::doOpenWeb() {
	// set channel
	QWebEngineView* view = m_webDialog->webView();
	QWebEnginePage* page = view->page();
	QWebChannel* channel = new QWebChannel(page);
	page->setWebChannel(channel);

	// register self
	channel->registerObject(QStringLiteral("lp"), this);

	// setup javascript and event
	connect(view, &QWebEngineView::loadFinished, this, &LivePlatformWeb::onLoadFinished);
	connect(view, &QWebEngineView::loadStarted, this, &LivePlatformWeb::onLoadStarted);

	// set title
	m_webDialog->setWindowTitle(L(LivePlatformNames[m_curPlatform]));

	// load
	const QUrl url(getPlatformHomeUrl(m_curPlatform));
	view->load(url);

	// open
	m_webDialog->exec();
}

void LivePlatformWeb::onLoadStarted() {
	hideWeb();
}

void LivePlatformWeb::onLoadFinished(bool ok) {
	QWebEngineView* view = m_webDialog->webView();
	QWebEnginePage* page = view->page();
	QString url = page->url().toString();
	if(url != "about:blank") {
		m_webDialog->autoFit();
		page->runJavaScript(XLUtil::getDataFileContent("js/qwebchannel.js"));
		page->runJavaScript(XLUtil::getDataFileContent("js/util.js"));
		page->runJavaScript(XLUtil::getDataFileContent(QString("js/%1.js").arg(type2Id(m_curPlatform))));
	}
}

void LivePlatformWeb::jsLog(QString t) {
	blog(LOG_INFO, "jsLog: %s", t.toStdString().c_str());
}

void LivePlatformWeb::saveLivePlatformRtmpInfo(QString url, QString key) {
	// save rtmp info
	live_platform_info_t& info = getCurrentPlatformInfo();
	string curl = url.toStdString();
	string ckey = key.toStdString();
	memcpy(info.rtmpUrl, curl.c_str(), curl.length());
	memcpy(info.liveCode, ckey.c_str(), ckey.length());

	// signal
	emit liveRtmpGot(type2Id(m_curPlatform));
}

void LivePlatformWeb::saveLivePlatformUserInfo(QString username, QString password) {
	// save user info
	live_platform_info_t& info = getCurrentPlatformInfo();
	if(username.length() > 0) {
		string cuser = username.toStdString();
		memcpy(info.username, cuser.c_str(), cuser.length());
	}
	if(password.length() > 0) {
		string cpwd = password.toStdString();
		memcpy(info.password, cpwd.c_str(), cpwd.length());
	}

	// save
	saveLivePlatformInfo(m_curPlatform);

	// logged in signal
	emit liveUserLoggedIn(type2Id(m_curPlatform));
}

bool LivePlatformWeb::isLivePlatformUserInfoSaved() {
	live_platform_info_t& info = getCurrentPlatformInfo();
	return strlen(info.username) > 0;
}

bool LivePlatformWeb::isLoggedIn() {
	live_platform_info_t& info = getCurrentPlatformInfo();
	return strlen(info.username) > 0;
}

void LivePlatformWeb::closeWeb() {
	if(m_webDialog) {
		disconnect(m_webDialog->webView());
		m_webDialog->closeWeb();
		m_webDialog = Q_NULLPTR;
	}
}

void LivePlatformWeb::delayOpenWeb() {
	// stop timer
	m_delayOpenTimer->stop();
	m_delayOpenTimer->deleteLater();
	m_delayOpenTimer = Q_NULLPTR;

	// remove progress dialog
	m_progressDialog->hide();
	m_progressDialog->deleteLater();
	m_progressDialog = Q_NULLPTR;

	// open web now
	doOpenWeb();
}

void LivePlatformWeb::clearCookiesAsync() {
	if(m_webDialog) {
		// show progress
		m_progressDialog = new XLProgressDialog();
		m_progressDialog->show();

		// start deleting
		int c = m_cookieList.count();
		for(int i = c - 1; i >= 0; i--) {
			QNetworkCookie& cookie = m_cookieList[i];
			if(cookie.domain().indexOf(s_livePlatformDomains[m_curPlatform]) != -1) {
				blog(LOG_INFO, "cookie %s matched, delete it", QString(cookie.name()).toStdString().c_str());
				m_cookieStore->deleteCookie(cookie);
				m_cookieList.removeAt(i);
			}
		}

		// delay open web
		if(!m_delayOpenTimer) {
			// save time, start timer
			m_delayOpenTimer = new QTimer(this);
			m_delayOpenTimer->setSingleShot(true);
			connect(m_delayOpenTimer, &QTimer::timeout, this, &LivePlatformWeb::delayOpenWeb);
			m_delayOpenTimer->start(3000);
			blog(LOG_INFO, "delay for 3 seconds to open web");
		}
	}
}

void LivePlatformWeb::clearCookies() {
	int c = m_cookieList.count();
	for(int i = c - 1; i >= 0; i--) {
		QNetworkCookie& cookie = m_cookieList[i];
		if(cookie.domain().indexOf(s_livePlatformDomains[m_curPlatform]) != -1) {
			blog(LOG_INFO, "cookie %s matched, delete it", QString(cookie.name()).toStdString().c_str());
			m_cookieStore->deleteCookie(cookie);
			m_cookieList.removeAt(i);
		}
	}
}

void LivePlatformWeb::onCookieAdded(const QNetworkCookie &cookie) {
	blog(LOG_INFO, "cookie %s added, domain: %s", QString(cookie.name()).toStdString().c_str(), cookie.domain().toStdString().c_str());
	m_cookieList.append(cookie);
}

void LivePlatformWeb::showMessageBox(QString title, QString msg) {
	QMessageBox::warning(Q_NULLPTR, title, msg);
}

live_platform_info_t& LivePlatformWeb::getCurrentPlatformInfo() {
	return getPlatformInfo(m_curPlatform);
}

live_platform_info_t& LivePlatformWeb::getPlatformInfo(LivePlatform p) {
	map<int, live_platform_info_t>::iterator itor = m_infos.find(p);
	if(itor == m_infos.end()) {
		m_infos[p] = {
			false,
			"",
			"",
			"",
			""
		};
	}
	return m_infos[p];
}

live_platform_info_t& LivePlatformWeb::getPlatformInfo(QString name) {
	LivePlatform plt = id2Type(name);
	return getPlatformInfo(plt);
}

void LivePlatformWeb::setCurrentPlatformInfo(live_platform_info_t &info) {
	m_infos[m_curPlatform] = info;
}

const char* LivePlatformWeb::getPlatformHomeUrl(LivePlatform p) {
	return s_homeUrls[p];
}

LivePlatform LivePlatformWeb::id2Type(QString id) {
	int len = sizeof(s_livePlatformIds) / sizeof(const char*);
	for(int i = 0; i < len; i++) {
		if(id == s_livePlatformIds[i]) {
			return (LivePlatform)i;
		}
	}
	return LIVE_PLATFORM_DOUYU;
}

QString LivePlatformWeb::type2Id(LivePlatform plt) {
	return s_livePlatformIds[(int)plt];
}