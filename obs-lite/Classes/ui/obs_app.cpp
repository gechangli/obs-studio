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
string opt_starting_collection;
string opt_starting_profile;
string opt_starting_scene;

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
    
    // init
    if (!MakeUserDirs())
        throw "Failed to create required user directories";
    if (!InitGlobalConfig())
        throw "Failed to initialize global config";
    
    // set default values
    config_set_default_string(globalConfig, "Basic", "Profile", "Untitled");
    config_set_default_string(globalConfig, "Basic", "ProfileDir", "Untitled");
    config_set_default_string(globalConfig, "Basic", "SceneCollection", "Untitled");
    config_set_default_string(globalConfig, "Basic", "SceneCollectionFile", "Untitled");
    
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

string OBSApp::GetProfileDirFromName(const char *name) {
    string outputPath;
    os_glob_t *glob;
    char path[512];
    
    if (GetConfigPath(path, sizeof(path), "data/profiles") <= 0)
        return outputPath;
    
    strcat(path, "/*");
    
    if (os_glob(path, 0, &glob) != 0)
        return outputPath;
    
    for (size_t i = 0; i < glob->gl_pathc; i++) {
        struct os_globent ent = glob->gl_pathv[i];
        if (!ent.directory)
            continue;
        
        strcpy(path, ent.path);
        strcat(path, "/basic.ini");
        
        ConfigFile config;
        if (config.Open(path, CONFIG_OPEN_EXISTING) != 0)
            continue;
        
        const char *curName = config_get_string(config, "General", "Name");
        if (astrcmpi(curName, name) == 0) {
            outputPath = ent.path;
            break;
        }
    }
    
    os_globfree(glob);
    
    if (!outputPath.empty()) {
        replace(outputPath.begin(), outputPath.end(), '\\', '/');
        const char *start = strrchr(outputPath.c_str(), '/');
        if (start)
            outputPath.erase(0, start - outputPath.c_str() + 1);
    }
    
    return outputPath;
}

string OBSApp::GetSceneCollectionFileFromName(const char *name) {
    string outputPath;
    os_glob_t *glob;
    char path[512];
    
    if (GetConfigPath(path, sizeof(path), "data/scenes") <= 0)
        return outputPath;
    
    strcat(path, "/*.json");
    
    if (os_glob(path, 0, &glob) != 0)
        return outputPath;
    
    for (size_t i = 0; i < glob->gl_pathc; i++) {
        struct os_globent ent = glob->gl_pathv[i];
        if (ent.directory)
            continue;
        
        obs_data_t *data =
        obs_data_create_from_json_file_safe(ent.path, "bak");
        const char *curName = obs_data_get_string(data, "name");
        
        if (astrcmpi(name, curName) == 0) {
            outputPath = ent.path;
            obs_data_release(data);
            break;
        }
        
        obs_data_release(data);
    }
    
    os_globfree(glob);
    
    if (!outputPath.empty()) {
        outputPath.resize(outputPath.size() - 5);
        replace(outputPath.begin(), outputPath.end(), '\\', '/');
        const char *start = strrchr(outputPath.c_str(), '/');
        if (start)
            outputPath.erase(0, start - outputPath.c_str() + 1);
    }
    
    return outputPath;
}

bool OBSApp::do_mkdir(const char *path) {
    if (os_mkdirs(path) == MKDIR_ERROR) {
        blog(LOG_ERROR, "Failed to create directory %s", path);
        return false;
    }
    
    return true;
}

bool OBSApp::MakeUserProfileDirs() {
    char path[512];
    
    if (GetConfigPath(path, sizeof(path), "obs-studio/basic/profiles") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;
    
    if (GetConfigPath(path, sizeof(path), "obs-studio/basic/scenes") <= 0)
        return false;
    if (!do_mkdir(path))
        return false;
    
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
    
    if (!opt_starting_collection.empty()) {
        string path = GetSceneCollectionFileFromName(opt_starting_collection.c_str());
        if (!path.empty()) {
            config_set_string(globalConfig,
                              "Basic", "SceneCollection",
                              opt_starting_collection.c_str());
            config_set_string(globalConfig,
                              "Basic", "SceneCollectionFile",
                              path.c_str());
            changed = true;
        }
    }
    
    if (!opt_starting_profile.empty()) {
        string path = GetProfileDirFromName(opt_starting_profile.c_str());
        if (!path.empty()) {
            config_set_string(globalConfig, "Basic", "Profile",
                              opt_starting_profile.c_str());
            config_set_string(globalConfig, "Basic", "ProfileDir",
                              path.c_str());
            changed = true;
        }
    }
    
    if (!config_has_user_value(globalConfig, "General", "Pre19Defaults")) {
        uint32_t lastVersion = config_get_int(globalConfig, "General",
                                              "LastVersion");
        bool useOldDefaults = lastVersion &&
        lastVersion < MAKE_SEMANTIC_VERSION(19, 0, 0);
        
        config_set_bool(globalConfig, "General", "Pre19Defaults",
                        useOldDefaults);
        changed = true;
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
    config_set_default_uint(globalConfig, "General", "MaxLogs", 10);
    config_set_default_string(globalConfig, "General", "ProcessPriority",
                              "Normal");
    config_set_default_bool(globalConfig, "General", "EnableAutoUpdates",
                            true);
    
#if _WIN32
    config_set_default_string(globalConfig, "Video", "Renderer",
                              "Direct3D 11");
#else
    config_set_default_string(globalConfig, "Video", "Renderer", "OpenGL");
#endif
    
    config_set_default_bool(globalConfig, "BasicWindow", "PreviewEnabled",
                            true);
    config_set_default_bool(globalConfig, "BasicWindow",
                            "PreviewProgramMode", false);
    config_set_default_bool(globalConfig, "BasicWindow",
                            "SceneDuplicationMode", true);
    config_set_default_bool(globalConfig, "BasicWindow",
                            "SwapScenesMode", true);
    config_set_default_bool(globalConfig, "BasicWindow",
                            "SnappingEnabled", true);
    config_set_default_bool(globalConfig, "BasicWindow",
                            "ScreenSnapping", true);
    config_set_default_bool(globalConfig, "BasicWindow",
                            "SourceSnapping", true);
    config_set_default_bool(globalConfig, "BasicWindow",
                            "CenterSnapping", false);
    config_set_default_double(globalConfig, "BasicWindow",
                              "SnapDistance", 10.0);
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
