#pragma once

#include <stdio.h>
#include <string>
#include <util/util.hpp>

#ifdef __APPLE__
#define BASE_PATH ".."
#else
#define BASE_PATH "../.."
#endif

#define CONFIG_PATH BASE_PATH "/config"

#ifndef OBS_UNIX_STRUCTURE
#define OBS_UNIX_STRUCTURE 0
#endif

/**
 * OBS application
 */
class OBSApp {
private:
    profiler_name_store_t* profilerNameStore = nullptr;
    ConfigFile globalConfig;
    
    bool MakeUserDirs();
    bool MakeUserProfileDirs();
    bool do_mkdir(const char *path);
    std::string GetSceneCollectionFileFromName(const char *name);
    std::string GetProfileDirFromName(const char *name);
    bool InitGlobalConfig();
    bool InitGlobalConfigDefaults();
    
public:
    // get config file path
    static int GetConfigPath(char* path, size_t size, const char* name);
    static void AddExtraModulePaths();
    static int GetProgramDataPath(char *path, size_t size, const char *name);
    static char* GetProgramDataPathPtr(const char *name);
    
    OBSApp(profiler_name_store_t *store);
    virtual ~OBSApp();
    
    void RegisterStaticModuleLoader(OBS_STATIC_MODULE_LOADER loader);
    bool StartupOBS(const char* locale);
    
    profiler_name_store_t* GetProfilerNameStore() const {
        return profilerNameStore;
    }
};
