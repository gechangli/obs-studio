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

// live platforms
typedef enum {
    LIVE_PLATFORM_DOUYU = 0,
	LIVE_PLATFORM_HUYA = 1,
	LIVE_PLATFORM_PANDA = 2,
//    LIVE_PLATFORM_ZHANQI = 2,
//    LIVE_PLATFORM_NETEASE = 3,
//    LIVE_PLATFORM_HUAJIAO = 5
	LIVE_PLATFORM_LAST = LIVE_PLATFORM_PANDA
} LivePlatform;

// name key of live platform
static const char* LivePlatformNames[] = {
    "Basic.Live.DouYu",
	"Basic.Live.HuYa",
    "Basic.Live.Panda",
    "Basic.Live.ZhanQi",
    "Basic.Live.NetEase",
    "Basic.Live.HuaJiao"
};

// info to push stream
struct live_platform_info {
    char rtmpUrl[256];
    char liveCode[256];
	char username[128];
	char password[128];
};
typedef struct live_platform_info live_platform_info_t;

// declaration
class OBSBasic;
class QWebEngineView;
class XLProgressDialog;

// helper to access live platform web site
class LivePlatformWeb : public QObject {
	Q_OBJECT

private:
    LivePlatform m_curPlatform;
    std::map<int, live_platform_info_t> m_infos;
	OBSBasic* m_main;
	QWebEngineView* m_webView;
	XLProgressDialog* m_progressDialog;
	int m_pageWidth;
	int m_pageHeight;
	bool m_loggedIn;

private:
	QString getJavascriptFileContent(const char* path);

public:
	Q_INVOKABLE int getPageWidth();
	Q_INVOKABLE int getPageHeight();
	Q_PROPERTY(int m_pageWidth READ getPageWidth);
	Q_PROPERTY(int m_pageHeight READ getPageHeight);
	Q_INVOKABLE bool isLoggedIn();
	Q_PROPERTY(bool m_loggedIn READ isLoggedIn);

public slots:
	Q_INVOKABLE void saveLivePlatformRtmpInfo(QString url, QString key);
	Q_INVOKABLE void saveLivePlatformUserInfo(QString username, QString password);
	Q_INVOKABLE void closeWeb();
	Q_INVOKABLE void showMessageBox(QString title, QString msg);
	Q_INVOKABLE void clearCookies();
	Q_INVOKABLE void hideWeb();
	Q_INVOKABLE void showWeb();
	Q_INVOKABLE void jsLog(QString t);

public:
    LivePlatformWeb();
    virtual ~LivePlatformWeb();

    // open web site
    void openWeb(bool clearSession = false);

	// associate main
	void setMain(OBSBasic *m);

    // set current platform
    inline void setCurrentPlatform(LivePlatform p) { m_curPlatform = p; }
	inline LivePlatform getCurrentPlatform() { return m_curPlatform; }

    // platform info
	LivePlatform id2Type(QString id);
	QString type2Id(LivePlatform plt);
    live_platform_info_t& getCurrentPlatformInfo();
	live_platform_info_t& getPlatformInfo(LivePlatform p);
	live_platform_info_t& getPlatformInfo(QString name);
	void setCurrentPlatformInfo(live_platform_info_t &info);

	// get platform home url
	const char* getPlatformHomeUrl(LivePlatform p);
};