#include "live-cast.hpp"
#include <QtWebEngineWidgets/QtWebEngineWidgets>
#include "platform.hpp"
#include "window-basic-main.hpp"
#include <QProgressDialog>
#include <QMessageBox>

using namespace std;

// home page of live platform
static const char* s_homeUrls[] = {
    "https://www.douyu.com/room/my",
	"http://i.huya.com/index.php?m=ProfileSetting",
//    "https://www.panda.tv/setting",
//    "http://www.zhanqi.tv/user/follow",
//    "http://i.cc.163.com/",
//    "http://www.huajiao.com/user"
};

LivePlatformWeb::LivePlatformWeb() :
m_curPlatform(LIVE_PLATFORM_DOUYU),
m_main(nullptr),
m_webView(nullptr),
m_progressDialog(nullptr),
m_pageWidth(0),
m_pageHeight(0) {
}

LivePlatformWeb::~LivePlatformWeb() {
	if(m_progressDialog) {
		delete m_progressDialog;
	}
}

QString LivePlatformWeb::GetJavascriptFileContent(const char* path) {
	string jsPath;
	GetDataFilePath(path, jsPath);
	QFile jsFile(QString::fromStdString(jsPath));
	jsFile.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream in(&jsFile);
	return in.readAll();
}

int LivePlatformWeb::getPageWidth() {
	return m_pageWidth;
}

int LivePlatformWeb::getPageHeight() {
	return m_pageHeight;
}

void LivePlatformWeb::HideWeb() {
	if(m_webView) {
		m_webView->hide();
		m_progressDialog->show();
	}
}

void LivePlatformWeb::ShowWeb() {
	if(m_webView) {
		m_webView->show();
		m_progressDialog->hide();
	}
}

void LivePlatformWeb::OpenWeb(bool clearSession) {
	// create web view
	QWebEngineView* view = new QWebEngineView();
	m_webView = view;
	view->setWindowModality(Qt::ApplicationModal);
	view->setAttribute(Qt::WA_DeleteOnClose);

	// clear session
	if(clearSession) {
		ClearCookies();
	}

	// set channel
	QWebChannel* channel = new QWebChannel(view->page());
	view->page()->setWebChannel(channel);

	// register self
	channel->registerObject(QStringLiteral("lp"), this);

	// create progress dialog
	if(!m_progressDialog) {
		m_progressDialog = new QProgressDialog();
		m_progressDialog->setWindowModality(Qt::ApplicationModal);
		m_progressDialog->setWindowTitle("Please wait...");
		m_progressDialog->setRange(0, 0);
		m_progressDialog->setCancelButtonText(QString());
	}
	m_progressDialog->hide();

	// save web view size
	QSize size = view->size();
	m_pageWidth = size.width();
	m_pageHeight = size.height();

	// setup javascript and event
	connect(view, &QWebEngineView::loadFinished, [=](bool ok) {
		UNUSED_PARAMETER(ok);
		view->page()->runJavaScript(GetJavascriptFileContent("js/qwebchannel.js"));
		view->page()->runJavaScript(GetJavascriptFileContent("js/util.js"));
		switch(m_curPlatform) {
			case LIVE_PLATFORM_DOUYU:
				view->page()->runJavaScript(GetJavascriptFileContent("js/douyu.js"));
				break;
			case LIVE_PLATFORM_HUYA:
				view->page()->runJavaScript(GetJavascriptFileContent("js/huya.js"));
				break;
		}
	});
	connect(view, &QWebEngineView::loadStarted, [=]() {
		HideWeb();
	});

	// open home
	const QUrl url(GetPlatformHomeUrl(m_curPlatform));
	view->load(url);
}

void LivePlatformWeb::SaveLivePlatformInfo(QString url, QString key, QString username) {
	// save live info
	live_platform_info_t& info = GetCurrentPlatformInfo();
	string curl = url.toStdString();
	string ckey = key.toStdString();
	memcpy(info.rtmpUrl, curl.c_str(), curl.length());
	memcpy(info.liveCode, ckey.c_str(), ckey.length());

	// show account name
	m_main->SetLivePlatformState(m_curPlatform, username);
	m_main->UpdateLivePlatformHint();
}

void LivePlatformWeb::CloseWeb() {
	if(m_webView) {
		m_webView->close();
		m_webView = nullptr;
		m_progressDialog->hide();
	}
}

void LivePlatformWeb::ClearCookies() {
	if(m_webView) {
		m_webView->page()->profile()->cookieStore()->deleteAllCookies();
	}
}

void LivePlatformWeb::ShowMessageBox(QString title, QString msg) {
	QMessageBox::warning(nullptr, title, msg);
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