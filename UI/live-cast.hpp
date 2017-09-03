#ifndef OBS_STUDIO_LIVE_CAST_HPP_H
#define OBS_STUDIO_LIVE_CAST_HPP_H

#include <obs.hpp>
#include <map>

// live platforms
typedef enum {
    LIVE_PLATFORM_DOUYU = 0,
    LIVE_PLATFORM_PANDA = 1,
    LIVE_PLATFORM_ZHANQI = 2,
    LIVE_PLATFORM_NETEASE = 3,
    LIVE_PLATFORM_HUYA = 4,
    LIVE_PLATFORM_HUAJIAO = 5
} LivePlatform;

// name key of live platform
static const char* LivePlatformNames[] = {
    "Basic.Live.DouYu",
    "Basic.Live.Panda",
    "Basic.Live.ZhanQi",
    "Basic.Live.NetEase",
    "Basic.Live.HuYa",
    "Basic.Live.HuaJiao"
};

// info to push stream
struct live_platform_info {
    char rtmpUrl[256];
    char liveCode[256];
};
typedef struct live_platform_info live_platform_info_t;

// helper to access live platform web site
class LivePlatformWeb {
private:
    LivePlatform m_curPlatform;
    std::map<int, live_platform_info_t> m_infos;

public:
    LivePlatformWeb();
    virtual ~LivePlatformWeb();

    // open web site
    void OpenWeb();

    // set current platform
    inline void SetCurrentPlatform(LivePlatform p) { m_curPlatform = p; }
	inline LivePlatform GetCurrentPlatform() { return m_curPlatform; }

    // platform stream info
    live_platform_info_t& GetCurrentPlatformInfo();
	void SetCurrentPlatformInfo(live_platform_info_t& info);

	// get platform home url
	const char* GetPlatformHomeUrl(LivePlatform p);
};

#endif //OBS_STUDIO_LIVE_CAST_HPP_H
