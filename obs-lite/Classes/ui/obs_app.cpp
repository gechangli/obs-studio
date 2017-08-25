#include <time.h>
#include <stdio.h>
#include <wchar.h>
#include <chrono>
#include <ratio>
#include <string>
#include <sstream>
#include <mutex>
#include <util/bmem.h>
#include <util/dstr.h>
#include <util/platform.h>
#include <util/profiler.hpp>
#include <util/util.hpp>
#include <obs-config.h>
#include <obs.hpp>
#include "obs_app.hpp"
#include <TargetConditionals.h>

using namespace std;

#define DEFAULT_LANG "en_US"
#define SERVICE_PATH "service.json"

bool portable_mode = false;

OBSApp::OBSApp(profiler_name_store_t *store) :
profilerNameStore(store) {
    
}

OBSApp::~OBSApp() {
    
}

int OBSApp::GetConfigPath(char *path, size_t size, const char *name) {
    if (!OBS_UNIX_STRUCTURE && portable_mode) {
        if (name && *name) {
            return snprintf(path, size, CONFIG_PATH "/%s", name);
        } else {
            return snprintf(path, size, CONFIG_PATH);
        }
    } else {
        return os_get_config_path(path, size, name);
    }
}

int OBSApp::GetProfilePath(char *path, size_t size, const char *file) {
    char profiles_path[512];
    const char *profile = config_get_string(globalConfig, "Basic", "ProfileDir");
    int ret;
    
    if (!profile)
        return -1;
    if (!path)
        return -1;
    if (!file)
        file = "";
    
    ret = GetConfigPath(profiles_path, 512, "data/profiles");
    if (ret <= 0)
        return ret;
    
    if (!*file)
        return snprintf(path, size, "%s/%s", profiles_path, profile);
    
    return snprintf(path, size, "%s/%s/%s", profiles_path, profile, file);
}

void OBSApp::RegisterStaticModuleLoader(OBS_STATIC_MODULE_LOADER loader) {
    obs_register_static_module_loader(loader);
}

bool OBSApp::StartupOBS(const char* locale) {
    char path[512];
    
    // init
    if (!MakeUserDirs())
        throw "Failed to create required user directories";
    if (!InitGlobalConfig())
        throw "Failed to initialize global config";
    
    // load config
    if (GetConfigPath(path, sizeof(path), "data/plugin_config") <= 0) {
        return false;
    }
    
    // startup obs
    if(!obs_startup(locale, path, profilerNameStore)) {
        return false;
    }
    
    // init modules
    obs_load_all_modules();
    blog(LOG_INFO, "---------------------------------");
    obs_log_loaded_modules();
    blog(LOG_INFO, "---------------------------------");
    obs_post_load_modules();
    
    // init rtmp services
    if (!InitService())
        throw "Failed to initialize service";
    
    return true;
}

int OBSApp::ResetVideo(int w, int h) {
    struct obs_video_info ovi;
    ovi.adapter         = 0;
    ovi.base_width      = w;
    ovi.base_height     = h;
    ovi.fps_num         = 30000;
    ovi.fps_den         = 1001;
    ovi.graphics_module = config_get_string(globalConfig, "Video", "Renderer");
    ovi.output_format   = VIDEO_FORMAT_RGBA;
    ovi.output_width    = w;
    ovi.output_height   = h;
    
    if (obs_reset_video(&ovi) != 0)
        throw "Couldn't initialize video";
    return 0;
}

bool OBSApp::InitService() {
    ProfileScope("OBSApp::InitService");
    
    // create service from profile
    if (LoadService())
        return true;
    
    // if there is no profile, try to create a common rtmp service to test
    service = obs_service_create("rtmp_common", "default_service", nullptr, nullptr);
    if (!service)
        return false;
    obs_service_release(service);
    
    return true;
}

bool OBSApp::LoadService() {
    const char *type;
    
    // get service profile
    char serviceJsonPath[512];
    int ret = GetProfilePath(serviceJsonPath, sizeof(serviceJsonPath), SERVICE_PATH);
    if (ret <= 0)
        return false;
    
    // if profile exists, load it and test to create service
    if(os_file_exists(serviceJsonPath)) {
        obs_data_t *data = obs_data_create_from_json_file_safe(serviceJsonPath, nullptr);
        
        obs_data_set_default_string(data, "type", "rtmp_common");
        type = obs_data_get_string(data, "type");
        
        obs_data_t *settings = obs_data_get_obj(data, "settings");
        obs_data_t *hotkey_data = obs_data_get_obj(data, "hotkeys");
        
        service = obs_service_create(type, "default_service", settings, hotkey_data);
        obs_service_release(service);
        
        obs_data_release(hotkey_data);
        obs_data_release(settings);
        obs_data_release(data);
    }

    return !!service;
}

bool OBSApp::do_mkdir(const char *path) {
    if (os_mkdirs(path) == MKDIR_ERROR) {
        blog(LOG_ERROR, "Failed to create directory %s", path);
        return false;
    }
    
    return true;
}

bool OBSApp::MakeUserDirs() {
    char path[512];
    
    if (GetConfigPath(path, sizeof(path), "data") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;
    
    if (GetConfigPath(path, sizeof(path), "data/logs") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;
    
    if (GetConfigPath(path, sizeof(path), "data/profiler_data") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;
    
    if (GetConfigPath(path, sizeof(path), "data/profiles") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;
    
#ifdef _WIN32
    if (GetConfigPath(path, sizeof(path), "data/crashes") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;
    
    if (GetConfigPath(path, sizeof(path), "data/updates") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;
#endif
    
    if (GetConfigPath(path, sizeof(path), "data/plugin_config") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;
    
    return true;
}

bool OBSApp::InitGlobalConfig() {
    char path[512];
    bool changed = false;
    
    int len = GetConfigPath(path, sizeof(path), "data/global.ini");
    if (len <= 0) {
        return false;
    }
    
    int errorcode = globalConfig.Open(path, CONFIG_OPEN_ALWAYS);
    if (errorcode != CONFIG_SUCCESS) {
        blog(LOG_ERROR, "Failed to open global.ini: %d", errorcode);
        return false;
    }
    
    // write config file to disk
    if (changed)
        config_save_safe(globalConfig, "tmp", nullptr);
    
    // init default value
    return InitGlobalConfigDefaults();
}

bool OBSApp::InitGlobalConfigDefaults() {
    config_set_default_string(globalConfig, "General", "Language", DEFAULT_LANG);
    config_set_default_string(globalConfig, "General", "ProcessPriority", "Normal");
    config_set_default_string(globalConfig, "Basic", "ProfileDir", "default");
    
#if _WIN32
    config_set_default_string(globalConfig, "Video", "Renderer",
                              "Direct3D 11");
#else
    config_set_default_string(globalConfig, "Video", "Renderer", "OpenGL-static");
#endif

#if TARGET_OS_OSX
    config_set_default_bool(globalConfig, "Video", "DisableOSXVSync", true);
    config_set_default_bool(globalConfig, "Video", "ResetOSXVSyncOnExit",
                            true);
#endif
    return true;
}
