#include <qsystemdetection.h>

#ifdef Q_OS_OSX

#include "xl-util.hpp"
#import <AppKit/AppKit.h>
#include <QtMac>

int XLUtil::getMonitorCount() {
	return [NSScreen screens].count;
}

QPixmap XLUtil::fromNativeImage(void* p) {
	// create a bitmap rep
	// in OSX, p is a NSImage
	NSImage* img = (__bridge NSImage*)p;
	NSSize size = img.size;
	NSBitmapImageRep* bmp = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:nil
		pixelsWide:size.width
		pixelsHigh:size.height
		bitsPerSample:8
		samplesPerPixel:4
		hasAlpha:YES
		isPlanar:NO
		colorSpaceName:NSDeviceRGBColorSpace
		bitmapFormat:NSAlphaFirstBitmapFormat
		bytesPerRow:0
		bitsPerPixel:0];

	// draw NSImage into the rep
	[NSGraphicsContext saveGraphicsState];
	[NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:bmp]];
	[img drawInRect:NSMakeRect(0, 0, size.width, size.height)
		fromRect:NSZeroRect
		operation:NSCompositeSourceOver
		fraction:1];
	[NSGraphicsContext restoreGraphicsState];

	// create pixmap from CGImage
	QPixmap pix = QtMac::fromCGImageRef([bmp CGImage]);

	// release if no ARC
#if !__has_feature(objc_arc)
	[bmp release];
#endif

	// return
	return pix;
}

#endif