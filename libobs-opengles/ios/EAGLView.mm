#ifdef __APPLE__
#include <TargetConditionals.h>

#if TARGET_OS_IPHONE

#import <QuartzCore/QuartzCore.h>
#import "EAGLView.h"
#import "GLES3FrameBuffer.h"
#include "graphics.h"
#include "util/base.h"
#include "gl-helpers.h"

static EAGLView *view = 0;

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
        
        view = self;
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
        
        view = self;
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
    
    
    self.renderer = [[GLES3FrameBuffer alloc] initWithDepthFormat:self.depthFormat
                                         withPixelFormat:[self convertPixelFormat:self.pixelFormat]
                                          withSharegroup:sharegroup
                                       withMultiSampling:self.multiSampling
                                     withNumberOfSamples:requestedSamples_];
    
    NSAssert(self.renderer, @"OpenGL ES 3.O is required.");
    if (!self.renderer)
        return NO;
    
    self.context = [self.renderer context];
    
    gl_success("setupSurfaceWithSharegroup");
    
    return YES;
}

+ (id) sharedEGLView
{
    return view;
}

- (void) layoutSubviews
{
    [self.renderer resizeFromLayer:(CAEAGLLayer*)self.layer];
    self.surfaceSize = [self.renderer backingSize];
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
        glBindFramebuffer(GL_READ_FRAMEBUFFER, self.renderer.msaaFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, self.renderer.defaultFramebuffer);
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
            
            glBindRenderbuffer(GL_RENDERBUFFER, self.renderer.colorRenderbuffer);
    
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
        glBindFramebuffer(GL_FRAMEBUFFER, self.renderer.msaaFramebuffer);
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

#endif // #if TARGET_OS_IPHONE

#endif // #ifdef __APPLE__
