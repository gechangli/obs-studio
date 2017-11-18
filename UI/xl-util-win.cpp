#include <qsystemdetection.h>

#ifdef Q_OS_WIN

#include "xl-util.hpp"
#include <windows.h>
#include <psapi.h>
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

#define LOWER_HALFBYTE(x) ((x) & 0xF)
#define UPPER_HALFBYTE(x) (((x) >> 4) & 0xF)

static void deobfuscate_str(char *str, uint64_t val)
{
		uint8_t *dec_val = (uint8_t*)&val;
		int i = 0;

		while (*str != 0) {
				int pos = i / 2;
				bool bottom = (i % 2) == 0;
				uint8_t *ch = (uint8_t*)str;
				uint8_t xor = bottom ?
						LOWER_HALFBYTE(dec_val[pos]) :
						UPPER_HALFBYTE(dec_val[pos]);

				*ch ^= xor;

				if (++i == sizeof(uint64_t) * 2)
						i = 0;

				str++;
		}
}

static void *get_obfuscated_func(HMODULE module, const char *str, uint64_t val)
{
		char new_name[128];
		strcpy(new_name, str);
		deobfuscate_str(new_name, val);
		return GetProcAddress(module, new_name);
}

static HMODULE kernel32(void)
{
		static HMODULE kernel32_handle = NULL;
		if (!kernel32_handle)
				kernel32_handle = GetModuleHandleA("kernel32");
		return kernel32_handle;
}

typedef HANDLE(WINAPI *OpenProcessProc)(DWORD, BOOL, DWORD);
static inline HANDLE open_process(DWORD desired_access, bool inherit_handle,
		DWORD process_id) {
		static OpenProcessProc open_process_proc = NULL;
		if (!open_process_proc)
				open_process_proc = (OpenProcessProc)get_obfuscated_func(kernel32(),
						"B}caZyah`~q", 0x2D5BEBAF6DDULL);

		return open_process_proc(desired_access, inherit_handle, process_id);
}

static inline char *decode_str(const char *src) {
		struct dstr str = { 0 };
		dstr_copy(&str, src);
		dstr_replace(&str, "#3A", ":");
		dstr_replace(&str, "#22", "#");
		return str.array;
}

static void build_window_strings(const char *str,
		char **klass,
		char **title,
		char **exe) {
	char **strlist;

	*klass = NULL;
	*title = NULL;
	*exe   = NULL;

	if (!str) {
		return;
	}

	strlist = strlist_split(str, ':', true);

	if (strlist && strlist[0] && strlist[1] && strlist[2]) {
		*title = decode_str(strlist[0]);
		*klass = decode_str(strlist[1]);
		*exe   = decode_str(strlist[2]);
	}

	strlist_free(strlist);
}

static bool get_window_exe(struct dstr *name, HWND window) {
	wchar_t     wname[MAX_PATH];
	struct dstr temp    = {0};
	bool        success = false;
	HANDLE      process = NULL;
	char        *slash;
	DWORD       id;

	GetWindowThreadProcessId(window, &id);
	if (id == GetCurrentProcessId())
		return false;

	process = open_process(PROCESS_QUERY_LIMITED_INFORMATION, false, id);
	if (!process)
		goto fail;

	if (!GetProcessImageFileNameW(process, wname, MAX_PATH))
		goto fail;

	dstr_from_wcs(&temp, wname);
	slash = strrchr(temp.array, '\\');
	if (!slash)
		goto fail;

	dstr_copy(name, slash+1);
	success = true;

fail:
	if (!success)
		dstr_copy(name, "unknown");

	dstr_free(&temp);
	CloseHandle(process);
	return true;
}

static void get_window_title(struct dstr *name, HWND hwnd) {
	wchar_t *temp;
	int len;

	len = GetWindowTextLengthW(hwnd);
	if (!len)
		return;

	temp = (wchar_t*)malloc(sizeof(wchar_t) * (len+1));
	if (GetWindowTextW(hwnd, temp, len+1))
		dstr_from_wcs(name, temp);
	free(temp);
}

static void get_window_class(struct dstr *klass, HWND hwnd) {
	wchar_t temp[256];

	temp[0] = 0;
	if (GetClassNameW(hwnd, temp, sizeof(temp) / sizeof(wchar_t)))
		dstr_from_wcs(klass, temp);
}

static int window_rating(HWND window,
		enum window_priority priority,
		const char *klass,
		const char *title,
		const char *exe,
		bool uwp_window) {
	struct dstr cur_class = {0};
	struct dstr cur_title = {0};
	struct dstr cur_exe   = {0};
	int val = 0x7FFFFFFF;

	if (!get_window_exe(&cur_exe, window))
		return 0x7FFFFFFF;
	get_window_title(&cur_title, window);
	get_window_class(&cur_class, window);

	bool class_matches = dstr_cmpi(&cur_class, klass) == 0;
	bool exe_matches = dstr_cmpi(&cur_exe, exe) == 0;
	int title_val = abs(dstr_cmpi(&cur_title, title));

	/* always match by name with UWP windows */
	if (uwp_window) {
		if (priority == WINDOW_PRIORITY_EXE && !exe_matches)
			val = 0x7FFFFFFF;
		else
			val = title_val == 0 ? 0 : 0x7FFFFFFF;

	} else if (priority == WINDOW_PRIORITY_CLASS) {
		val = class_matches ? title_val : 0x7FFFFFFF;
		if (val != 0x7FFFFFFF && !exe_matches)
			val += 0x1000;

	} else if (priority == WINDOW_PRIORITY_TITLE) {
		val = title_val == 0 ? 0 : 0x7FFFFFFF;

	} else if (priority == WINDOW_PRIORITY_EXE) {
		val = exe_matches ? title_val : 0x7FFFFFFF;
	}

	dstr_free(&cur_class);
	dstr_free(&cur_title);
	dstr_free(&cur_exe);

	return val;
}

static HWND get_uwp_actual_window(HWND parent) {
	DWORD parent_id = 0;
	HWND child;

	GetWindowThreadProcessId(parent, &parent_id);
	child = FindWindowEx(parent, NULL, NULL, NULL);

	while (child) {
		DWORD child_id = 0;
		GetWindowThreadProcessId(child, &child_id);

		if (child_id != parent_id)
			return child;

		child = FindWindowEx(parent, child, NULL, NULL);
	}

	return NULL;
}

static bool is_uwp_window(HWND hwnd) {
	wchar_t name[256];

	name[0] = 0;
	if (!GetClassNameW(hwnd, name, sizeof(name) / sizeof(wchar_t)))
		return false;

	return wcscmp(name, L"ApplicationFrameWindow") == 0;
}

static bool check_window_valid(HWND window, enum window_search_mode mode) {
	DWORD styles, ex_styles;
	RECT  rect;

	if (!IsWindowVisible(window) ||
	    (mode == EXCLUDE_MINIMIZED && IsIconic(window)))
		return false;

	GetClientRect(window, &rect);
	styles    = (DWORD)GetWindowLongPtr(window, GWL_STYLE);
	ex_styles = (DWORD)GetWindowLongPtr(window, GWL_EXSTYLE);

	if (ex_styles & WS_EX_TOOLWINDOW)
		return false;
	if (styles & WS_CHILD)
		return false;
	if (mode == EXCLUDE_MINIMIZED && (rect.bottom == 0 || rect.right == 0))
		return false;

	return true;
}

static HWND next_window(HWND window, enum window_search_mode mode,
		HWND *parent, bool use_findwindowex) {
	if (*parent) {
		window = *parent;
		*parent = NULL;
	}

	while (true) {
		if (use_findwindowex)
			window = FindWindowEx(GetDesktopWindow(), window, NULL,
					NULL);
		else
			window = GetNextWindow(window, GW_HWNDNEXT);

		if (!window || check_window_valid(window, mode))
			break;
	}

	if (is_uwp_window(window)) {
		HWND child = get_uwp_actual_window(window);
		if (child) {
			*parent = window;
			return child;
		}
	}

	return window;
}

static HWND first_window(enum window_search_mode mode, HWND *parent,
		bool *use_findwindowex) {
	HWND window = FindWindowEx(GetDesktopWindow(), NULL, NULL, NULL);

	if (!window) {
		*use_findwindowex = false;
		window = GetWindow(GetDesktopWindow(), GW_CHILD);
	} else {
		*use_findwindowex = true;
	}

	*parent = NULL;

	if (!check_window_valid(window, mode)) {
		window = next_window(window, mode, parent, *use_findwindowex);

		if (!window && *use_findwindowex) {
			*use_findwindowex = false;

			window = GetWindow(GetDesktopWindow(), GW_CHILD);
			if (!check_window_valid(window, mode))
				window = next_window(window, mode, parent,
						*use_findwindowex);
		}
	}

	if (is_uwp_window(window)) {
		HWND child = get_uwp_actual_window(window);
		if (child) {
			*parent = window;
			return child;
		}
	}

	return window;
}

static HWND find_window(enum window_search_mode mode,
		enum window_priority priority,
		const char *klass,
		const char *title,
		const char *exe) {
	HWND parent;
	bool use_findwindowex = false;

	HWND window      = first_window(mode, &parent, &use_findwindowex);
	HWND best_window = NULL;
	int  best_rating = 0x7FFFFFFF;

	if (!klass)
		return NULL;

	bool uwp_window  = strcmp(klass, "Windows.UI.Core.CoreWindow") == 0;

	while (window) {
		int rating = window_rating(window, priority, klass, title, exe,
				uwp_window);
		if (rating < best_rating) {
			best_rating = rating;
			best_window = window;
			if (rating == 0)
				break;
		}

		window = next_window(window, mode, &parent, use_findwindowex);
	}

	return best_window;
}

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