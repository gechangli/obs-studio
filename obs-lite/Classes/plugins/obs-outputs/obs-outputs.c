#include <obs-module.h>
#include <obs-internal.h>
#include <obs-config.h>
#include "obs-outputs-config.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#endif

// declare module
OBS_DECLARE_MODULE(obs_outputs)
OBS_MODULE_USE_DEFAULT_LOCALE(obs_outputs, "zh_CN")

extern struct obs_output_info rtmp_output_info;
extern struct obs_output_info null_output_info;
extern struct obs_output_info flv_output_info;
#if COMPILE_FTL
extern struct obs_output_info ftl_output_info;
#endif

MODULE_VISIBILITY bool MODULE_MANGLING(obs_module_load)()
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

MODULE_VISIBILITY void MODULE_MANGLING(obs_module_unload)()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

#ifdef __cplusplus
extern "C" {
#endif
    
obs_module_t* STATIC_MODULE_CREATOR(obs_outputs)() {
    obs_module_t* mod = (obs_module_t*)bzalloc(sizeof(obs_module_t));
    mod->mod_name = bstrdup("obs_outputs");
    mod->file = bstrdup("obs_outputs");
    mod->data_path = bstrdup("");
    mod->is_static = true;
    mod->load = MODULE_MANGLING(obs_module_load);
    mod->unload = MODULE_MANGLING(obs_module_unload);
    mod->set_locale = MODULE_MANGLING(obs_module_set_locale);
    mod->free_locale = MODULE_MANGLING(obs_module_free_locale);
    mod->ver = MODULE_MANGLING(obs_module_ver);
    mod->set_pointer = MODULE_MANGLING(obs_module_set_pointer);
    return mod;
}
    
#ifdef __cplusplus
}
#endif
