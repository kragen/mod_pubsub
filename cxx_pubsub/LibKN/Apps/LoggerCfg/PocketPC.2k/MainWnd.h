#if !defined(MAINWND_H)
#define MAINWND_H

#include <AtlCtrls.h>
#include "Layout.h"

class TMainWindow : 
	public CWindowImpl<TMainWindow>, 
	public TLayoutWindowImp,
	public CMessageFilter
{
public:
	enum
	{
		ExitId,
		EnabledId,
		MaskId,
		FileId,
	};

	TMainWindow();
	~TMainWindow();

	BOOL OnCreate(LPCREATESTRUCT cs);
	void OnDestroy();
	void OnSize(UINT nType, CSize size);
	void OnExit(UINT code, int id, HWND ctl);
	void OnEnabled(UINT code, int id, HWND ctl);
	void OnLButtonUp(UINT u, CPoint p);
	void OnPaint(HDC dc);
	void OnFileKillFocus(UINT code, int id, HWND ctl);
	void OnMaskKillFocus(UINT code, int id, HWND ctl);
	LRESULT OnRefreshMsg(UINT u, WPARAM wp, LPARAM lp);

DECLARE_WND_CLASS(_T("MainWindow"))

BEGIN_MSG_MAP_EX(TMainWindow)
	MSG_WM_CREATE(OnCreate)
	MSG_WM_DESTROY(OnDestroy)
	MSG_WM_SIZE(OnSize)
	MSG_WM_LBUTTONUP(OnLButtonUp)
	MSG_WM_PAINT(OnPaint)
	MESSAGE_HANDLER_EX(m_RefreshMsg, OnRefreshMsg)
	COMMAND_HANDLER_EX(ExitId, BN_CLICKED, OnExit)
	COMMAND_HANDLER_EX(EnabledId, BN_CLICKED, OnEnabled)
	COMMAND_HANDLER_EX(FileId, EN_KILLFOCUS, OnFileKillFocus)
	COMMAND_HANDLER_EX(MaskId, EN_KILLFOCUS, OnMaskKillFocus)
END_MSG_MAP()

	BOOL PreTranslateMessage(MSG* pMsg);

private:
	void SendRefreshMsg();
	void UpdateFromRegistry();

	UINT m_RefreshMsg;
	CButton m_ExitButton;

	CStatic m_stEnabled;
	CButton m_chkEnabled;

	CStatic m_stFile;
	CEdit m_File;

	CStatic m_stMask;
	CEdit m_Mask;

	HKEY m_Key;
};

#endif
