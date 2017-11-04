#include <qsystemdetection.h>

#ifdef Q_OS_OSX

#include "xl-util.hpp"
#import <AppKit/AppKit.h>

int XLUtil::getMonitorCount() {
	return [NSScreen screens].count;
}

#endif