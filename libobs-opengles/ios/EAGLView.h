#ifdef __APPLE__
#include <TargetConditionals.h>

#if TARGET_OS_IPHONE

#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import <CoreFoundation/CoreFoundation.h>
#import "GLES3FrameBuffer.h"

/** EAGLView Class.
 * This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
 * The view content is basically an EAGL surface you render your OpenGL scene into.
 * Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
 */
@interface EAGLView : UIView
{
    BOOL                    preserveBackbuffer_;
    BOOL                    discardFramebufferSupported_;

    //fsaa addition
    unsigned int               requestedSamples_;
}

/** creates an initializes an EAGLView with a frame and 0-bit depth buffer, and a RGB565 color buffer */
+ (id) viewWithFrame:(CGRect)frame;
/** creates an initializes an EAGLView with a frame, a color buffer format, and 0-bit depth buffer */
+ (id) viewWithFrame:(CGRect)frame pixelFormat:(NSString*)format;
/** creates an initializes an EAGLView with a frame, a color buffer format, and a depth buffer format */
+ (id) viewWithFrame:(CGRect)frame pixelFormat:(NSString*)format depthFormat:(GLuint)depth;
/** creates an initializes an EAGLView with a frame, a color buffer format, a depth buffer format, a sharegroup, and multisamping */
+ (id) viewWithFrame:(CGRect)frame pixelFormat:(NSString*)format depthFormat:(GLuint)depth preserveBackbuffer:(BOOL)retained sharegroup:(EAGLSharegroup*)sharegroup multiSampling:(BOOL)multisampling numberOfSamples:(unsigned int)samples;

/** Initializes an EAGLView with a frame and 0-bit depth buffer, and a RGB565 color buffer */
- (id) initWithFrame:(CGRect)frame; //These also set the current context
/** Initializes an EAGLView with a frame, a color buffer format, and 0-bit depth buffer */
- (id) initWithFrame:(CGRect)frame pixelFormat:(NSString*)format;
/** Initializes an EAGLView with a frame, a color buffer format, a depth buffer format, a sharegroup and multisampling support */
- (id) initWithFrame:(CGRect)frame pixelFormat:(NSString*)format depthFormat:(GLuint)depth preserveBackbuffer:(BOOL)retained sharegroup:(EAGLSharegroup*)sharegroup multiSampling:(BOOL)sampling numberOfSamples:(unsigned int)nSamples;

/** pixel format: it could be RGBA8 (32-bit) or RGB565 (16-bit) */
@property(nonatomic, strong) NSString* pixelFormat;
/** depth format of the render buffer: 0, 16 or 24 bits*/
@property(nonatomic, assign) GLuint depthFormat;

/** returns surface size in pixels */
@property(nonatomic, assign) CGSize surfaceSize;

/** OpenGL context */
@property(nonatomic, strong) EAGLContext *context;

/// renderer
@property (nonatomic, strong) GLES3FrameBuffer* renderer;

@property(nonatomic, assign) BOOL multiSampling;

// get the view object
+ (id)sharedEGLView;

/** EAGLView uses double-buffer. This method swaps the buffers */
- (void) swapBuffers;

-(int) getWidth;
-(int) getHeight;

@end

#endif // #if TARGET_OS_IPHONE

#endif // #ifdef __APPLE__
