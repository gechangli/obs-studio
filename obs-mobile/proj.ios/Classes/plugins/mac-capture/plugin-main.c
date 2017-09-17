#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_OSX

#include <obs-module.h>
#include <obs-internal.h>

// it is statically linked
#define __STATIC_MODULE__

#ifdef __STATIC_MODULE__
OBS_DECLARE_STATIC_MODULE(mac_capture)
OBS_STATIC_MODULE_USE_DEFAULT_LOCALE(mac_capture, "zh_CN")
#else
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("mac_capture", "zh_CN")
#endif

extern struct obs_source_info coreaudio_input_capture_info;
extern struct obs_source_info coreaudio_output_capture_info;
extern struct obs_source_info display_capture_info;
extern struct obs_source_info window_capture_info;

#ifdef __STATIC_MODULE__
static bool _obs_module_load()
#else
bool obs_module_load(void)
#endif
{
	obs_register_source(&coreaudio_input_capture_info);
	obs_register_source(&coreaudio_output_capture_info);
	obs_register_source(&display_capture_info);
	obs_register_source(&window_capture_info);
	return true;
}

#ifdef __STATIC_MODULE__

#ifdef __cplusplus
extern "C" {
#endif
    
obs_module_t* create_static_module_mac_capture() {
    obs_module_t* mod = (obs_module_t*)bzalloc(sizeof(obs_module_t));
    mod->mod_name = bstrdup("mac_capture");
    mod->file = bstrdup("mac_capture");
    mod->data_path = bstrdup("");
    mod->is_static = true;
    mod->load = _obs_module_load;
    mod->set_locale = _obs_module_set_locale;
    mod->free_locale = _obs_module_free_locale;
    mod->ver = _obs_module_ver;
    mod->set_pointer = _obs_module_set_pointer;
    return mod;
}
    
#ifdef __cplusplus
}
#endif

#endif // #ifdef __STATIC_MODULE__

#endif // #if TARGET_OS_OSX
#endif // #ifdef __APPLE__
