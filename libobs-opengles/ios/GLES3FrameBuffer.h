#ifdef __APPLE__
#include <TargetConditionals.h>

#if TARGET_OS_IPHONE

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

@interface GLES3FrameBuffer : NSObject

/** Color Renderbuffer */
@property (nonatomic, assign) GLuint colorRenderbuffer;

/** Default Renderbuffer */
@property (nonatomic, assign) GLuint defaultFramebuffer;

/** MSAA Framebuffer */
@property (nonatomic, assign) GLuint msaaFramebuffer;

/** MSAA Color Buffer */
@property (nonatomic, assign) GLuint msaaColorbuffer;

/// depth buffer
@property (nonatomic, assign) GLuint depthBuffer;

/// depth format
@property (nonatomic, assign) unsigned int depthFormat;

/// pixel format
@property (nonatomic, assign) unsigned int pixelFormat;

/// sample count
@property (nonatomic, assign) unsigned int samplesToUse;

/// multisample or not
@property (nonatomic, assign) BOOL multiSampling;

/// context
@property (strong, nonatomic) EAGLContext* context;

// layer size
@property (nonatomic, assign) CGSize backingSize;

- (BOOL)resizeFromLayer:(CAEAGLLayer *)layer;

- (id) initWithDepthFormat:(unsigned int)depthFormat withPixelFormat:(unsigned int)pixelFormat withSharegroup:(EAGLSharegroup*)sharegroup withMultiSampling:(BOOL) multiSampling withNumberOfSamples:(unsigned int) requestedSamples;

@end

#endif // #if TARGET_OS_IPHONE

#endif // #ifdef __APPLE__
