#include <qsystemdetection.h>

#ifdef Q_OS_WIN

#include "xl-util.hpp"
#include <windows.h>

static BOOL CALLBACK enum_monitor_props(HMONITOR handle, HDC hdc, LPRECT rect, LPARAM param) {
	UNUSED_PARAMETER(handle);
	UNUSED_PARAMETER(hdc);
	UNUSED_PARAMETER(rect);

	// just increase count
	int* pc = (int*)param;
	(*pc)++;

	return TRUE;
}

int XLUtil::getMonitorCount() {
	int count = 0;
	EnumDisplayMonitors(NULL, NULL, enum_monitor_props, (LPARAM)&count);
	return count;
}

QPixmap XLUtil::fromNativeImage(void* p) {
	HICON icon = (HICON)p;
	bool foundAlpha = false;
	HDC screenDevice = GetDC(0);
	HDC hdc = CreateCompatibleDC(screenDevice);
	ReleaseDC(0, screenDevice);

	ICONINFO iconinfo;
	bool result = GetIconInfo(icon, &iconinfo); //x and y Hotspot describes the icon center
	if (!result)
		qWarning("XLUtil::fromNativeImage(), failed to GetIconInfo()");

	int w = iconinfo.xHotspot * 2;
	int h = iconinfo.yHotspot * 2;
	BITMAPINFOHEADER bitmapInfo;
	bitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.biWidth = w;
	bitmapInfo.biHeight = h;
	bitmapInfo.biPlanes = 1;
	bitmapInfo.biBitCount = 32;
	bitmapInfo.biCompression = BI_RGB;
	bitmapInfo.biSizeImage = 0;
	bitmapInfo.biXPelsPerMeter = 0;
	bitmapInfo.biYPelsPerMeter = 0;
	bitmapInfo.biClrUsed = 0;
	bitmapInfo.biClrImportant = 0;
	DWORD* bits;

	HBITMAP winBitmap = CreateDIBSection(hdc, (BITMAPINFO*)&bitmapInfo, DIB_RGB_COLORS, (VOID**)&bits, NULL, 0);
	HGDIOBJ oldhdc = (HBITMAP)SelectObject(hdc, winBitmap);
	DrawIconEx( hdc, 0, 0, icon, iconinfo.xHotspot * 2, iconinfo.yHotspot * 2, 0, 0, DI_NORMAL);
	QImage image = qt_fromWinHBITMAP(hdc, winBitmap, w, h);

	for (int y = 0 ; y < h && !foundAlpha ; y++) {
		QRgb *scanLine = reinterpret_cast(image.scanLine(y));
		for (int x = 0; x < w ; x++) {
			if (qAlpha(scanLine[x]) != 0) {
				foundAlpha = true;
				break;
			}
		}
	}

	// If no alpha was found, we use the mask to set alpha values
	if (!foundAlpha) {
		DrawIconEx( hdc, 0, 0, icon, w, h, 0, 0, DI_MASK);
		QImage mask = qt_fromWinHBITMAP(hdc, winBitmap, w, h);

		for (int y = 0 ; y < h ; y++) {
			QRgb* scanlineImage = reinterpret_cast(image.scanLine(y));
			QRgb* scanlineMask = mask.isNull() ? 0 : reinterpret_cast(mask.scanLine(y));
			for (int x = 0; x < w ; x++) {
				if (scanlineMask && qRed(scanlineMask[x]) != 0) {
					scanlineImage[x] = 0; //mask out this pixel
				} else {
					scanlineImage[x] |= 0xff000000; // set the alpha channel to 255
				}
			}
		}
	}

	// dispose resources created by iconinfo call
	DeleteObject(iconinfo.hbmMask);
	DeleteObject(iconinfo.hbmColor);

	SelectObject(hdc, oldhdc); //restore state
	DeleteObject(winBitmap);
	DeleteDC(hdc);

	// return
	return QPixmap::fromImage(image);
}

#endif