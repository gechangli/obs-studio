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

#include "xl-live-cast.hpp"
#include <QtWebEngineWidgets/QtWebEngineWidgets>
#include "platform.hpp"
#include "window-basic-main.hpp"
#include "xl-progress-dialog.hpp"
#include <QMessageBox>

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

LivePlatformWeb::LivePlatformWeb() :
m_curPlatform(LIVE_PLATFORM_DOUYU),
m_main(nullptr),
m_webView(nullptr),
m_progressDialog(nullptr),
m_pageWidth(0),
m_pageHeight(0) {
	// load info from config
	loadLivePlatformInfos();
}

LivePlatformWeb::~LivePlatformWeb() {
	if(m_progressDialog) {
		delete m_progressDialog;
	}
}

QString LivePlatformWeb::getJavascriptFileContent(QString path) {
	string jsPath;
	GetDataFilePath(path.toStdString().c_str(), jsPath);
	QFile jsFile(QString::fromStdString(jsPath));
	jsFile.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream in(&jsFile);
	return in.readAll();
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

int LivePlatformWeb::getPageWidth() {
	return m_pageWidth;
}

int LivePlatformWeb::getPageHeight() {
	return m_pageHeight;
}

void LivePlatformWeb::hideWeb() {
	if(m_webView) {
		m_webView->hide();
		m_progressDialog->show();
	}
}

void LivePlatformWeb::showWeb() {
	if(m_webView) {
		m_webView->show();
		m_progressDialog->hide();
	}
}

void LivePlatformWeb::openWeb(bool clearSession) {
	// create web view
	QWebEngineView* view = new QWebEngineView();
	m_webView = view;
	view->setWindowModality(Qt::ApplicationModal);
	view->setAttribute(Qt::WA_DeleteOnClose);

	// clear session
	if(clearSession) {
		clearCookies();
	}

	// set channel
	QWebEnginePage* page = view->page();
	QWebChannel* channel = new QWebChannel(page);
	page->setWebChannel(channel);

	// register self
	channel->registerObject(QStringLiteral("lp"), this);

	// create progress dialog
	if(!m_progressDialog) {
		m_progressDialog = new XLProgressDialog(m_main);
	}
	m_progressDialog->hide();

	// save web view size
	QSize size = view->size();
	m_pageWidth = size.width();
	m_pageHeight = size.height();

	// setup javascript and event
	connect(view, &QWebEngineView::loadFinished, [=](bool ok) {
		UNUSED_PARAMETER(ok);
		page->runJavaScript(getJavascriptFileContent("js/qwebchannel.js"));
		page->runJavaScript(getJavascriptFileContent("js/util.js"));
		QString jsFile = QString("js/%1.js").arg(type2Id(m_curPlatform));
		page->runJavaScript(getJavascriptFileContent(jsFile));
	});
	connect(view, &QWebEngineView::loadStarted, [=]() {
		hideWeb();
	});

	// open home
	const QUrl url(getPlatformHomeUrl(m_curPlatform));
	view->load(url);
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

bool LivePlatformWeb::isLoggedIn() {
	live_platform_info_t& info = getCurrentPlatformInfo();
	return strlen(info.username) > 0;
}

void LivePlatformWeb::closeWeb() {
	if(m_webView) {
		m_webView->close();
		m_webView = nullptr;
		m_progressDialog->hide();
	}
}

void LivePlatformWeb::clearCookies() {
	if(m_webView) {
		m_webView->page()->profile()->cookieStore()->deleteAllCookies();
	}
}

void LivePlatformWeb::showMessageBox(QString title, QString msg) {
	QMessageBox::warning(nullptr, title, msg);
}

void LivePlatformWeb::setMain(OBSBasic *m) {
	m_main = m;
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