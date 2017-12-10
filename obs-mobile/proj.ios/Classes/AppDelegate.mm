#import "AppDelegate.h"
#import "ViewController.h"
#import <GLKit/GLKit.h>
#import "obs.h"
#import "obs_app.hpp"

@interface AppDelegate()

@property (nonatomic, assign) OBSApp* obsApp;

@end

@implementation AppDelegate

- (void)dealloc {
    delete self.obsApp;
    self.obsApp = nullptr;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    // init obs app
    if(!self.obsApp) {
        /* use 1920x1080 for new default base res if main monitor is above
         * 1920x1080 */
        UIScreen* mainScreen = [UIScreen mainScreen];
        CGRect screenRect = [mainScreen nativeBounds];
        int sw = (int)screenRect.size.width;
        int sh = (int)screenRect.size.height;
        if ((sw * sh) > (1920 * 1080)) {
            sw = 1920;
            sh = 1080;
        }
        
        // create app
        UIApplication* app = [UIApplication sharedApplication];
        GLKView* glView = (GLKView*)((ViewController*)app.keyWindow.rootViewController).view;
        self.obsApp = new OBSApp(sw,
                                 sh,
                                 (int)glView.frame.size.width,
                                 (int)glView.frame.size.height);
        self.obsApp->StartupOBS("zh-CN");
        self.obsApp->LoadDefaultScene();
        self.obsApp->CreateDisplay({ glView });
    }
}


- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}


@end
