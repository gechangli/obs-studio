#ifndef OBS_STUDIO_LIVE_CAST_HPP_H
#define OBS_STUDIO_LIVE_CAST_HPP_H

#include <obs.hpp>
#include <map>
#include <QObject>
#include <QVariant>
#include <QSize>

// live platforms
typedef enum {
    LIVE_PLATFORM_DOUYU = 0,
	LIVE_PLATFORM_HUYA = 1,
//    LIVE_PLATFORM_ZHANQI = 2,
//    LIVE_PLATFORM_NETEASE = 3,
//	LIVE_PLATFORM_PANDA = 4,
//    LIVE_PLATFORM_HUAJIAO = 5
	LIVE_PLATFORM_LAST = LIVE_PLATFORM_HUYA
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

private:
	QString GetJavascriptFileContent(const char* path);

public:
	Q_INVOKABLE int getPageWidth();
	Q_INVOKABLE int getPageHeight();
	Q_PROPERTY(int m_pageWidth READ getPageWidth);
	Q_PROPERTY(int m_pageHeight READ getPageHeight);

public slots:
	Q_INVOKABLE void SaveLivePlatformInfo(QString url, QString key, QString username);
	Q_INVOKABLE void CloseWeb();
	Q_INVOKABLE void ShowMessageBox(QString title, QString msg);
	Q_INVOKABLE void ClearCookies();
	Q_INVOKABLE void HideWeb();
	Q_INVOKABLE void ShowWeb();

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

#endif //OBS_STUDIO_LIVE_CAST_HPP_H
