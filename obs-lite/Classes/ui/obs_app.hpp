#pragma once

#include "obs.hpp"
#include <util/util.hpp>
#include <util/profiler.h>

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
    // size of video
    int videoWidth;
    int videoHeight;
    
    // profiler store
    profiler_name_store_t* profilerNameStore = nullptr;
    
    // global config file
    ConfigFile globalConfig;
    
    // rtmp service
    OBSService service;
    
    // current scene
    OBSSource curScene;
    
    // fade transition
    obs_source_t *fadeTransition;
    
    // buffer used for renderring
    gs_vertbuffer_t *box = nullptr;
    gs_vertbuffer_t *boxLeft = nullptr;
    gs_vertbuffer_t *boxTop = nullptr;
    gs_vertbuffer_t *boxRight = nullptr;
    gs_vertbuffer_t *boxBottom = nullptr;
    gs_vertbuffer_t *circle = nullptr;
    
    // config related
    bool MakeUserDirs();
    bool do_mkdir(const char *path);
    bool InitGlobalConfig();
    bool InitGlobalConfigDefaults();
    bool InitService();
    bool LoadService();
    void InitPrimitives();
    void ClearSceneData();
    void InitDefaultTransitions();
    void InitTransition(obs_source_t *transition);
    static void SourceLoaded(void *data, obs_source_t *source);
    
public:
    // get config file path
    int GetConfigPath(char* path, size_t size, const char* name);
    int GetProfilePath(char *path, size_t size, const char *file);
    
    // ctor & dtor
    OBSApp(int w, int h, profiler_name_store_t *store = nullptr);
    virtual ~OBSApp();
    
    // startup
    void RegisterStaticModuleLoader(OBS_STATIC_MODULE_LOADER loader);
    bool StartupOBS(const char* locale);
    
    // reset
    int ResetVideo();
    
    // scene
    void LoadDefaultScene();
    void LoadScene(const char* file);
    void SetCurrentScene(OBSSource* s);

    profiler_name_store_t* GetProfilerNameStore() const {
        return profilerNameStore;
    }
};
