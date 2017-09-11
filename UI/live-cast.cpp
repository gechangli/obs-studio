#include "live-cast.hpp"

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
	m_curPlatform(LIVE_PLATFORM_DOUYU) {
}

LivePlatformWeb::~LivePlatformWeb() {

}

double LivePlatformWeb::getInt() {
	return myInt;
}

void LivePlatformWeb::setInt(double v) {
	myInt = v;
}

void LivePlatformWeb::OpenWeb() {

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