#include <qsystemdetection.h>

#ifdef Q_OS_WIN

#include "xl-util.hpp"
#include <windows.h>
#include <QtWin>
#include <util/dstr.h>

#ifdef _WIN64
	#define GCL_HICON GCLP_HICON
	#define GCL_HICONSM GCLP_HICONSM
#endif

enum window_priority {
	WINDOW_PRIORITY_CLASS,
	WINDOW_PRIORITY_TITLE,
	WINDOW_PRIORITY_EXE,
};
enum window_search_mode {
	INCLUDE_MINIMIZED,
	EXCLUDE_MINIMIZED
};
extern void build_window_strings(const char *str,
		char **klass,
		char **title,
		char **exe);
extern HWND find_window(enum window_search_mode mode,
		enum window_priority priority,
		const char *klass,
		const char *title,
		const char *exe);

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
	return QtWin::fromHICON(icon);
}

QPixmap XLUtil::getWindowIcon(const char* winStr) {
	// extract window info
	char *klass = NULL;
	char *title = NULL;
	char *exe = NULL;
	build_window_strings(winStr, &klass, &title, &exe);

	// find window
	QPixmap pix;
	HWND wnd = find_window(EXCLUDE_MINIMIZED, WINDOW_PRIORITY_EXE, klass, title, exe);

	// extract icon from HWND
	if(wnd == NULL) {
		return pix;
	} else {
		HICON icon = (HICON)GetClassLong(wnd, GCL_HICON);
		if(icon != NULL) {
			pix = fromNativeImage((void*)icon);
		}
	}

	// return
	return pix;
}

#endif