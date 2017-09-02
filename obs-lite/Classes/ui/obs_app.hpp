#pragma once

#include "obs.hpp"
#include <util/util.hpp>
#include <util/profiler.h>
#include <vector>
#include <string>
#include <map>

#ifdef __APPLE__
#define BASE_PATH ".."
#else
#define BASE_PATH "../.."
#endif

#define CONFIG_PATH BASE_PATH "/config"

#ifndef OBS_UNIX_STRUCTURE
#define OBS_UNIX_STRUCTURE 0
#endif

// audio device name
#define DESKTOP_AUDIO_1 "DesktopAudioDevice1"
#define DESKTOP_AUDIO_2 "DesktopAudioDevice2"
#define AUX_AUDIO_1     "AuxAudioDevice1"
#define AUX_AUDIO_2     "AuxAudioDevice2"
#define AUX_AUDIO_3     "AuxAudioDevice3"

// encoder
#define SIMPLE_ENCODER_X264                    "x264"
#define SIMPLE_ENCODER_X264_LOWCPU             "x264_lowcpu"
#define SIMPLE_ENCODER_QSV                     "qsv"
#define SIMPLE_ENCODER_NVENC                   "nvenc"
#define SIMPLE_ENCODER_AMD                     "amd"

/**
 * OBS application
 */
class OBSApp {
private:
    // size of video, render scale
    int m_viewWidth;
    int m_viewHeight;
    int m_baseWidth;
    int m_baseHeight;
    float m_videoScale;
    
    // global config file
    ConfigFile m_globalConfig;
    
    // rtmp service
    OBSService m_rtmpService;
    
    // current scene
    OBSSource m_curScene;
    
    // display
    OBSDisplay m_display;
    
    // fade transition
    obs_source_t* m_fadeTransition;
    
    // transitions
    std::vector<OBSSource> m_transitions;
    
    // streaming
    bool m_streamingActive;
    OBSEncoder m_aacStreaming;
    OBSEncoder m_h264Streaming;
    std::string m_aacStreamEncID;
    OBSOutput m_streamOutput;
    OBSSignal m_signalStreamStarted;
    OBSSignal m_signalStreamStopped;
    OBSSignal m_signalStreamStarting;
    OBSSignal m_signalStreamStopping;
    void CreateH264Encoder();
    void CreateH264Encoder(const char *encoderId);
    bool CreateAACEncoder(OBSEncoder &res, std::string &id, int bitrate, const char *name, size_t idx);
    const char* FindAudioEncoderFromCodec(const char *type);
    const char* GetAACEncoderForBitrate(int bitrate);
    int GetAudioBitrate();
    int FindClosestAvailableAACBitrate(int bitrate);
    const std::map<int, const char*>& GetAACEncoderBitrateMap();
    void PopulateBitrateMap();
    static void HandleEncoderProperties(const char *id);
    static void HandleSampleRate(obs_property_t* prop, const char *id);
    static void HandleIntProperty(obs_property_t *prop, const char *id);
    static void HandleListProperty(obs_property_t *prop, const char *id);
    void SetupOutputs();
    
    // config related
    bool MakeUserDirs();
    bool do_mkdir(const char *path);
    bool InitGlobalConfig();
    bool InitGlobalConfigDefaults();
    obs_service_t* LoadCustomService(obs_data_t* settings);
    void ClearSceneData();
    void InitDefaultTransitions();
    void InitTransition(obs_source_t *transition);
    obs_source_t* FindTransition(const char *name);
    void LoadAudioDevice(const char *name, int channel, obs_data_t *parent);
    static void SourceLoaded(void *data, obs_source_t *source);
    static void RenderMain(void *data, uint32_t cx, uint32_t cy);
    
    // reset
    int ResetVideo();
    bool ResetAudio();
    
    // helper
    static const char* NullToEmpty(const char *str);
    static const char* EncoderName(const char *id);
    static const char* GetCodec(const char *id);
    
public:
    // get config file path
    int GetConfigPath(char* path, size_t size, const char* name);
    int GetProfilePath(char *path, size_t size, const char *file);
    
    // ctor & dtor
    OBSApp(int m_baseWidth, int m_baseHeight, int w, int h);
    virtual ~OBSApp();
    static OBSApp* sharedApp();
    
    // startup
    void RegisterStaticModuleLoader(OBS_STATIC_MODULE_LOADER loader);
    bool StartupOBS(const char* locale);
    
    // scene
    void LoadDefaultScene();
    void LoadScene(const char* file);
    void SetCurrentScene(OBSSource* s);

    // display
    void CreateDisplay(gs_window window);
    obs_display_t* GetDisplay();
    
    // streaming
    bool StartStreaming(const char* url, const char* key);
    
    // accessor
    inline float GetVideoScale() { return m_videoScale; }
    inline void SetVideoScale(float v) { m_videoScale = v; }
    inline int GetViewWidth() { return m_viewWidth; }
    inline int GetViewHeight() { return m_viewHeight; }
    inline ConfigFile& GetGlobalConfig() { return m_globalConfig; }
};
