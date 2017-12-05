/*
 * Copyright (c) 2016-2017 Luma <stubma@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef __APPLE__
#include <TargetConditionals.h>

#if TARGET_OS_IPHONE
#include <stdlib.h>
#include <unistd.h>
#include "platform.h"
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include "bmem.h"
#include "dstr.h"
#import <Foundation/Foundation.h>

int os_get_logical_cores(void) {
    unsigned int ncpu;
    size_t len = sizeof(ncpu);
    sysctlbyname ("hw.ncpu", &ncpu, &len, NULL, 0);
    return ncpu;
}

int os_get_physical_cores(void) {
    return os_get_logical_cores();
}

typedef uint64_t (*time_func)();

static uint64_t ns_time_simple() {
    return mach_absolute_time();
}

static double ns_time_compute_factor() {
    mach_timebase_info_data_t info = {1, 1};
    mach_timebase_info(&info);
    return ((double)info.numer) / info.denom;
}

static uint64_t ns_time_full() {
    static double factor = 0.;
    if (factor == 0.) factor = ns_time_compute_factor();
    return (uint64_t)(mach_absolute_time() * factor);
}

static time_func ns_time_select_func() {
    mach_timebase_info_data_t info = {1, 1};
    mach_timebase_info(&info);
    if (info.denom == info.numer)
        return ns_time_simple;
    return ns_time_full;
}

uint64_t os_gettime_ns(void) {
    static time_func f = NULL;
    if (!f) f = ns_time_select_func();
    return f();
}

static int os_get_path_internal(char *dst, size_t size, const char *name, NSSearchPathDomainMask domainMask) {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, domainMask, YES);
    
    if([paths count] == 0)
        bcrash("Could not get document directory (platform-ios)");
    
    NSString *docDir = paths[0];
    const char *base_path = [docDir UTF8String];
    
    if (!name || !*name)
        return snprintf(dst, size, "%s", base_path);
    else
        return snprintf(dst, size, "%s/%s", base_path, name);
}

static char *os_get_path_ptr_internal(const char *name, NSSearchPathDomainMask domainMask) {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, domainMask, YES);
    if([paths count] == 0)
        bcrash("Could not get document directory (platform-ios)");
    
    NSString *docDir = paths[0];
    NSUInteger len = [docDir lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
    
    char *path_ptr = bmalloc(len+1);
    
    path_ptr[len] = 0;
    
    memcpy(path_ptr, [docDir UTF8String], len);
    
    struct dstr path;
    dstr_init_move_array(&path, path_ptr);
    dstr_cat(&path, "/");
    dstr_cat(&path, name);
    return path.array;
}

int os_get_config_path(char *dst, size_t size, const char *name) {
    return os_get_path_internal(dst, size, name, NSUserDomainMask);
}

char *os_get_config_path_ptr(const char *name) {
    return os_get_path_ptr_internal(name, NSUserDomainMask);
}

int os_get_program_data_path(char *dst, size_t size, const char *name) {
    return os_get_path_internal(dst, size, name, NSUserDomainMask);
}

char *os_get_program_data_path_ptr(const char *name) {
    return os_get_path_ptr_internal(name, NSUserDomainMask);
}

os_performance_token_t *os_request_high_performance(const char *reason) {
    UNUSED_PARAMETER(reason);
    return NULL;
}

void os_end_high_performance(os_performance_token_t *token) {
    UNUSED_PARAMETER(token);
}

#endif // #if TARGET_OS_IPHONE

#endif // #ifdef __APPLE__
