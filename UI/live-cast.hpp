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

// string id of live platform
static const char* LivePlatformIds[] = {
	"douyu",
	"huya",
	"xiongmao",
	"zhanqi",
	"wangyi",
	"huajiao"
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
class QProgressDialog;

// helper to access live platform web site
class LivePlatformWeb : public QObject {
	Q_OBJECT

private:
    LivePlatform m_curPlatform;
    std::map<int, live_platform_info_t> m_infos;
	OBSBasic* m_main;
	QWebEngineView* m_webView;
	QProgressDialog* m_progressDialog;
	int m_pageWidth;
	int m_pageHeight;
	bool m_loggedIn;

private:
	QString GetJavascriptFileContent(const char* path);

public:
	Q_INVOKABLE int getPageWidth();
	Q_INVOKABLE int getPageHeight();
	Q_PROPERTY(int m_pageWidth READ getPageWidth);
	Q_PROPERTY(int m_pageHeight READ getPageHeight);
	Q_INVOKABLE bool IsLoggedIn();
	Q_PROPERTY(bool m_loggedIn READ IsLoggedIn);

public slots:
	Q_INVOKABLE void SaveLivePlatformRtmpInfo(QString url, QString key);
	Q_INVOKABLE void SaveLivePlatformUserInfo(QString username, QString password);
	Q_INVOKABLE void CloseWeb();
	Q_INVOKABLE void ShowMessageBox(QString title, QString msg);
	Q_INVOKABLE void ClearCookies();
	Q_INVOKABLE void HideWeb();
	Q_INVOKABLE void ShowWeb();
	Q_INVOKABLE void JSLog(QString t);

public:
    LivePlatformWeb();
    virtual ~LivePlatformWeb();

    // open web site
    void OpenWeb(bool clearSession = false);

	// associate main
	void SetMain(OBSBasic* m);

    // set current platform
    inline void SetCurrentPlatform(LivePlatform p) { m_curPlatform = p; }
	inline LivePlatform GetCurrentPlatform() { return m_curPlatform; }

    // platform stream info
    live_platform_info_t& GetCurrentPlatformInfo();
	live_platform_info_t& GetPlatformInfo(LivePlatform p);
	void SetCurrentPlatformInfo(live_platform_info_t& info);

	// get platform home url
	const char* GetPlatformHomeUrl(LivePlatform p);
};