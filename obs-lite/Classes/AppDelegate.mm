//
//  AppDelegate.m
//  obs-lite
//
//  Created by MaRuoJie on 2017/8/21.
//  Copyright © 2017年 thu. All rights reserved.
//

#import "AppDelegate.h"
#import "obs.h"
#import "obs_app.hpp"

@interface AppDelegate ()

@property (nonatomic, assign) OBSApp* obsApp;

@end

@implementation AppDelegate

- (void)dealloc {
    delete self.obsApp;
    self.obsApp = nullptr;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    
    // init obs app
    self.obsApp = new OBSApp(nullptr);
    self.obsApp->StartupOBS("en_US");
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


@end