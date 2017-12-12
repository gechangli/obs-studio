/*
 * cocos2d for iPhone: http://www.cocos2d-iphone.org
 *
 * Copyright (c) 2011 Ricardo Quesada
 * Copyright (c) 2011 Zynga Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 * File autogenerated with Xcode. Adapted for cocos2d needs.
 */

// Only compile this code on iOS. These files should NOT be included on your Mac project.
// But in case they are included, it won't be compiled.

#import "CCES3Renderer.h"
#include "graphics.h"
#include "util/base.h"
#include "gl-helpers.h"

@implementation CCES3Renderer

// Create an OpenGL ES 2.0 context
- (id) initWithDepthFormat:(unsigned int)depthFormat withPixelFormat:(unsigned int)pixelFormat withSharegroup:(EAGLSharegroup*)sharegroup withMultiSampling:(BOOL) multiSampling withNumberOfSamples:(unsigned int) requestedSamples
{
    self = [super init];
    if (self)
    {
        if( ! sharegroup )
            self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        else
            self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:sharegroup];

        if (!self.context || ![EAGLContext setCurrentContext:self.context]) {
            return nil;
        }
        
        depthFormat_ = depthFormat;
        pixelFormat_ = pixelFormat;
        multiSampling_ = multiSampling;

        // Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
        glGenFramebuffers(1, &defaultFramebuffer_);
        NSAssert( defaultFramebuffer_, @"Can't create default frame buffer");

        glGenRenderbuffers(1, &colorRenderbuffer_);
        NSAssert( colorRenderbuffer_, @"Can't create default render buffer");

        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer_);
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer_);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer_);

        if (multiSampling_)
        {
            GLint maxSamplesAllowed;
            glGetIntegerv(GL_MAX_SAMPLES, &maxSamplesAllowed);
            samplesToUse_ = MIN(maxSamplesAllowed, requestedSamples);
            
            /* Create the MSAA framebuffer (offscreen) */
            glGenFramebuffers(1, &msaaFramebuffer_);
            NSAssert( msaaFramebuffer_, @"Can't create default MSAA frame buffer");
            glBindFramebuffer(GL_FRAMEBUFFER, msaaFramebuffer_);
        }

        gl_success("CCES3Renderer::initWithDepthFormat");
    }

    return self;
}

- (BOOL)resizeFromLayer:(CAEAGLLayer *)layer
{
    // Allocate color buffer backing based on the current layer size
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer_);

    if( ! [self.context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer] )
    {
        NSLog(@"failed to call context");
    }
    
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth_);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight_);

    NSLog(@"cocos2d: surface size: %dx%d", (int)backingWidth_, (int)backingHeight_);

    if (multiSampling_)
    {
        if ( msaaColorbuffer_) {
            glDeleteRenderbuffers(1, &msaaColorbuffer_);
            msaaColorbuffer_ = 0;
        }
        
        /* Create the offscreen MSAA color buffer.
         After rendering, the contents of this will be blitted into ColorRenderbuffer */
        
        //msaaFrameBuffer needs to be binded
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFramebuffer_);
        glGenRenderbuffers(1, &msaaColorbuffer_);
        NSAssert(msaaFramebuffer_, @"Can't create MSAA color buffer");
        
        glBindRenderbuffer(GL_RENDERBUFFER, msaaColorbuffer_);
        
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samplesToUse_, pixelFormat_ , backingWidth_, backingHeight_);
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColorbuffer_);
        
        GLenum error;
        if ( (error=glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
        {
            NSLog(@"Failed to make complete framebuffer object 0x%X", error);
            return NO;
        }
    }

    gl_success("resizeFromLayer");

    if (depthFormat_)
    {
        if( ! depthBuffer_ ) {
            glGenRenderbuffers(1, &depthBuffer_);
            NSAssert(depthBuffer_, @"Can't create depth buffer");
        }

        glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer_);
        
        if( multiSampling_ )
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samplesToUse_, depthFormat_,backingWidth_, backingHeight_);
        else
            glRenderbufferStorage(GL_RENDERBUFFER, depthFormat_, backingWidth_, backingHeight_);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer_);
        
        if (depthFormat_ == GL_DEPTH24_STENCIL8) {
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer_);
		}

        // bind color buffer
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer_);        
    }

    gl_success("resizeFromLayer");

    GLenum error;
    if( (error=glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
        blog(LOG_ERROR, "Failed to make complete framebuffer object 0x%X", error);
        return NO;
    }

    return YES;
}

-(CGSize) backingSize
{
    return CGSizeMake( backingWidth_, backingHeight_);
}

- (NSString*) description
{
    return [NSString stringWithFormat:@"<%@ = %08X | size = %ix%i>", [self class], (unsigned int)self, backingWidth_, backingHeight_];
}

- (unsigned int) colorRenderBuffer
{
    return colorRenderbuffer_;
}

- (unsigned int) defaultFrameBuffer
{
    return defaultFramebuffer_;
}

- (unsigned int) msaaFrameBuffer
{
    return msaaFramebuffer_;
}

- (unsigned int) msaaColorBuffer
{
    return msaaColorbuffer_;
}

- (void)dealloc {
    // Tear down GL
    if (defaultFramebuffer_) {
        glDeleteFramebuffers(1, &defaultFramebuffer_);
        defaultFramebuffer_ = 0;
    }

    if (colorRenderbuffer_) {
        glDeleteRenderbuffers(1, &colorRenderbuffer_);
        colorRenderbuffer_ = 0;
    }

    if( depthBuffer_ ) {
        glDeleteRenderbuffers(1, &depthBuffer_ );
        depthBuffer_ = 0;
    }
    
    if ( msaaColorbuffer_)
    {
        glDeleteRenderbuffers(1, &msaaColorbuffer_);
        msaaColorbuffer_ = 0;
    }
    
    if ( msaaFramebuffer_)
    {
        glDeleteRenderbuffers(1, &msaaFramebuffer_);
        msaaFramebuffer_ = 0;
    }

    // Tear down context
    if ([EAGLContext currentContext] == self.context)
        [EAGLContext setCurrentContext:nil];
    
    self.context = nil;
}

@end

