// Pocket PC extensions for WTL for Windows CE
// Author: Herbert Fann (hhfann@rogers.com)
//
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
//

#ifndef __WTLCE_H__
#define __WTLCE_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

namespace WTL
{
	// Load an empty Pocket PC Command Bar
	inline BOOL AtlLoadEmptyBar(HWND hwnd) {
		ATLASSERT(hwnd != NULL);
		SHMENUBARINFO mbi;
		memset(&mbi, 0, sizeof(SHMENUBARINFO));
		mbi.cbSize = sizeof(SHMENUBARINFO);

		mbi.dwFlags = SHCMBF_EMPTYBAR;
		mbi.hwndParent = hwnd;
		mbi.nToolBarId = 0;
		mbi.hInstRes = _Module.GetModuleInstance();

		if(!SHCreateMenuBar(&mbi)) 
			return FALSE;

		CommandBar_Show(mbi.hwndMB, TRUE);
		return (mbi.hwndMB != NULL);
	}

// These macros are for reflected messages
#define REFLECT_COMMAND_ID_HANDLER(id, func) \
	if(uMsg == OCM_COMMAND && id == LOWORD(wParam)) \
	{ \
		bHandled = TRUE; \
		lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define REFLECT_COMMAND_CODE_HANDLER(code, func) \
	if(uMsg == OCM_COMMAND && code == HIWORD(wParam)) \
	{ \
		bHandled = TRUE; \
		lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}
#define REFLECT_COMMAND_RANGE_HANDLER(idFirst, idLast, func) \
	if(uMsg == OCM_COMMAND && LOWORD(wParam) >= idFirst  && LOWORD(wParam) <= idLast) \
	{ \
		bHandled = TRUE; \
		lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}
};

#endif
