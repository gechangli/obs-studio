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

using namespace std;

bool portable_mode = false;

OBSApp::OBSApp(profiler_name_store_t *store) :
profilerNameStore(store) {
    
}

OBSApp::~OBSApp() {
    
}

void OBSApp::AddExtraModulePaths() {
    char base_module_dir[512];
#if defined(_WIN32) || defined(__APPLE__)
    int ret = GetProgramDataPath(base_module_dir, sizeof(base_module_dir),
                                 "obs-studio/plugins/%module%");
#else
    int ret = GetConfigPath(base_module_dir, sizeof(base_module_dir),
                            "obs-studio/plugins/%module%");
#endif
    
    if (ret <= 0)
        return;
    
    string path = (char*)base_module_dir;
#if defined(__APPLE__)
    obs_add_module_path((path + "/bin").c_str(), (path + "/data").c_str());
    
    BPtr<char> config_bin = os_get_config_path_ptr("obs-studio/plugins/%module%/bin");
    BPtr<char> config_data = os_get_config_path_ptr("obs-studio/plugins/%module%/data");
    obs_add_module_path(config_bin, config_data);
    
#elif ARCH_BITS == 64
    obs_add_module_path((path + "/bin/64bit").c_str(),
                        (path + "/data").c_str());
#else
    obs_add_module_path((path + "/bin/32bit").c_str(),
                        (path + "/data").c_str());
#endif
}

int OBSApp::GetProgramDataPath(char *path, size_t size, const char *name) {
    return os_get_program_data_path(path, size, name);
}

char* OBSApp::GetProgramDataPathPtr(const char *name) {
    return os_get_program_data_path_ptr(name);
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
    
    // load config
    if (GetConfigPath(path, sizeof(path), "obs-studio/plugin_config") <= 0) {
        return false;
    }
    
    // startup obs
    if(!obs_startup(locale, path, profilerNameStore)) {
        return false;
    }
    
    // init modules
    blog(LOG_INFO, "---------------------------------");
    obs_load_all_modules();
    blog(LOG_INFO, "---------------------------------");
    obs_log_loaded_modules();
    blog(LOG_INFO, "---------------------------------");
    obs_post_load_modules();
    return true;
}
