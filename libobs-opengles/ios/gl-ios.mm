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
#import <GLKit/GLKit.h>

#ifdef __cplusplus
extern "C" {
#endif
    
// extension strings
static char* _gl_extensions;
    
struct gl_windowinfo {
	GLKView* view;
};

struct gl_platform {
	EAGLContext *context;
};

struct gl_platform *gl_platform_create(gs_device_t *device, uint32_t adapter)
{
    // allocate platform struct
	struct gl_platform *plat = (struct gl_platform *)bzalloc(sizeof(struct gl_platform));
    
    // set low version flag
    GLES_VERSION_1_0 = false;
    GLES_VERSION_1_1 = false;
    GLES_VERSION_2_0 = true;
    
    // create context
    plat->context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if(plat->context == nil) {
        GLES_VERSION_3_0 = false;
        GLES_VERSION_3_1 = false;
        GLES_VERSION_3_2 = false;
        plat->context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    } else {
        GLES_VERSION_3_0 = true;
        GLES_VERSION_3_1 = false;
        GLES_VERSION_3_2 = false;
    }
    
    // set current context
    [EAGLContext setCurrentContext:plat->context];
    
    // get gl extensions
    _gl_extensions = (char*)glGetString(GL_EXTENSIONS);
    
//    int compiled = 0;
//    GLuint shader = glCreateShader(GL_VERTEX_SHADER);
//    const GLchar* sources[] = {
//        "#version 300 es\n"
//        "uniform mat4x4 ViewProj;\n"
//        "in vec4 _input_attrib0;\n"
//        "in vec2 _input_attrib1;\n"
//        "out vec4 _vertex_shader_attrib0;\n"
//        "out vec2 _vertex_shader_attrib1;\n"
//        "struct VertInOut {\n"
//        "    vec4 pos;\n"
//        "    vec2 uv;\n"
//        "};\n"
//        "VertInOut VSDefault(VertInOut vert_in)\n"
//        "{\n"
//        "    VertInOut vert_out;\n"
//        "    vert_out.pos = ((vec4(vert_in.pos.xyz, 1.0)) * (ViewProj));\n"
//        "    vert_out.uv  = vert_in.uv;\n"
//        "    return vert_out;\n"
//        "}\n"
//        "VertInOut _main_wrap(VertInOut vert_in)\n"
//        "{\n"
//        "    return VSDefault(vert_in);\n"
//        "}\n"
//        "void main(void)\n"
//        "{\n"
//        "    VertInOut vert_in;\n"
//        "    VertInOut outputval;\n"
//        "\n"
//        "    vert_in.pos = _input_attrib0;\n"
//        "    vert_in.uv = _input_attrib1;\n"
//        "\n"
//        "    outputval = _main_wrap(vert_in);\n"
//        "\n"
//        "    _vertex_shader_attrib0 = outputval.pos;\n"
//        "    gl_Position = outputval.pos;\n"
//        "    _vertex_shader_attrib1 = outputval.uv;\n"
//        "}\n"
//    };
//    glShaderSource(shader, 1, sources, 0);
//    glCompileShader(shader);
//    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
//    if (!compiled) {
//        printf("test");
//    }
    
    // return
	return plat;
}
    
bool gl_has_extension(const char* ext) {
    return strstr(_gl_extensions, ext) != NULL;
}

void gl_platform_destroy(struct gl_platform *platform)
{
	if(!platform)
		return;
    
	platform->context = nil;
	bfree(platform);
}

bool gl_platform_init_swapchain(struct gs_swap_chain *swap) {
	return true;
}

void gl_platform_cleanup_swapchain(struct gs_swap_chain *swap) {
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
	if (swap && swap->wi->view.context == nil) {
        [swap->wi->view setContext:device->plat->context];
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
