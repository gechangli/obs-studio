#include "live-cast.hpp"
#include <QtWebEngineWidgets/QtWebEngineWidgets>
#include "platform.hpp"

using namespace std;

// home page of live platform
static const char* s_homeUrls[] = {
    "https://www.douyu.com/room/my",
    "https://www.panda.tv/setting",
    "http://www.zhanqi.tv/user/follow",
    "http://i.cc.163.com/",
    "http://i.huya.com/index.php",
    "http://www.huajiao.com/user"
};

LivePlatformWeb::LivePlatformWeb() :
m_curPlatform(LIVE_PLATFORM_DOUYU),
m_main(nullptr) {
}

LivePlatformWeb::~LivePlatformWeb() {

}

QString LivePlatformWeb::GetJavascriptFileContent(const char* path) {
	string jsPath;
	GetDataFilePath(path, jsPath);
	QFile jsFile(QString::fromStdString(jsPath));
	jsFile.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream in(&jsFile);
	return in.readAll();
}

void LivePlatformWeb::OpenWeb() {
	// create web view and set channel
	QWebEngineView* view = new QWebEngineView();
	QWebChannel* channel = new QWebChannel(view->page());
	view->page()->setWebChannel(channel);

	// register self
	channel->registerObject(QStringLiteral("lp"), this);

	// setup javascript and event
	connect(view, &QWebEngineView::loadFinished, [=](bool ok) {
		view->page()->runJavaScript(GetJavascriptFileContent("js/qwebchannel.js"));
		if(m_curPlatform == LIVE_PLATFORM_DOUYU) {
			view->page()->runJavaScript(GetJavascriptFileContent("js/douyu.js"));
		}
		view->show();
	});
	connect(view, &QWebEngineView::loadStarted, [=]() {
		view->hide();
	});

	// open home
	const QUrl url(GetPlatformHomeUrl(m_curPlatform));
	view->load(url);
}

void LivePlatformWeb::GrabLivePlatformInfo(const QString& url, const QString& key) {
	live_platform_info_t& info = GetCurrentPlatformInfo();
}

void LivePlatformWeb::SetMain(OBSBasic* m) {
	m_main = m;
}

live_platform_info_t& LivePlatformWeb::GetCurrentPlatformInfo() {
	return GetPlatformInfo(m_curPlatform);
}

live_platform_info_t& LivePlatformWeb::GetPlatformInfo(LivePlatform p) {
	map<int, live_platform_info_t>::iterator itor = m_infos.find(p);
	if(itor == m_infos.end()) {
		m_infos[p] = {
			"",
			""
		};
	}
	return m_infos[p];
}

void LivePlatformWeb::SetCurrentPlatformInfo(live_platform_info_t& info) {
	m_infos[m_curPlatform] = info;
}

const char* LivePlatformWeb::GetPlatformHomeUrl(LivePlatform p) {
	return s_homeUrls[p];
}