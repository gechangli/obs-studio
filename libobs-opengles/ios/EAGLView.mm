/*

===== IMPORTANT =====

This is sample code demonstrating API, technology or techniques in development.
Although this sample code has been reviewed for technical accuracy, it is not
final. Apple is supplying this information to help you plan for the adoption of
the technologies and programming interfaces described herein. This information
is subject to change, and software implemented based on this sample code should
be tested with final operating system software and final documentation. Newer
versions of this sample code may be provided with future seeds of the API or
technology. For information about updates to this and other developer
documentation, view the New & Updated sidebars in subsequent documentation
seeds.

=====================

File: EAGLView.m
Abstract: Convenience class that wraps the CAEAGLLayer from CoreAnimation into a
UIView subclass.

Version: 1.3

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc.
("Apple") in consideration of your agreement to the following terms, and your
use, installation, modification or redistribution of this Apple software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject
to these terms, Apple grants you a personal, non-exclusive license, under
Apple's copyrights in this original Apple software (the "Apple Software"), to
use, reproduce, modify and redistribute the Apple Software, with or without
modifications, in source and/or binary forms; provided that if you redistribute
the Apple Software in its entirety and without modifications, you must retain
this notice and the following text and disclaimers in all such redistributions
of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Inc. may be used
to endorse or promote products derived from the Apple Software without specific
prior written permission from Apple.  Except as expressly stated in this notice,
no other rights or licenses, express or implied, are granted by Apple herein,
including but not limited to any patent rights that may be infringed by your
derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR
DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF
CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF
APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2008 Apple Inc. All Rights Reserved.

*/

#import <QuartzCore/QuartzCore.h>
#import "EAGLView.h"
#import "CCES3Renderer.h"
#include "graphics.h"
#include "util/base.h"
#include "gl-helpers.h"

@interface EAGLView (Private)
- (BOOL) setupSurfaceWithSharegroup:(EAGLSharegroup*)sharegroup;
- (unsigned int) convertPixelFormat:(NSString*) pixelFormat;
@end

@implementation EAGLView

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

+ (id) viewWithFrame:(CGRect)frame
{
    return [[self alloc] initWithFrame:frame];
}

+ (id) viewWithFrame:(CGRect)frame pixelFormat:(NSString*)format
{
    return [[self alloc]initWithFrame:frame pixelFormat:format];
}

+ (id) viewWithFrame:(CGRect)frame pixelFormat:(NSString*)format depthFormat:(GLuint)depth
{
    return [[self alloc] initWithFrame:frame pixelFormat:format depthFormat:depth preserveBackbuffer:NO sharegroup:nil multiSampling:NO numberOfSamples:0];
}

+ (id) viewWithFrame:(CGRect)frame pixelFormat:(NSString*)format depthFormat:(GLuint)depth preserveBackbuffer:(BOOL)retained sharegroup:(EAGLSharegroup*)sharegroup multiSampling:(BOOL)multisampling numberOfSamples:(unsigned int)samples
{
    return [[self alloc]initWithFrame:frame pixelFormat:format depthFormat:depth preserveBackbuffer:retained sharegroup:sharegroup multiSampling:multisampling numberOfSamples:samples];
}

- (id) initWithFrame:(CGRect)frame
{
    return [self initWithFrame:frame pixelFormat:kEAGLColorFormatRGB565 depthFormat:0 preserveBackbuffer:NO sharegroup:nil multiSampling:NO numberOfSamples:0];
}

- (id) initWithFrame:(CGRect)frame pixelFormat:(NSString*)format 
{
    return [self initWithFrame:frame pixelFormat:format depthFormat:0 preserveBackbuffer:NO sharegroup:nil multiSampling:NO numberOfSamples:0];
}

- (id) initWithFrame:(CGRect)frame pixelFormat:(NSString*)format depthFormat:(GLuint)depth preserveBackbuffer:(BOOL)retained sharegroup:(EAGLSharegroup*)sharegroup multiSampling:(BOOL)sampling numberOfSamples:(unsigned int)nSamples;
{
    if((self = [super initWithFrame:frame]))
    {
        self.pixelFormat = format;
        self.depthFormat = depth;
        self.multiSampling = sampling;
        requestedSamples_ = nSamples;
        preserveBackbuffer_ = retained;

        if( ! [self setupSurfaceWithSharegroup:sharegroup] ) {
            return nil;
        }
    }
        
    return self;
}

-(id) initWithCoder:(NSCoder *)aDecoder
{
    if( (self = [super initWithCoder:aDecoder]) ) {
        
        CAEAGLLayer* eaglLayer = (CAEAGLLayer*)[self layer];
        self.pixelFormat = kEAGLColorFormatRGB565;
        self.depthFormat = 0; // GL_DEPTH_COMPONENT24;
        self.multiSampling = NO;
        requestedSamples_ = 0;
        self.surfaceSize = [eaglLayer bounds].size;
        
        if( ! [self setupSurfaceWithSharegroup:nil] ) {
            return nil;
        }
    }
    
    return self;
}

-(int) getWidth
{
    CGSize bound = [self bounds].size;
    return bound.width * self.contentScaleFactor;
}

-(int) getHeight
{
    CGSize bound = [self bounds].size;
    return bound.height * self.contentScaleFactor;
}


-(BOOL) setupSurfaceWithSharegroup:(EAGLSharegroup*)sharegroup
{
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
    
    eaglLayer.opaque = YES;
    eaglLayer.drawableProperties = @{ kEAGLDrawablePropertyRetainedBacking : [NSNumber numberWithBool:preserveBackbuffer_],
                                      kEAGLDrawablePropertyColorFormat : self.pixelFormat
                                    };
    
    
    renderer_ = [[CCES3Renderer alloc] initWithDepthFormat:self.depthFormat
                                         withPixelFormat:[self convertPixelFormat:self.pixelFormat]
                                          withSharegroup:sharegroup
                                       withMultiSampling:self.multiSampling
                                     withNumberOfSamples:requestedSamples_];
    
    NSAssert(renderer_, @"OpenGL ES 2.O is required.");
    if (!renderer_)
        return NO;
    
    self.context = [renderer_ context];
    

    //discardFramebufferSupported_ = [[CCConfiguration sharedConfiguration] supportsDiscardFramebuffer];
    gl_success("setupSurfaceWithSharegroup");
    
    return YES;
}

- (void) layoutSubviews
{
    [renderer_ resizeFromLayer:(CAEAGLLayer*)self.layer];
    self.surfaceSize = [renderer_ backingSize];
}

- (void) swapBuffers
{
    // IMPORTANT:
    // - preconditions
    //    -> context_ MUST be the OpenGL context
    //    -> renderbuffer_ must be the the RENDER BUFFER

#ifdef __IPHONE_4_0
    
    if (self.multiSampling)
    {
        /* Resolve from msaaFramebuffer to resolveFramebuffer */
        glBindFramebuffer(GL_READ_FRAMEBUFFER, [renderer_ msaaFrameBuffer]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, [renderer_ defaultFrameBuffer]);
        glBlitFramebuffer(0, 0, self.surfaceSize.width, self.surfaceSize.height,
                          0, 0, self.surfaceSize.width, self.surfaceSize.height,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
    
    if( discardFramebufferSupported_)
    {    
        if (self.multiSampling)
        {
            if (self.depthFormat)
            {
                GLenum attachments[] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT};
                glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 2, attachments);
            }
            else
            {
                GLenum attachments[] = {GL_COLOR_ATTACHMENT0};
                glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, attachments);
            }
            
            glBindRenderbuffer(GL_RENDERBUFFER, [renderer_ colorRenderBuffer]);
    
        }    
        
        // not MSAA
        else if (self.depthFormat ) {
            GLenum attachments[] = { GL_DEPTH_ATTACHMENT};
            glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments);
        }
    }
    
#endif // __IPHONE_4_0
    
     if(![self.context presentRenderbuffer:GL_RENDERBUFFER]) {
         blog(LOG_ERROR, "Failed to swap renderbuffer in %s\n", __FUNCTION__);
     }
    
    // We can safely re-bind the framebuffer here, since this will be the
    // 1st instruction of the new main loop
    if(self.multiSampling)
        glBindFramebuffer(GL_FRAMEBUFFER, [renderer_ msaaFrameBuffer]);    
}

- (unsigned int) convertPixelFormat:(NSString*) pixelFormat
{
    // define the pixel format
    GLenum pFormat;
    
    if([pixelFormat isEqualToString:kEAGLColorFormatRGB565])
        pFormat = GL_RGB565;
    else 
        pFormat = GL_RGBA8;
    
    return pFormat;
}

@end
