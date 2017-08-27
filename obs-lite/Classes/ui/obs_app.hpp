#pragma once

#include "obs.hpp"
#include <util/util.hpp>
#include <util/profiler.h>
#include <vector>

#ifdef __APPLE__
#define BASE_PATH ".."
#else
#define BASE_PATH "../.."
#endif

#define CONFIG_PATH BASE_PATH "/config"

#ifndef OBS_UNIX_STRUCTURE
#define OBS_UNIX_STRUCTURE 0
#endif

#define DESKTOP_AUDIO_1 Str("DesktopAudioDevice1")
#define DESKTOP_AUDIO_2 Str("DesktopAudioDevice2")
#define AUX_AUDIO_1     Str("AuxAudioDevice1")
#define AUX_AUDIO_2     Str("AuxAudioDevice2")
#define AUX_AUDIO_3     Str("AuxAudioDevice3")

/**
 * OBS application
 */
class OBSApp {
private:
    // size of video, render scale
    int viewWidth;
    int viewHeight;
    float videoScale;
    
    // profiler store
    profiler_name_store_t* profilerNameStore = nullptr;
    
    // global config file
    ConfigFile globalConfig;
    
    // rtmp service
    OBSService service;
    
    // current scene
    OBSSource curScene;
    
    // display
    OBSDisplay display;
    
    // fade transition
    obs_source_t *fadeTransition;
    
    // transitions
    std::vector<OBSSource> transitions;
    
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
    obs_source_t* FindTransition(const char *name);
    void LoadAudioDevice(const char *name, int channel, obs_data_t *parent);
    static void SourceLoaded(void *data, obs_source_t *source);
    static void RenderMain(void *data, uint32_t cx, uint32_t cy);
    
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

    // display
    void CreateDisplay(gs_window window);
    obs_display_t* GetDisplay();
    
    // accessor
    inline float GetVideoScale() { return videoScale; }
    inline void SetVideoScale(float v) { videoScale = v; }
    inline int GetViewWidth() { return viewWidth; }
    inline int GetViewHeight() { return viewHeight; }
    
    profiler_name_store_t* GetProfilerNameStore() const {
        return profilerNameStore;
    }
};
