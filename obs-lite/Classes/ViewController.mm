#import "ViewController.h"
#import "obs_app.hpp"

@interface ViewController()

- (IBAction)onPushClicked:(id)sender;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (IBAction)onPushClicked:(id)sender {
    OBSApp* obsApp = OBSApp::sharedApp();
    obsApp->StartStreaming("rtmp://send1.douyu.com/live", "2957252rJyQceiGP?wsSecret=a6d92f0f7ce2bc87077f39b7d79ce0e1&wsTime=59a98a40&wsSeek=off");
}

@end
