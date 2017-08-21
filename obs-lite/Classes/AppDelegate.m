//
//  AppDelegate.m
//  obs-lite
//
//  Created by MaRuoJie on 2017/8/21.
//  Copyright © 2017年 thu. All rights reserved.
//

#import "AppDelegate.h"
#import "obs.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    
    // load all obs modules
    obs_load_all_modules();
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


@end
