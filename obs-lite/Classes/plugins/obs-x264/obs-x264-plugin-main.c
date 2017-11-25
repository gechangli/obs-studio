#include <obs-module.h>
#include <obs-internal.h>
#include <obs-config.h>

// declare module
OBS_DECLARE_MODULE(obs_x264)
OBS_MODULE_USE_DEFAULT_LOCALE(obs_x264, "zh_CN")

extern struct obs_encoder_info obs_x264_encoder;

MODULE_VISIBILITY bool MODULE_MANGLING(obs_module_load)()
{
	obs_register_encoder(&obs_x264_encoder);
	return true;
}

#ifdef __cplusplus
extern "C" {
#endif
    
obs_module_t* STATIC_MODULE_CREATOR(obs_x264)() {
    obs_module_t* mod = (obs_module_t*)bzalloc(sizeof(obs_module_t));
    mod->mod_name = bstrdup("obs_x264");
    mod->file = bstrdup("obs_x264");
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
