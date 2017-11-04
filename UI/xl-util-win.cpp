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

#endif