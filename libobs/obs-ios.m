/******************************************************************************
    Copyright (C) 2013 by Ruwen Hahn <palana@stunned.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#include "util/platform.h"
#include "util/dstr.h"
#include "obs.h"
#include "obs-internal.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#include <objc/objc.h>
#import <Foundation/Foundation.h>

const char *get_module_extension(void)
{
	return ".so";
}

static const char *module_bin[] = {
	"plugins"
};

static const char *module_data[] = {
	"data/plugins/%module%"
};

static const int module_patterns_size =
	sizeof(module_bin)/sizeof(module_bin[0]);

void add_default_module_paths(void)
{
	NSString* resPath = [NSBundle mainBundle].resourcePath;
	for (int i = 0; i < module_patterns_size; i++) {
		NSString* dataPath = [resPath stringByAppendingFormat:@"/%s", module_data[i]];
		obs_add_module_path(module_bin[i], [dataPath UTF8String]);
	}
}

const char* get_absolute_module_data_path(const char* mod_name) {
	NSString* resPath = [NSBundle mainBundle].resourcePath;
	NSString* dataPath = [resPath stringByAppendingFormat:@"/%s", module_data[0]];
	dataPath = [dataPath stringByReplacingOccurrencesOfString:@"%module%" withString:[NSString stringWithUTF8String:mod_name]];
	return bstrdup([dataPath UTF8String]);
}

char *find_libobs_data_file(const char *file)
{
	NSString* resPath = [NSBundle mainBundle].resourcePath;
	NSString* dataPath = [resPath stringByAppendingFormat:@"/data/libobs/%s", file];
	return bstrdup([dataPath UTF8String]);
}

static void log_processor_name(void)
{
	char   *name = NULL;
	size_t size;
	int    ret;

	ret = sysctlbyname("machdep.cpu.brand_string", NULL, &size, NULL, 0);
	if (ret != 0)
		return;

	name = malloc(size);

	ret = sysctlbyname("machdep.cpu.brand_string", name, &size, NULL, 0);
	if (ret == 0)
		blog(LOG_INFO, "CPU Name: %s", name);

	free(name);
}

static void log_processor_speed(void)
{
	size_t    size;
	long long freq;
	int       ret;

	size = sizeof(freq);
	ret = sysctlbyname("hw.cpufrequency", &freq, &size, NULL, 0);
	if (ret == 0)
		blog(LOG_INFO, "CPU Speed: %lldMHz", freq / 1000000);
}

static void log_processor_cores(void)
{
	blog(LOG_INFO, "Physical Cores: %d, Logical Cores: %d",
			os_get_physical_cores(), os_get_logical_cores());
}

static void log_available_memory(void)
{
	size_t    size;
	long long memory_available;
	int       ret;

	size = sizeof(memory_available);
	ret = sysctlbyname("hw.memsize", &memory_available, &size, NULL, 0);
	if (ret == 0)
		blog(LOG_INFO, "Physical Memory: %lldMB Total",
				memory_available / 1024 / 1024);
}

static void log_os(void)
{
    NSProcessInfo* pi = [NSProcessInfo processInfo];
    NSString *version = [pi operatingSystemVersionString];
    NSString *os = [pi operatingSystemName];
    blog(LOG_INFO, "OS Name: %s", os ? [os cStringUsingEncoding:NSUTF8StringEncoding] : "Unknown");
    blog(LOG_INFO, "OS Version: %s", version ? [version cStringUsingEncoding:NSUTF8StringEncoding] : "Unknown");
}

void log_system_info(void)
{
	log_processor_name();
	log_processor_speed();
	log_processor_cores();
	log_available_memory();
	log_os();
}

struct obs_hotkeys_platform {
};

#define INVALID_KEY 0xff

int obs_key_to_virtual_key(obs_key_t code)
{
    // we don't support key in ios
    return INVALID_KEY;
}

void obs_key_to_str(obs_key_t key, struct dstr *str)
{
    // we don't support shortcut key in iOS
}

void obs_key_combination_to_str(obs_key_combination_t key, struct dstr *str)
{
}

obs_key_t obs_key_from_virtual_key(int code)
{
	return OBS_KEY_NONE;
}

bool obs_hotkeys_platform_init(struct obs_core_hotkeys *hotkeys)
{
    // do nothing for iOS
    return true;
}

void obs_hotkeys_platform_free(struct obs_core_hotkeys *hotkeys)
{
}

bool obs_hotkeys_platform_is_pressed(obs_hotkeys_platform_t *plat,
		obs_key_t key)
{
	return false;
}

#endif // #if TARGET_OS_IPHONE
#endif // #ifdef __APPLE__
