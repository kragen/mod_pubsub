// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__1DF8AAAD_2D72_445D_8A65_31C38D4E9988__INCLUDED_)
#define AFX_STDAFX_H__1DF8AAAD_2D72_445D_8A65_31C38D4E9988__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <tchar.h>
#include <wininet.h>
#include <stdio.h>

#include <cppunit\extensions\HelperMacros.h>
#include <cppunit\CompilerOutputter.h>
#include <cppunit\extensions\TestFactoryRegistry.h>
#include <cppunit\ui\text\TestRunner.h>

#pragma warning(disable: 4786)

#if defined(_DEBUG)
#	pragma comment(lib, "cppunitd.lib")
#else
#	pragma comment(lib, "cppunit.lib")
#endif

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__1DF8AAAD_2D72_445D_8A65_31C38D4E9988__INCLUDED_)
