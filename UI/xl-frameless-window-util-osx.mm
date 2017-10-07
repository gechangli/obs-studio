#include "xl-frameless-window-util.hpp"
#include <QMenuBar>

#ifdef Q_OS_OSX

#include <Cocoa/Cocoa.h>

void XLFramelessWindowUtil::setupUI(QMainWindow* w) {
    // set full size content
    NSView *nativeView = reinterpret_cast<NSView *>(w->winId());
    NSWindow* nativeWindow = [nativeView window];
    [nativeWindow setStyleMask:[nativeWindow styleMask] | NSFullSizeContentViewWindowMask | NSWindowTitleHidden];
    [nativeWindow setTitlebarAppearsTransparent:YES];

    // hide standard buttons
    [[nativeWindow standardWindowButton:NSWindowCloseButton] setHidden:YES];
    [[nativeWindow standardWindowButton:NSWindowMiniaturizeButton] setHidden:YES];
    [[nativeWindow standardWindowButton:NSWindowZoomButton] setHidden:YES];

    // set central widget top margin based on menu property
    QWidget* centralWidget = w->centralWidget();
    QMenuBar* menuBar = w->menuBar();
    if(menuBar && menuBar->isNativeMenuBar()) {
        QMargins m = centralWidget->contentsMargins();
        m.setTop(36);
        centralWidget->setContentsMargins(m);
    }
}

#endif // #ifdef Q_OS_OSX