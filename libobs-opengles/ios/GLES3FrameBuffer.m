#ifdef __APPLE__
#include <TargetConditionals.h>

#if TARGET_OS_IPHONE

#import "GLES3FrameBuffer.h"
#include "graphics.h"
#include "util/base.h"
#include "gl-helpers.h"

@implementation GLES3FrameBuffer

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
        
        self.depthFormat = depthFormat;
        self.pixelFormat = pixelFormat;
        self.multiSampling = multiSampling;

        // Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
        glGenFramebuffers(1, &_defaultFramebuffer);
        NSAssert( self.defaultFramebuffer, @"Can't create default frame buffer");

        glGenRenderbuffers(1, &_colorRenderbuffer);
        NSAssert( self.colorRenderbuffer, @"Can't create default render buffer");

        glBindFramebuffer(GL_FRAMEBUFFER, self.defaultFramebuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, self.colorRenderbuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, self.colorRenderbuffer);

        if (self.multiSampling)
        {
            GLint maxSamplesAllowed;
            glGetIntegerv(GL_MAX_SAMPLES, &maxSamplesAllowed);
            self.samplesToUse = MIN(maxSamplesAllowed, requestedSamples);
            
            /* Create the MSAA framebuffer (offscreen) */
            glGenFramebuffers(1, &_msaaFramebuffer);
            NSAssert( self.msaaFramebuffer, @"Can't create default MSAA frame buffer");
            glBindFramebuffer(GL_FRAMEBUFFER, self.msaaFramebuffer);
        }

        gl_success("GLES3FrameBuffer::initWithDepthFormat");
    }

    return self;
}

- (BOOL)resizeFromLayer:(CAEAGLLayer *)layer
{
    // Allocate color buffer backing based on the current layer size
    glBindRenderbuffer(GL_RENDERBUFFER, self.colorRenderbuffer);

    if( ! [self.context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer] )
    {
        NSLog(@"failed to call context");
    }
    
    GLint w, h;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &w);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &h);
    self.backingSize = CGSizeMake(w, h);

    if (self.multiSampling)
    {
        if ( self.msaaColorbuffer) {
            glDeleteRenderbuffers(1, &_msaaColorbuffer);
            self.msaaColorbuffer = 0;
        }
        
        /* Create the offscreen MSAA color buffer.
         After rendering, the contents of this will be blitted into ColorRenderbuffer */
        
        //msaaFrameBuffer needs to be binded
        glBindFramebuffer(GL_FRAMEBUFFER, self.msaaFramebuffer);
        glGenRenderbuffers(1, &_msaaColorbuffer);
        NSAssert(self.msaaFramebuffer, @"Can't create MSAA color buffer");
        
        glBindRenderbuffer(GL_RENDERBUFFER, self.msaaColorbuffer);
        
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, self.samplesToUse, self.pixelFormat, (GLint)self.backingSize.width, (GLint)self.backingSize.height);
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, self.msaaColorbuffer);
        
        GLenum error;
        if ( (error=glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
        {
            NSLog(@"Failed to make complete framebuffer object 0x%X", error);
            return NO;
        }
    }

    if (self.depthFormat)
    {
        if( ! self.depthBuffer ) {
            glGenRenderbuffers(1, &_depthBuffer);
            NSAssert(self.depthBuffer, @"Can't create depth buffer");
        }

        glBindRenderbuffer(GL_RENDERBUFFER, self.depthBuffer);
        
        if( self.multiSampling )
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, self.samplesToUse, self.depthFormat, (GLint)self.backingSize.width, (GLint)self.backingSize.height);
        else
            glRenderbufferStorage(GL_RENDERBUFFER, self.depthFormat, (GLint)self.backingSize.width, (GLint)self.backingSize.height);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, self.depthBuffer);
        
        if (self.depthFormat == GL_DEPTH24_STENCIL8) {
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, self.depthBuffer);
		}

        // bind color buffer
        glBindRenderbuffer(GL_RENDERBUFFER, self.colorRenderbuffer);        
    }

    gl_success("resizeFromLayer");

    GLenum error;
    if( (error=glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
        blog(LOG_ERROR, "Failed to make complete framebuffer object 0x%X", error);
        return NO;
    }

    return YES;
}

- (NSString*)description {
    return [NSString stringWithFormat:@"<%@ = %08X | size = %ix%i>", [self class], (unsigned int)self, (int)self.backingSize.width, (int)self.backingSize.height];
}

- (void)dealloc {
    // Tear down GL
    if (self.defaultFramebuffer) {
        glDeleteFramebuffers(1, &_defaultFramebuffer);
        self.defaultFramebuffer = 0;
    }

    if (self.colorRenderbuffer) {
        glDeleteRenderbuffers(1, &_colorRenderbuffer);
        self.colorRenderbuffer = 0;
    }

    if( self.depthBuffer ) {
        glDeleteRenderbuffers(1, &_depthBuffer);
        self.depthBuffer = 0;
    }
    
    if ( self.msaaColorbuffer)
    {
        glDeleteRenderbuffers(1, &_msaaColorbuffer);
        self.msaaColorbuffer = 0;
    }
    
    if ( self.msaaFramebuffer)
    {
        glDeleteRenderbuffers(1, &_msaaFramebuffer);
        self.msaaFramebuffer = 0;
    }

    // Tear down context
    if ([EAGLContext currentContext] == self.context)
        [EAGLContext setCurrentContext:nil];
    
    self.context = nil;
}

@end

#endif // #if TARGET_OS_IPHONE

#endif // #ifdef __APPLE__
