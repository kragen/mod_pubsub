// LoggerCfg.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MainWnd.h"

CAppModule _Module;

int WINAPI _tWinMain (HINSTANCE hinst, HINSTANCE hinstPrev, LPTSTR lpCmdLine, int nCmdShow)
{
	_Module.Init(0, hinst);
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	TMainWindow mainWnd;

	theLoop.AddMessageFilter(&mainWnd);

#if defined(_WIN32_WCE)
	CRect r = CWindow::rcDefault;
#else
	CRect r(0, 0, 240, 320);
#endif

	mainWnd.Create(0, r, _T("Main"), WS_OVERLAPPEDWINDOW | WS_VISIBLE);
	mainWnd.ShowWindow(nCmdShow);

	int retVal = theLoop.Run();
	
	theLoop.RemoveMessageFilter(&mainWnd);
	_Module.RemoveMessageLoop();
	_Module.Term();

	return retVal;
}


