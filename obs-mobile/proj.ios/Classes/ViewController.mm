#import "ViewController.h"
#import <GLKit/GLKit.h>
#include "obs_app.hpp"

@interface ViewController ()

@property (weak, nonatomic) IBOutlet UIButton *pushButton;

- (IBAction)onPushClicked:(id)sender;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)onPushClicked:(id)sender {
    OBSApp* obsApp = OBSApp::sharedApp();
    if(obsApp->IsStreamingActive()) {
        obsApp->StopStreaming();
        self.pushButton.titleLabel.text = @"Push";
    } else {
        obsApp->StartStreaming("rtmp://send1.douyu.com/live", "2957252r44UxbR5k?wsSecret=3c301ca80d115fe8ff9544e814ba12e7&wsTime=5a312ac0&wsSeek=off");
        self.pushButton.titleLabel.text = @"Stop";
    }
}

@end
