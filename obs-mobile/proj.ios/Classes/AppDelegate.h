#import <UIKit/UIKit.h>
#import "obs.h"
#import "obs_app.hpp"

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;
@property (nonatomic, assign) OBSApp* obsApp;

@end

