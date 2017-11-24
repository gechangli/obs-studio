#include <obs-module.h>
#include <obs-internal.h>
#include <obs-config.h>
#include "obs-outputs-config.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#endif

// declare module, static or not
#ifdef __STATIC_MODULE__
OBS_DECLARE_STATIC_MODULE(obs_outputs)
OBS_STATIC_MODULE_USE_DEFAULT_LOCALE(obs_outputs, "zh_CN")
#else
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs_outputs", "zh_CN")
#endif

extern struct obs_output_info rtmp_output_info;
extern struct obs_output_info null_output_info;
extern struct obs_output_info flv_output_info;
#if COMPILE_FTL
extern struct obs_output_info ftl_output_info;
#endif

#ifdef __STATIC_MODULE__
static bool _obs_module_load()
#else
bool obs_module_load(void)
#endif
{
#ifdef _WIN32
	WSADATA wsad;
	WSAStartup(MAKEWORD(2, 2), &wsad);
#endif

	obs_register_output(&rtmp_output_info);
	obs_register_output(&null_output_info);
	obs_register_output(&flv_output_info);
#if COMPILE_FTL
	obs_register_output(&ftl_output_info);
#endif
	return true;
}

#ifdef __STATIC_MODULE__
static void _obs_module_unload()
#else
void obs_module_unload(void)
#endif
{
#ifdef _WIN32
	WSACleanup();
#endif
}

#ifdef __STATIC_MODULE__

#ifdef __cplusplus
extern "C" {
#endif
    
obs_module_t* create_static_module_obs_outputs() {
    obs_module_t* mod = (obs_module_t*)bzalloc(sizeof(obs_module_t));
    mod->mod_name = bstrdup("obs_outputs");
    mod->file = bstrdup("obs_outputs");
    mod->data_path = bstrdup("");
    mod->is_static = true;
    mod->load = _obs_module_load;
    mod->unload = _obs_module_unload;
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
