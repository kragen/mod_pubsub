/*
 * File to have Windows CE Toolkit for VC++ 5.0 working with STL
 * 09 - 03 - 1999
 * Giuseppe Govi - g.govi@iol.it
 */

#ifndef INC_WCE_DEFS_H
#define INC_WCE_DEFS_H

#if defined (UNDER_CE)

#include <windows.h>

#ifdef __STL_WCE_USE_OUTPUTDEBUGSTRING
#define STLTRACE(msg)   OutputDebugString(msg)
#else
#define STLTRACE(msg)   MessageBox(NULL,(msg),NULL,MB_OK)
#endif

#define abort()	TerminateProcess(GetCurrentProcess(), 0)

#ifndef __THROW_BAD_ALLOC
#define __THROW_BAD_ALLOC STLTRACE(L"out of memory"); ExitThread(1)
#endif

template <class charT> //charT == TCHAR under Widnows CE
void wce_assert(bool cond, charT* file, int line, charT* exp)
{
charT buffer[512];
	if (!cond)
	{
		wsprintf(buffer, _T("%s:%d assertion failure:\n%s"), file, line, exp);
		if (MessageBox(NULL, buffer, NULL, MB_RETRYCANCEL) == IDCANCEL)
			abort();
	}
}

#define assert(expr)	wce_assert<TCHAR>((expr), TEXT(__FILE__), __LINE__, L# expr)

#ifndef __PLACEMENT_NEW_INLINE
inline void *__cdecl operator new(size_t, void *_P)
        {return (_P); }
#define __PLACEMENT_NEW_INLINE
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

//ptrdiff_t is not defined in Windows CE SDK
#ifndef _PTRDIFF_T_DEFINED
typedef int ptrdiff_t;
#define _PTRDIFF_T_DEFINED
#endif

#endif //UNDER_CE

#endif //INC_WCE_DEFS_H


