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
#include "gl-subsystem.h"
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import <UIKit/UIKit.h>
#import "EAGLView.h"

#ifdef __cplusplus
extern "C" {
#endif
    
struct gl_windowinfo {
	EAGLView* view;
};

struct gl_platform {
	EAGLContext *context;
};

static EAGLContext *gl_context_create(void) {
    return nil;
}

struct gl_platform *gl_platform_create(gs_device_t *device, uint32_t adapter)
{
	struct gl_platform *plat = (struct gl_platform *)bzalloc(sizeof(struct gl_platform));
	return plat;
}

void gl_platform_destroy(struct gl_platform *platform)
{
	if(!platform)
		return;
    
	platform->context = nil;
	bfree(platform);
}

bool gl_platform_init_swapchain(struct gs_swap_chain *swap)
{
	UNUSED_PARAMETER(swap);

	return true;
}

void gl_platform_cleanup_swapchain(struct gs_swap_chain *swap)
{
	UNUSED_PARAMETER(swap);
}

struct gl_windowinfo *gl_windowinfo_create(const struct gs_init_data *info)
{
	if(!info)
		return NULL;

	if(!info->window.view)
		return NULL;

	struct gl_windowinfo *wi = (struct gl_windowinfo *)bzalloc(sizeof(struct gl_windowinfo));

	wi->view = info->window.view;

	return wi;
}

void gl_windowinfo_destroy(struct gl_windowinfo *wi)
{
	if(!wi)
		return;

	wi->view = nil;
	bfree(wi);
}

void gl_update(gs_device_t *device)
{
    // nothing to do for OpenGLES
}

void GL_MANGLING(device_enter_context)(gs_device_t *device)
{
    [EAGLContext setCurrentContext:device->plat->context];
}

void GL_MANGLING(device_leave_context)(gs_device_t *device)
{
	UNUSED_PARAMETER(device);

    [EAGLContext setCurrentContext:nil];
}

void GL_MANGLING(device_load_swapchain)(gs_device_t *device, gs_swapchain_t *swap)
{
	if(device->cur_swap == swap)
		return;

	device->cur_swap = swap;
	if (swap) {
        [EAGLContext setCurrentContext:swap->wi->view.context];
	} else {
        [EAGLContext setCurrentContext:nil];
	}
}

void GL_MANGLING(device_present)(gs_device_t *device)
{
	[device->plat->context presentRenderbuffer:GL_FRAMEBUFFER];
}

void gl_getclientsize(const struct gs_swap_chain *swap, uint32_t *width,
		uint32_t *height)
{
	if(width) *width = swap->info.cx;
	if(height) *height = swap->info.cy;
}
    
bool GL_MANGLING(device_enum_adapters)(bool (*callback)(void *param, const char *name, uint32_t id),
                                       void *param) {
    // it is optional and not needed in iOS
    return false;
}
    
#ifdef __cplusplus
}
#endif

#endif // #if TARGET_OS_IPHONE

#endif // #ifdef __APPLE__