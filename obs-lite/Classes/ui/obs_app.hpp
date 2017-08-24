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
    bool do_mkdir(const char *path);
    bool InitGlobalConfig();
    bool InitGlobalConfigDefaults();
    
public:
    // get config file path
    static int GetConfigPath(char* path, size_t size, const char* name);
    
    // ctor & dtor
    OBSApp(profiler_name_store_t *store);
    virtual ~OBSApp();
    
    // startup
    void RegisterStaticModuleLoader(OBS_STATIC_MODULE_LOADER loader);
    bool StartupOBS(const char* locale);
    
    profiler_name_store_t* GetProfilerNameStore() const {
        return profilerNameStore;
    }
};
