#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_OSX

#include <obs-module.h>
#include <obs-internal.h>
#include <obs-config.h>

// declare module
OBS_DECLARE_MODULE(mac_capture)
OBS_MODULE_USE_DEFAULT_LOCALE(mac_capture, "zh-CN")

extern struct obs_source_info coreaudio_input_capture_info;
extern struct obs_source_info coreaudio_output_capture_info;
extern struct obs_source_info display_capture_info;
extern struct obs_source_info window_capture_info;

MODULE_VISIBILITY bool MODULE_MANGLING(obs_module_load)()
{
	obs_register_source(&coreaudio_input_capture_info);
	obs_register_source(&coreaudio_output_capture_info);
	obs_register_source(&display_capture_info);
	obs_register_source(&window_capture_info);
	return true;
}

#ifdef __cplusplus
extern "C" {
#endif
    
obs_module_t* STATIC_MODULE_CREATOR(mac_capture)() {
    obs_module_t* mod = (obs_module_t*)bzalloc(sizeof(obs_module_t));
    mod->mod_name = bstrdup("mac_capture");
    mod->file = bstrdup("mac_capture");
    mod->data_path = bstrdup("");
    mod->is_static = true;
    mod->load = MODULE_MANGLING(obs_module_load);
    mod->set_locale = MODULE_MANGLING(obs_module_set_locale);
    mod->free_locale = MODULE_MANGLING(obs_module_free_locale);
    mod->ver = MODULE_MANGLING(obs_module_ver);
    mod->set_pointer = MODULE_MANGLING(obs_module_set_pointer);
    return mod;
}
    
#ifdef __cplusplus
}
#endif

#endif // #if TARGET_OS_OSX
#endif // #ifdef __APPLE__
