#include <obs-module.h>
#include <obs-internal.h>

// it is statically linked
#define __STATIC_MODULE__

#ifdef __STATIC_MODULE__
OBS_DECLARE_STATIC_MODULE(obs_x264)
OBS_STATIC_MODULE_USE_DEFAULT_LOCALE(obs_x264, "zh_CN")
#else
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs_x264", "zh_CN")
#endif

extern struct obs_encoder_info obs_x264_encoder;

#ifdef __STATIC_MODULE__
static bool _obs_module_load()
#else
bool obs_module_load(void)
#endif
{
	obs_register_encoder(&obs_x264_encoder);
	return true;
}

#ifdef __STATIC_MODULE__

#ifdef __cplusplus
extern "C" {
#endif
    
obs_module_t* create_static_module_obs_x264() {
    obs_module_t* mod = (obs_module_t*)bzalloc(sizeof(obs_module_t));
    mod->mod_name = bstrdup("obs_x264");
    mod->file = bstrdup("obs_x264");
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
