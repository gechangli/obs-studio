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
    
    // init gl view
    GLKView* glView = (GLKView*)self.view;
    glView.delegate = self;
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    NSLog(@"ggg");
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
        obsApp->StartStreaming("rtmp://send1.douyu.com/live", "2957252ruUkRlHvo?wsSecret=a303f4610c88e209dc79fa350ea08cd9&wsTime=59aa3479&wsSeek=off");
        self.pushButton.titleLabel.text = @"Stop";
    }
}

@end
