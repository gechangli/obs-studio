#include "xl-frameless-window-util.hpp"
#include <QMenuBar>

#ifdef Q_OS_OSX

#include <Cocoa/Cocoa.h>

void XLFramelessWindowUtil::setupUI(QWidget* w) {
    QMainWindow* mw = dynamic_cast<QMainWindow*>(w);
    if(mw) {
		w->setWindowFlags(w->windowFlags() | Qt::FramelessWindowHint | Qt::Window);
		QMargins m = w->contentsMargins();
		m.setTop(m.top() + 48);
		w->setContentsMargins(m);

		// let window can be resized and minimized
		NSView *nativeView = reinterpret_cast<NSView*>(w->winId());
		NSWindow* nativeWindow = [nativeView window];
		[nativeWindow setStyleMask:[nativeWindow styleMask] | NSResizableWindowMask | NSMiniaturizableWindowMask];

		// XXX: This is old way to make a frameless resizable window
		// it is good, BUT, it causes a weird cpu hang-up, maybe NSFullSizeContentViewWindowMask
		// conflicts with Qt internal implementation, I don't know.
		// I have to comment those code and set content margins directly.
//		// set full size content
//        NSView *nativeView = reinterpret_cast<NSView *>(w->winId());
//        NSWindow* nativeWindow = [nativeView window];
//        [nativeWindow setStyleMask:[nativeWindow styleMask] | NSFullSizeContentViewWindowMask | NSWindowTitleHidden];
//        [nativeWindow setTitlebarAppearsTransparent:YES];
//
//        // hide standard buttons
//        [[nativeWindow standardWindowButton:NSWindowCloseButton] setHidden:YES];
//        [[nativeWindow standardWindowButton:NSWindowMiniaturizeButton] setHidden:YES];
//        [[nativeWindow standardWindowButton:NSWindowZoomButton] setHidden:YES];
//
//        // set central widget top margin based on menu property
//        QWidget *centralWidget = mw->centralWidget();
//        QMenuBar *menuBar = mw->menuBar();
//        if (menuBar && menuBar->isNativeMenuBar()) {
//            QMargins m = centralWidget->contentsMargins();
//            m.setTop(36);
//            centralWidget->setContentsMargins(m);
//        }
    } else {
        w->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        QMargins m = w->contentsMargins();
        m.setTop(m.top() + 32);
        w->setContentsMargins(m);
    }
}

#endif // #ifdef Q_OS_OSX