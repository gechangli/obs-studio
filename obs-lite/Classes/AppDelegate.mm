#import "AppDelegate.h"
#import "obs-module.h"
#import "ViewController.h"

OBS_DECLARE_STATIC_MODULE_CREATOR(mac_avcapture)
OBS_DECLARE_STATIC_MODULE_CREATOR(rtmp_services)

static void loadStaticModules() {
    OBS_OPEN_STATIC_MODULE(mac_avcapture);
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
    self.obsApp = new OBSApp(nullptr);
    self.obsApp->RegisterStaticModuleLoader(loadStaticModules);
    self.obsApp->StartupOBS("zh_CN");
    NSOpenGLView* glView = ((ViewController*)NSApp.mainWindow.contentViewController).glView;
    self.obsApp->ResetVideo((int)glView.frame.size.width, (int)glView.frame.size.height);
}

@end
