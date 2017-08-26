#include <util/text-lookup.h>
#include <util/threading.h>
#include <util/platform.h>
#include <util/dstr.h>
#include <obs-module.h>
#include <obs-internal.h>
#include <file-updater/file-updater.h>

#include "rtmp-format-ver.h"
#include "lookup-config.h"

// it is statically linked
#define __STATIC_MODULE__

#ifdef __STATIC_MODULE__
OBS_DECLARE_STATIC_MODULE(rtmp_services)
OBS_STATIC_MODULE_USE_DEFAULT_LOCALE(rtmp_services, "zh_CN")
#else
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("rtmp_services", "zh_CN")
#endif

#define RTMP_SERVICES_LOG_STR "[rtmp-services plugin] "
#define RTMP_SERVICES_VER_STR "rtmp-services plugin (libobs " OBS_VERSION ")"

extern struct obs_service_info rtmp_common_service;
extern struct obs_service_info rtmp_custom_service;

static update_info_t *update_info = NULL;

static bool confirm_service_file(void *param, struct file_download_data *file)
{
	if (astrcmpi(file->name, "services.json") == 0) {
		obs_data_t *data;
		int format_version;

		data = obs_data_create_from_json((char*)file->buffer.array);
		if (!data)
			return false;

		format_version = (int)obs_data_get_int(data, "format_version");
		obs_data_release(data);

		if (format_version != RTMP_SERVICES_FORMAT_VERSION)
			return false;
	}

	UNUSED_PARAMETER(param);
	return true;
}

extern void init_twitch_data(void);
extern void load_twitch_data(const char *module_str);
extern void unload_twitch_data(void);

#ifdef __STATIC_MODULE__
static bool _obs_module_load()
#else
bool obs_module_load(void)
#endif
{
	init_twitch_data();

#if !defined(_WIN32) && CHECK_FOR_SERVICE_UPDATES
	char *local_dir = obs_module_file("");
	char *cache_dir = obs_module_config_path("");
	struct dstr module_name = {0};

	dstr_copy(&module_name, "rtmp-services plugin (libobs ");
	dstr_cat(&module_name, obs_get_version_string());
	dstr_cat(&module_name, ")");

	if (cache_dir) {
		update_info = update_info_create(
				RTMP_SERVICES_LOG_STR,
				module_name.array,
				RTMP_SERVICES_URL,
				local_dir,
				cache_dir,
				confirm_service_file, NULL);
	}

	load_twitch_data(module_name.array);

	bfree(local_dir);
	bfree(cache_dir);
	dstr_free(&module_name);
#endif

	obs_register_service(&rtmp_common_service);
	obs_register_service(&rtmp_custom_service);
	return true;
}

#ifdef __STATIC_MODULE__
static void _obs_module_unload()
#else
void obs_module_unload(void)
#endif
{
	update_info_destroy(update_info);
	unload_twitch_data();
}

#ifdef __STATIC_MODULE__

#ifdef __cplusplus
extern "C" {
#endif
    
obs_module_t* create_static_module_rtmp_services() {
    obs_module_t* mod = (obs_module_t*)bzalloc(sizeof(obs_module_t));
    mod->mod_name = bstrdup("rtmp_services");
    mod->file = bstrdup("rtmp_services");
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
