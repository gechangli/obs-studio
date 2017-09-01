#include <time.h>
#include <stdio.h>
#include <wchar.h>
#include <chrono>
#include <ratio>
#include <string>
#include <sstream>
#include <mutex>
#include <iomanip>
#include <util/bmem.h>
#include <util/dstr.h>
#include <util/platform.h>
#include <util/profiler.hpp>
#include <util/util.hpp>
#include <obs-config.h>
#include <obs.hpp>
#include "obs-internal.h"
#include "obs_app.hpp"
#include <TargetConditionals.h>

using namespace std;

#define DEFAULT_LANG "en_US"
#define INVALID_BITRATE 10000

// shared instance
static OBSApp* sharedInstance;

// audio encoder supported bitrates
static map<int, const char*> bitrateMap;
static once_flag populateBitrateMap;

// audio encoder available
static const string aac_ = "AAC";
static const string encoders[] = {
//    "ffmpeg_aac",
//    "mf_aac",
//    "libfdk_aac",
    "CoreAudio_AAC",
};
static const string &fallbackEncoder = encoders[0];

OBSApp::OBSApp(int baseWidth, int baseHeight, int w, int h, profiler_name_store_t *store) :
baseWidth(baseWidth),
baseHeight(baseHeight),
viewWidth(w),
viewHeight(h),
videoScale(0),
streamingActive(false),
profilerNameStore(store) {
    sharedInstance = this;
}

OBSApp::~OBSApp() {
    sharedInstance = nullptr;
}

OBSApp* OBSApp::sharedApp() {
    return sharedInstance;
}

int OBSApp::GetConfigPath(char *path, size_t size, const char *name) {
    return os_get_config_path(path, size, name);
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
    
    // reset audio
    if(!ResetAudio())
        throw "Failed to initialize audio";
    
    // reset video
    ResetVideo();
    
    // init graphics
    InitPrimitives();
    
    // create encoder
    CreateH264Encoder();
    if(!CreateAACEncoder(aacStreaming, aacStreamEncID, GetAudioBitrate(), "simple_aac", 0))
        throw "Failed to create aac streaming encoder (simple output)";
    
    return true;
}

bool OBSApp::ResetAudio() {
    ProfileScope("OBSApp::ResetAudio");
    
    struct obs_audio_info ai;
    ai.samples_per_sec = config_get_uint(globalConfig, "Audio", "SampleRate");
    
    const char *channelSetupStr = config_get_string(globalConfig, "Audio", "ChannelSetup");

    if (strcmp(channelSetupStr, "Mono") == 0)
        ai.speakers = SPEAKERS_MONO;
    else
        ai.speakers = SPEAKERS_STEREO;
    
    return obs_reset_audio(&ai);
}

int OBSApp::ResetVideo() {
    struct obs_video_info ovi;
    ovi.adapter         = 0;
    ovi.base_width      = baseWidth;
    ovi.base_height     = baseHeight;
    ovi.fps_num         = 30000;
    ovi.fps_den         = 1001;
    ovi.graphics_module = config_get_string(globalConfig, "Video", "Renderer");
    ovi.output_format   = VIDEO_FORMAT_RGBA;
    ovi.output_width    = 1280;
    ovi.output_height   = 720;
    
    if (obs_reset_video(&ovi) != 0)
        throw "Couldn't initialize video";
    return 0;
}

void OBSApp::InitPrimitives() {
    ProfileScope("OBSApp::InitPrimitives");
    
    obs_enter_graphics();
    
    gs_render_start(true);
    gs_vertex2f(0.0f, 0.0f);
    gs_vertex2f(0.0f, 1.0f);
    gs_vertex2f(1.0f, 1.0f);
    gs_vertex2f(1.0f, 0.0f);
    gs_vertex2f(0.0f, 0.0f);
    box = gs_render_save();
    
    gs_render_start(true);
    gs_vertex2f(0.0f, 0.0f);
    gs_vertex2f(0.0f, 1.0f);
    boxLeft = gs_render_save();
    
    gs_render_start(true);
    gs_vertex2f(0.0f, 0.0f);
    gs_vertex2f(1.0f, 0.0f);
    boxTop = gs_render_save();
    
    gs_render_start(true);
    gs_vertex2f(1.0f, 0.0f);
    gs_vertex2f(1.0f, 1.0f);
    boxRight = gs_render_save();
    
    gs_render_start(true);
    gs_vertex2f(0.0f, 1.0f);
    gs_vertex2f(1.0f, 1.0f);
    boxBottom = gs_render_save();
    
    gs_render_start(true);
    for (int i = 0; i <= 360; i += (360/20)) {
        float pos = RAD(float(i));
        gs_vertex2f(cosf(pos), sinf(pos));
    }
    circle = gs_render_save();
    
    obs_leave_graphics();
}

obs_service_t* OBSApp::LoadCustomService(obs_data_t* settings) {
    obs_service_t* service = obs_service_create("rtmp_custom", "default_service", settings, nullptr);
    return service;
}

void OBSApp::SourceLoaded(void *data, obs_source_t *source) {
    blog(LOG_INFO, "source loaded for id: %s", source->info.id);
    if (obs_scene_from_source(source) != NULL) {
        OBSSource sref = OBSSource(source);
        ((OBSApp*)data)->SetCurrentScene(&sref);
    }
}

void OBSApp::LoadDefaultScene() {
    // load scene
    char scenePath[512];
    int ret = GetConfigPath(scenePath, sizeof(scenePath), "data/scenes/default.json");
    if (ret <= 0)
        throw "Failed to get scene collection json file path";
    LoadScene(scenePath);
}

void OBSApp::LoadScene(const char *file) {
    // load scene json
    obs_data_t *data = obs_data_create_from_json_file_safe(file, nullptr);
    if (!data) {
        return;
    }
    
    // clear old scene data
    ClearSceneData();
    InitDefaultTransitions();
    
    // get fields from json
    obs_data_array_t *sources = obs_data_get_array(data, "sources");
    const char *sceneName = obs_data_get_string(data, "current_scene");
    const char *transitionName = obs_data_get_string(data, "current_transition");
    
    // get transition duration
    int newDuration = (int)obs_data_get_int(data, "transition_duration");
    if (!newDuration)
        newDuration = 300;
    
    // get transition name, if don't have, get fade transition name
    if (!transitionName)
        transitionName = obs_source_get_name(fadeTransition);
    
    const char *curSceneCollection = config_get_string(globalConfig, "Basic", "SceneCollection");
    
    obs_data_set_default_string(data, "name", curSceneCollection);
    
    const char       *name = obs_data_get_string(data, "name");
    obs_source_t     *curScene;
    obs_source_t     *curTransition;
    
    if (!name || !*name)
        name = curSceneCollection;

    LoadAudioDevice(DESKTOP_AUDIO_1, 1, data);
    LoadAudioDevice(DESKTOP_AUDIO_2, 2, data);
    LoadAudioDevice(AUX_AUDIO_1,     3, data);
    LoadAudioDevice(AUX_AUDIO_2,     4, data);
    LoadAudioDevice(AUX_AUDIO_3,     5, data);
    
    // load sources
    obs_load_sources(sources, OBSApp::SourceLoaded, this);
    
    // find transition
    curTransition = FindTransition(transitionName);
    if (!curTransition)
        curTransition = fadeTransition;
    
    // set transition
    obs_set_output_source(0, curTransition);
    
    // get current scene
    curScene = obs_get_source_by_name(sceneName);
    obs_transition_set(curTransition, curScene);
}

obs_source_t* OBSApp::FindTransition(const char *name) {
    for(vector<OBSSource>::iterator iter = transitions.begin(); iter != transitions.end(); iter++) {
        OBSSource tr = *iter;
        const char *trName = obs_source_get_name(tr);
        if (strcmp(trName, name) == 0)
            return tr;
    }
    
    return nullptr;
}

void OBSApp::SetCurrentScene(OBSSource* s) {
    curScene = *s;
}

void OBSApp::ClearSceneData() {
    // clear output sources
    obs_set_output_source(0, nullptr);
    obs_set_output_source(1, nullptr);
    obs_set_output_source(2, nullptr);
    obs_set_output_source(3, nullptr);
    obs_set_output_source(4, nullptr);
    obs_set_output_source(5, nullptr);
    
    // remove unused sources
    auto cb = [](void *unused, obs_source_t *source)
    {
        obs_source_remove(source);
        UNUSED_PARAMETER(unused);
        return true;
    };
    obs_enum_sources(cb, nullptr);
    
    // log
    blog(LOG_INFO, "All scene data cleared");
    blog(LOG_INFO, "------------------------------------------------");
}

void OBSApp::InitTransition(obs_source_t *transition) {
    // TODO: not understand this yet
}

void OBSApp::InitDefaultTransitions() {
    size_t idx = 0;
    const char *id;
    
    /* automatically add transitions that have no configuration (things
     * such as cut/fade/etc) */
    while (obs_enum_transition_types(idx++, &id)) {
        if (!obs_is_source_configurable(id)) {
            const char *name = obs_source_get_display_name(id);
            
            obs_source_t *tr = obs_source_create_private(id, name, NULL);
            InitTransition(tr);
            transitions.emplace_back(tr);
            
            if (strcmp(id, "fade_transition") == 0)
                fadeTransition = tr;
            
            obs_source_release(tr);
        }
    }
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
    
    // audio
    config_set_default_string(globalConfig, "Audio", "MonitoringDeviceId", "default");
    config_set_default_string(globalConfig, "Audio", "MonitoringDeviceName", "Default");
    config_set_default_uint  (globalConfig, "Audio", "SampleRate", 44100);
    config_set_default_string(globalConfig, "Audio", "ChannelSetup", "Stereo");
    
    // streaming
    config_set_default_uint(globalConfig, "SimpleOutput", "VBitrate", 2500);
    config_set_default_string(globalConfig, "SimpleOutput", "StreamEncoder", SIMPLE_ENCODER_X264);
    config_set_default_uint(globalConfig, "SimpleOutput", "ABitrate", 160);
    
    // general output
    config_set_default_bool(globalConfig, "Output", "DelayEnable", false);
    config_set_default_uint(globalConfig, "Output", "DelaySec", 20);
    config_set_default_bool(globalConfig, "Output", "DelayPreserve", true);
    config_set_default_bool(globalConfig, "Output", "Reconnect", true);
    config_set_default_uint(globalConfig, "Output", "RetryDelay", 10);
    config_set_default_uint(globalConfig, "Output", "MaxRetries", 20);
    config_set_default_string(globalConfig, "Output", "BindIP", "default");
    config_set_default_bool(globalConfig, "Output", "NewSocketLoopEnable", false);
    config_set_default_bool(globalConfig, "Output", "LowLatencyEnable", false);
    
    return true;
}

void OBSApp::CreateDisplay(gs_window window) {
    // if already created, return
    if(display) {
        return;
    }
    
    // create display
    gs_init_data info = {};
    info.cx = viewWidth;
    info.cy = viewHeight;
    info.format = GS_RGBA;
    info.zsformat = GS_ZS_NONE;
    info.window = window;
    display = obs_display_create(&info);
    
    // add draw callback
    obs_display_add_draw_callback(GetDisplay(), OBSApp::RenderMain, this);
    
    // get video settings
    obs_video_info ovi;
    obs_get_video_info(&ovi);
    
    // ensure video scale is set
    if(videoScale == 0) {
        float viewAspect = float(viewWidth) / float(viewHeight);
        float videoAspect = float(ovi.base_width) / float(ovi.base_height);
        if(viewAspect > videoAspect) {
            videoScale = float(viewHeight) / float(ovi.base_height);
        } else {
            videoScale = float(viewWidth) / float(ovi.base_width);
        }
    }
}

obs_display_t* OBSApp::GetDisplay() {
    return display;
}

void OBSApp::LoadAudioDevice(const char *name, int channel, obs_data_t *parent) {
    obs_data_t *data = obs_data_get_obj(parent, name);
    if (!data)
        return;
    
    obs_source_t *source = obs_load_source(data);
    if (source) {
        obs_set_output_source(channel, source);
        obs_source_release(source);
    }
    
    obs_data_release(data);
}

void OBSApp::RenderMain(void *data, uint32_t cx, uint32_t cy) {
    // get video settings
    obs_video_info ovi;
    obs_get_video_info(&ovi);
    
    // ensure video scale is set
    OBSApp* app = (OBSApp*)data;
    float videoScale = app->GetVideoScale();
    int previewCX = int(videoScale * ovi.base_width);
    int previewCY = int(videoScale * ovi.base_height);
    
    gs_viewport_push();
    gs_projection_push();
    
    /* --------------------------------------- */
    
    gs_ortho(0.0f, float(ovi.base_width), 0.0f, float(ovi.base_height),
             -100.0f, 100.0f);
    gs_set_viewport(0, 0, previewCX, previewCY);
    
    obs_render_main_view();
    gs_load_vertexbuffer(nullptr);
    
    /* --------------------------------------- */
    
    gs_projection_pop();
    gs_viewport_pop();
    
    UNUSED_PARAMETER(cx);
    UNUSED_PARAMETER(cy);
}

const char* OBSApp::FindAudioEncoderFromCodec(const char *type) {
    const char *alt_enc_id = nullptr;
    size_t i = 0;
    
    while (obs_enum_encoder_types(i++, &alt_enc_id)) {
        const char *codec = obs_get_encoder_codec(alt_enc_id);
        if (strcmp(type, codec) == 0) {
            return alt_enc_id;
        }
    }
    
    return nullptr;
}

bool OBSApp::StartStreaming(const char* url, const char* key) {
    // if already in streaming, return
    if(streamingActive) {
        return false;
    }
    
    // if has service, release it
    if(service != nullptr) {
        obs_service_release(service);
        service = nullptr;
    }
    
    // create services
    obs_data_t* settings = obs_data_create();
    obs_data_set_string(settings, "server", url);
    obs_data_set_string(settings, "key", key);
    service = LoadCustomService(settings);
    obs_data_release(settings);
    
    // if not ok, return
    if(!service) {
        return false;
    }
    
    // setup output
    SetupOutputs();
    
    // get output type
    const char *type = obs_service_get_output_type(service);
    if (!type)
        type = "rtmp_output";

    // create output
    streamOutput = obs_output_create(type, "simple_stream", nullptr, nullptr);
    if (!streamOutput)
        return false;
    obs_output_release(streamOutput);

    // get codec
    const char *codec = obs_output_get_supported_audio_codecs(streamOutput);
    if (!codec) {
        return false;
    }
    
    // if codec is not AAC, re-create audio encoder
    if (strcmp(codec, "aac") != 0) {
        const char *id = FindAudioEncoderFromCodec(codec);
        int audioBitrate = GetAudioBitrate();
        obs_data_t *settings = obs_data_create();
        obs_data_set_int(settings, "bitrate", audioBitrate);
        
        aacStreaming = obs_audio_encoder_create(id, "alt_audio_enc", nullptr, 0, nullptr);
        obs_encoder_release(aacStreaming);
        if (!aacStreaming)
            return false;
        
        obs_encoder_update(aacStreaming, settings);
        obs_encoder_set_audio(aacStreaming, obs_get_audio());
        obs_data_release(settings);
    }
    
    // set encoder to output, and bind service
    obs_output_set_video_encoder(streamOutput, h264Streaming);
    obs_output_set_audio_encoder(streamOutput, aacStreaming, 0);
    obs_output_set_service(streamOutput, service);
    
    // get output default config
    bool reconnect = config_get_bool(globalConfig, "Output", "Reconnect");
    int retryDelay = (int)config_get_uint(globalConfig, "Output", "RetryDelay");
    int maxRetries = (int)config_get_uint(globalConfig, "Output", "MaxRetries");
    bool useDelay = config_get_bool(globalConfig, "Output", "DelayEnable");
    int delaySec = (int)config_get_int(globalConfig, "Output", "DelaySec");
    bool preserveDelay = config_get_bool(globalConfig, "Output", "DelayPreserve");
    const char *bindIP = config_get_string(globalConfig, "Output", "BindIP");
    bool enableNewSocketLoop = config_get_bool(globalConfig, "Output", "NewSocketLoopEnable");
    bool enableLowLatencyMode = config_get_bool(globalConfig, "Output", "LowLatencyEnable");
    
    // update output settings
    settings = obs_data_create();
    obs_data_set_string(settings, "bind_ip", bindIP);
    obs_data_set_bool(settings, "new_socket_loop_enabled", enableNewSocketLoop);
    obs_data_set_bool(settings, "low_latency_mode_enabled", enableLowLatencyMode);
    obs_output_update(streamOutput, settings);
    obs_data_release(settings);
    
    // if not reconnect, set max retry time to zero
    if (!reconnect)
        maxRetries = 0;
    
    // set delay
    obs_output_set_delay(streamOutput, useDelay ? delaySec : 0, preserveDelay ? OBS_OUTPUT_DELAY_PRESERVE : 0);
    
    // set reconnect settings
    obs_output_set_reconnect_settings(streamOutput, maxRetries, retryDelay);
    
    // start streaming
    if (obs_output_start(streamOutput)) {
        return true;
    }
    
    return false;
}

void OBSApp::CreateH264Encoder() {
    const char *encoder = config_get_string(globalConfig, "SimpleOutput", "StreamEncoder");
    if (strcmp(encoder, SIMPLE_ENCODER_QSV) == 0)
        CreateH264Encoder("obs_qsv11");
    else if (strcmp(encoder, SIMPLE_ENCODER_AMD) == 0)
        CreateH264Encoder("amd_amf_h264");
    else if (strcmp(encoder, SIMPLE_ENCODER_NVENC) == 0)
        CreateH264Encoder("ffmpeg_nvenc");
    else
        CreateH264Encoder("obs_x264");
}

void OBSApp::CreateH264Encoder(const char *encoderId) {
    h264Streaming = obs_video_encoder_create(encoderId, "simple_h264_stream", nullptr, nullptr);
    if (!h264Streaming)
        throw "Failed to create h264 streaming encoder (simple output)";
    obs_encoder_release(h264Streaming);
}

bool OBSApp::CreateAACEncoder(OBSEncoder &res, string &id, int bitrate, const char *name, size_t idx) {
    const char *id_ = GetAACEncoderForBitrate(bitrate);
    if (!id_) {
        id.clear();
        res = nullptr;
        return false;
    }
    
    if (id == id_)
        return true;
    
    id = id_;
    res = obs_audio_encoder_create(id_, name, nullptr, idx, nullptr);
    
    if (res) {
        obs_encoder_release(res);
        return true;
    }
    
    return false;
}

const char* OBSApp::GetAACEncoderForBitrate(int bitrate) {
    auto &map_ = GetAACEncoderBitrateMap();
    auto res = map_.find(bitrate);
    if (res == end(map_))
        return NULL;
    return res->second;
}

int OBSApp::GetAudioBitrate() {
    int bitrate = (int)config_get_uint(globalConfig, "SimpleOutput", "ABitrate");
    return FindClosestAvailableAACBitrate(bitrate);
}

int OBSApp::FindClosestAvailableAACBitrate(int bitrate) {
    auto &map_ = GetAACEncoderBitrateMap();
    int prev = 0;
    int next = INVALID_BITRATE;
    
    for (auto val : map_) {
        if (next > val.first) {
            if (val.first == bitrate)
                return bitrate;
            
            if (val.first < next && val.first > bitrate)
                next = val.first;
            if (val.first > prev && val.first < bitrate)
                prev = val.first;
        }
    }
    
    if (next != INVALID_BITRATE)
        return next;
    if (prev != 0)
        return prev;
    return 192;
}

const map<int, const char*>& OBSApp::GetAACEncoderBitrateMap() {
    PopulateBitrateMap();
    return bitrateMap;
}

const char* OBSApp::NullToEmpty(const char *str) {
    return str ? str : "";
}

const char * OBSApp::EncoderName(const char *id) {
    return NullToEmpty(obs_encoder_get_display_name(id));
}

const char* OBSApp::GetCodec(const char *id) {
    return NullToEmpty(obs_get_encoder_codec(id));
}

void OBSApp::PopulateBitrateMap() {
    call_once(populateBitrateMap, []() {
        HandleEncoderProperties(fallbackEncoder.c_str());

        const char *id = nullptr;
        for (size_t i = 0; obs_enum_encoder_types(i, &id); i++) {
            auto Compare = [=](const string &val) {
                return val == NullToEmpty(id);
            };

            if (find_if(begin(encoders), end(encoders), Compare) != end(encoders))
                continue;

            if (aac_ != GetCodec(id))
                continue;

            HandleEncoderProperties(id);
        }

        for (auto &encoder : encoders) {
            if (encoder == fallbackEncoder)
                continue;

            if (aac_ != GetCodec(encoder.c_str()))
                continue;

            HandleEncoderProperties(encoder.c_str());
        }

        if (bitrateMap.empty()) {
            blog(LOG_ERROR, "Could not enumerate any AAC encoder bitrates");
            return;
        }

        ostringstream ss;
        for (auto &entry : bitrateMap)
            ss << "\n	" << setw(3) << entry.first << " kbit/s: '" << EncoderName(entry.second) << "' (" << entry.second << ')';

        blog(LOG_DEBUG, "AAC encoder bitrate mapping:%s",
        ss.str().c_str());
    });
}

void OBSApp::HandleIntProperty(obs_property_t *prop, const char *id) {
    const int max_ = obs_property_int_max(prop);
    const int step = obs_property_int_step(prop);
    
    for (int i = obs_property_int_min(prop); i <= max_; i += step)
        bitrateMap[i] = id;
}

void OBSApp::HandleListProperty(obs_property_t *prop, const char *id) {
    obs_combo_format format = obs_property_list_format(prop);
    if (format != OBS_COMBO_FORMAT_INT) {
        blog(LOG_ERROR, "Encoder '%s' (%s) returned bitrate "
             "OBS_PROPERTY_LIST property of unhandled "
             "format %d",
             EncoderName(id), id, static_cast<int>(format));
        return;
    }
    
    const size_t count = obs_property_list_item_count(prop);
    for (size_t i = 0; i < count; i++) {
        if (obs_property_list_item_disabled(prop, i))
            continue;
        
        int bitrate = static_cast<int>(
                                       obs_property_list_item_int(prop, i));
        bitrateMap[bitrate] = id;
    }
}

void OBSApp::HandleEncoderProperties(const char *id) {
    auto DestroyProperties = [](obs_properties_t *props) {
        obs_properties_destroy(props);
    };
    std::unique_ptr<obs_properties_t, decltype(DestroyProperties)> props{
        obs_get_encoder_properties(id),
        DestroyProperties};
    
    if (!props) {
        blog(LOG_ERROR, "Failed to get properties for encoder "
             "'%s' (%s)",
             EncoderName(id), id);
        return;
    }
    
    // if encoder has samplerate, replace it with value in global config
    obs_property_t *samplerate = obs_properties_get(props.get(), "samplerate");
    if (samplerate)
        HandleSampleRate(samplerate, id);
    
    obs_property_t *bitrate = obs_properties_get(props.get(), "bitrate");
    
    obs_property_type type = obs_property_get_type(bitrate);
    switch (type) {
        case OBS_PROPERTY_INT:
            return HandleIntProperty(bitrate, id);
        case OBS_PROPERTY_LIST:
            return HandleListProperty(bitrate, id);
        default:
            break;
    }
    
    blog(LOG_ERROR, "Encoder '%s' (%s) returned bitrate property "
         "of unhandled type %d", EncoderName(id), id,
         static_cast<int>(type));
}

void OBSApp::HandleSampleRate(obs_property_t* prop, const char *id) {
    auto ReleaseData = [](obs_data_t *data) {
        obs_data_release(data);
    };
    std::unique_ptr<obs_data_t, decltype(ReleaseData)> data {
        obs_encoder_defaults(id),
        ReleaseData
    };
    
    if (!data) {
        blog(LOG_ERROR, "Failed to get defaults for encoder '%s' (%s) "
             "while populating bitrate map",
             EncoderName(id), id);
        return;
    }
    
    ConfigFile& globalConfig = OBSApp::sharedApp()->GetGlobalConfig();
    uint32_t sampleRate = (uint32_t)config_get_uint(globalConfig, "Audio", "SampleRate");
    
    obs_data_set_int(data.get(), "samplerate", sampleRate);
    
    obs_property_modified(prop, data.get());
}

void OBSApp::SetupOutputs() {
    // create settings
    obs_data_t *h264Settings = obs_data_create();
    obs_data_t *aacSettings  = obs_data_create();
    
    // get bitrate of av
    int videoBitrate = (int)config_get_uint(globalConfig, "SimpleOutput", "VBitrate");
    int audioBitrate = GetAudioBitrate();
    
    // set settings
    obs_data_set_string(h264Settings, "rate_control", "CBR");
    obs_data_set_int(h264Settings, "bitrate", videoBitrate);
    obs_data_set_string(aacSettings, "rate_control", "CBR");
    obs_data_set_int(aacSettings, "bitrate", audioBitrate);
    
    // apply settings to rtmp service
    obs_service_apply_encoder_settings(service, h264Settings, aacSettings);
    
    // set perferred video format
    video_t *video = obs_get_video();
    enum video_format format = video_output_get_format(video);
    if (format != VIDEO_FORMAT_NV12 && format != VIDEO_FORMAT_I420) {
        obs_encoder_set_preferred_video_format(h264Streaming, VIDEO_FORMAT_NV12);
    }
    
    // apply settings to encoders
    obs_encoder_update(h264Streaming, h264Settings);
    obs_encoder_update(aacStreaming,  aacSettings);
    
    // release settings data struct
    obs_data_release(h264Settings);
    obs_data_release(aacSettings);
    
    // bind encoder with video/audio
    obs_encoder_set_video(h264Streaming, obs_get_video());
    obs_encoder_set_audio(aacStreaming,  obs_get_audio());
}
