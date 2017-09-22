#import "AppDelegate.h"
#import "ViewController.h"

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
        
        // create app
        NSOpenGLView* glView = ((ViewController*)NSApp.mainWindow.contentViewController).glView;
        self.obsApp = new OBSApp(sw,
                                 sh,
                                 (int)glView.frame.size.width,
                                 (int)glView.frame.size.height);
        self.obsApp->StartupOBS("zh_CN");
        self.obsApp->LoadDefaultScene();
        self.obsApp->CreateDisplay({ glView });
    }
}

@end
