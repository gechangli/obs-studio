#import "ViewController.h"
#import "obs_app.hpp"

@interface ViewController()

@property (weak) IBOutlet NSButton *liveButton;

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
    if(obsApp->IsStreamingActive()) {
        obsApp->StopStreaming();
        self.liveButton.title = @"Push";
    } else {
        obsApp->StartStreaming("rtmp://send1.douyu.com/live", "2957252ruUkRlHvo?wsSecret=a303f4610c88e209dc79fa350ea08cd9&wsTime=59aa3479&wsSeek=off");
        self.liveButton.title = @"Stop";
    }
}

@end
