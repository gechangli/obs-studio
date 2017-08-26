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
#include "obs-internal.h"
#include "obs_app.hpp"
#include <TargetConditionals.h>

using namespace std;

#define DEFAULT_LANG "en_US"
#define SERVICE_PATH "service.json"

bool portable_mode = false;

OBSApp::OBSApp(int w, int h, profiler_name_store_t *store) :
videoWidth(w),
videoHeight(h),
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
    
    // reset video
    ResetVideo();
    
    // init rtmp services
    if (!InitService())
        throw "Failed to initialize service";
    
    // init graphics
    InitPrimitives();
    
    return true;
}

int OBSApp::ResetVideo() {
    struct obs_video_info ovi;
    ovi.adapter         = 0;
    ovi.base_width      = videoWidth;
    ovi.base_height     = videoHeight;
    ovi.fps_num         = 30000;
    ovi.fps_den         = 1001;
    ovi.graphics_module = config_get_string(globalConfig, "Video", "Renderer");
    ovi.output_format   = VIDEO_FORMAT_RGBA;
    ovi.output_width    = videoWidth;
    ovi.output_height   = videoHeight;
    
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

bool OBSApp::InitService() {
    ProfileScope("OBSApp::InitService");
    
    // create service from profile
    if (LoadService())
        return true;
    
    // if there is no profile, try to create a common rtmp service to test
    service = obs_service_create("rtmp_common", "default_service", nullptr, nullptr);
    if (!service)
        return false;
    obs_service_release(service);
    
    return true;
}

bool OBSApp::LoadService() {
    const char *type;
    
    // get service profile
    char serviceJsonPath[512];
    int ret = GetProfilePath(serviceJsonPath, sizeof(serviceJsonPath), SERVICE_PATH);
    if (ret <= 0)
        return false;
    
    // if profile exists, load it and test to create service
    if(os_file_exists(serviceJsonPath)) {
        obs_data_t *data = obs_data_create_from_json_file_safe(serviceJsonPath, nullptr);
        
        obs_data_set_default_string(data, "type", "rtmp_common");
        type = obs_data_get_string(data, "type");
        
        obs_data_t *settings = obs_data_get_obj(data, "settings");
        obs_data_t *hotkey_data = obs_data_get_obj(data, "hotkeys");
        
        service = obs_service_create(type, "default_service", settings, hotkey_data);
        obs_service_release(service);
        
        obs_data_release(hotkey_data);
        obs_data_release(settings);
        obs_data_release(data);
    }

    return !!service;
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
    obs_data_array_t *sceneOrder = obs_data_get_array(data, "scene_order");
    obs_data_array_t *sources = obs_data_get_array(data, "sources");
    obs_data_array_t *transitions = obs_data_get_array(data, "transitions");
    const char *sceneName = obs_data_get_string(data, "current_scene");
    const char *programSceneName = obs_data_get_string(data, "current_program_scene");
    const char *transitionName = obs_data_get_string(data, "current_transition");
    
    // get transition duration
    int newDuration = obs_data_get_int(data, "transition_duration");
    if (!newDuration)
        newDuration = 300;
    
    // get transition name, if don't have, get fade transition name
    if (!transitionName)
        transitionName = obs_source_get_name(fadeTransition);
    
    const char *curSceneCollection = config_get_string(globalConfig, "Basic", "SceneCollection");
    
    obs_data_set_default_string(data, "name", curSceneCollection);
    
    const char       *name = obs_data_get_string(data, "name");
    obs_source_t     *curScene;
    obs_source_t     *curProgramScene;
    obs_source_t     *curTransition;
    
    if (!name || !*name)
        name = curSceneCollection;
    
//    LoadAudioDevice(DESKTOP_AUDIO_1, 1, data);
//    LoadAudioDevice(DESKTOP_AUDIO_2, 2, data);
//    LoadAudioDevice(AUX_AUDIO_1,     3, data);
//    LoadAudioDevice(AUX_AUDIO_2,     4, data);
//    LoadAudioDevice(AUX_AUDIO_3,     5, data);
    
    obs_load_sources(sources, OBSApp::SourceLoaded, this);
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
    return true;
}

void OBSApp::CreateDisplay(gs_window window) {
    // if already created, return
    if(display) {
        return;
    }
    
    // create display
    gs_init_data info = {};
    info.cx = videoWidth;
    info.cy = videoHeight;
    info.format = GS_RGBA;
    info.zsformat = GS_ZS_NONE;
    info.window = window;
    display = obs_display_create(&info);
    
    // add draw callback
    obs_display_add_draw_callback(GetDisplay(), OBSApp::RenderMain, this);
}

obs_display_t* OBSApp::GetDisplay() {
    return display;
}

void OBSApp::RenderMain(void *data, uint32_t cx, uint32_t cy) {
    obs_video_info ovi;
    
    obs_get_video_info(&ovi);
    
    int previewCX = int(ovi.base_width);
    int previewCY = int(ovi.base_height);
    
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
