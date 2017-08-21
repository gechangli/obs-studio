#pragma once

#include <stdio.h>

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
    
public:
    // get config file path
    static int GetConfigPath(char* path, size_t size, const char* name);
    static void AddExtraModulePaths();
    static int GetProgramDataPath(char *path, size_t size, const char *name);
    static char* GetProgramDataPathPtr(const char *name);
    
    OBSApp(profiler_name_store_t *store);
    virtual ~OBSApp();
    
    bool StartupOBS(const char* locale);
    
    profiler_name_store_t* GetProfilerNameStore() const {
        return profilerNameStore;
    }
};