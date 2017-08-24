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
    return true;
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
    config_set_default_string(globalConfig, "General", "Language",
                              DEFAULT_LANG);
    config_set_default_string(globalConfig, "General", "ProcessPriority",
                              "Normal");
    
#if _WIN32
    config_set_default_string(globalConfig, "Video", "Renderer",
                              "Direct3D 11");
#else
    config_set_default_string(globalConfig, "Video", "Renderer", "OpenGL");
#endif
    
    config_set_default_bool(globalConfig, "BasicWindow",
                            "RecordWhenStreaming", false);
    config_set_default_bool(globalConfig, "BasicWindow",
                            "KeepRecordingWhenStreamStops", false);
    
#ifdef _WIN32
    config_set_default_bool(globalConfig, "Audio", "DisableAudioDucking",
                            true);
#endif
    
#if TARGET_OS_OSX
    config_set_default_bool(globalConfig, "Video", "DisableOSXVSync", true);
    config_set_default_bool(globalConfig, "Video", "ResetOSXVSyncOnExit",
                            true);
#endif
    return true;
}
