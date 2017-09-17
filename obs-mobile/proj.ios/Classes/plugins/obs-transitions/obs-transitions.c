#include <obs-module.h>
#include <obs.h>
#include <obs-internal.h>

// it is statically linked
#define __STATIC_MODULE__

// declare module, static or not
#ifdef __STATIC_MODULE__
OBS_DECLARE_STATIC_MODULE(obs_transitions)
OBS_STATIC_MODULE_USE_DEFAULT_LOCALE(obs_transitions, "zh_CN")
#else
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs_transitions", "zh_CN")
#endif

extern struct obs_source_info cut_transition;
extern struct obs_source_info fade_transition;
extern struct obs_source_info swipe_transition;
extern struct obs_source_info slide_transition;
extern struct obs_source_info stinger_transition;
extern struct obs_source_info fade_to_color_transition;
extern struct obs_source_info luma_wipe_transition;

#ifdef __STATIC_MODULE__
static bool _obs_module_load()
#else
bool obs_module_load(void)
#endif
{
	obs_register_source(&cut_transition);
	obs_register_source(&fade_transition);
	obs_register_source(&swipe_transition);
	obs_register_source(&slide_transition);
	obs_register_source(&stinger_transition);
	obs_register_source(&fade_to_color_transition);
	obs_register_source(&luma_wipe_transition);
	return true;
}

#ifdef __STATIC_MODULE__

#ifdef __cplusplus
extern "C" {
#endif
    
obs_module_t* create_static_module_obs_transitions() {
    obs_module_t* mod = (obs_module_t*)bzalloc(sizeof(obs_module_t));
    mod->mod_name = bstrdup("obs_transitions");
    mod->file = bstrdup("obs_transitions");
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
