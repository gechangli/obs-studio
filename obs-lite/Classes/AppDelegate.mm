#import "AppDelegate.h"
#import "obs-module.h"
#import "ViewController.h"

OBS_DECLARE_STATIC_MODULE_CREATOR(coreaudio_encoder)
OBS_DECLARE_STATIC_MODULE_CREATOR(mac_avcapture)
OBS_DECLARE_STATIC_MODULE_CREATOR(mac_capture)
OBS_DECLARE_STATIC_MODULE_CREATOR(obs_outputs)
OBS_DECLARE_STATIC_MODULE_CREATOR(obs_transitions)
OBS_DECLARE_STATIC_MODULE_CREATOR(obs_x264)
OBS_DECLARE_STATIC_MODULE_CREATOR(rtmp_services)

static void loadStaticModules() {
    OBS_OPEN_STATIC_MODULE(coreaudio_encoder);
    OBS_OPEN_STATIC_MODULE(mac_avcapture);
    OBS_OPEN_STATIC_MODULE(mac_capture);
    OBS_OPEN_STATIC_MODULE(obs_outputs);
    OBS_OPEN_STATIC_MODULE(obs_transitions);
    OBS_OPEN_STATIC_MODULE(obs_x264);
    OBS_OPEN_STATIC_MODULE(rtmp_services);
}

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)dealloc {
    delete self.obsApp;
    self.obsApp = nullptr;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    // init obs app
    if(!self.obsApp) {
        /* use 1920x1080 for new default base res if main monitor is above
         * 1920x1080 */
        NSScreen* mainScreen = [NSScreen mainScreen];
        NSRect screenRect = [mainScreen visibleFrame];
        int sw = (int)screenRect.size.width;
        int sh = (int)screenRect.size.height;
        if ((sw * sh) > (1920 * 1080)) {
            sw = 1920;
            sh = 1080;
        }
        sw = 1280;
        sh = 720;
        
        // create app
        NSOpenGLView* glView = ((ViewController*)NSApp.mainWindow.contentViewController).glView;
        self.obsApp = new OBSApp(sw,
                                 sh,
                                 (int)glView.frame.size.width,
                                 (int)glView.frame.size.height);
        self.obsApp->RegisterStaticModuleLoader(loadStaticModules);
        self.obsApp->StartupOBS("zh_CN");
        self.obsApp->LoadDefaultScene();
        self.obsApp->CreateDisplay({ glView });
    }
}

@end
