//=====================================
// 迷你新闻窗口
// 编写:彗星网络.邓学彬
//=====================================
#pragma once

//没有MFC的情况下,使用ATL里的CString
#ifndef __AFX_H__
#include <atlstr.h>
#endif

#define WM_CONTROLCLICK WM_APP+106

class CNewsWindow
{
public:
	CNewsWindow(void);
	~CNewsWindow(void);
public:
	HWND m_hWnd;
public:
	// 设置皮肤(位图资源名称,标题栏文本颜色)
	BOOL SetSkin(LPCTSTR lpSkinBitmapName,COLORREF CaptionColor=0x000000);
	// 创建窗口
	BOOL Create(LPCTSTR lpWindowName,int nWidth=250,int nHeight=180);
	BOOL Create(LPCTSTR lpClassName,LPCTSTR lpWindowName,int nWidth,int nHeight);
	// 设置新闻标题、内容、链接
	BOOL SetNews(LPCTSTR lpNewsTitle,LPCTSTR lpNewsContent,LPCTSTR lpNewsURL);
	// 显示窗口
	void Show();
	// 设置为主窗口,主窗口销毁后会退出消息循环
	void SetMainWindow(BOOL bMainWindow=TRUE);
	// 设置自动关闭,如果为真,点击链接后窗口自动关闭
	void SetAutoClose(BOOL bAutoClose=TRUE);
public:
	BOOL IsWindow();
	BOOL DestroyWindow();
private:
	#define NCT_CLOSE	0
	#define NCT_VIEW	1
	#define NCT_TITLE	2
	#define NCT_CONTENT	3

	typedef struct tagNEWSCONTROL
	{
		CString		strText;
		int			nType;
		int			x;
		int			y;
		int			nWidth;
		int			nHeight;
		RECT		Rect;
	}NEWSCONTROL, *LPNEWSCONTROL;
private:	
	LPNEWSCONTROL m_pControls;
	int m_nControlCount;
private:
	HCURSOR m_hArrowCursor;
	HCURSOR m_hHandCursor;
	HCURSOR m_hCurCursor;

	HICON m_hAppSmallIcon;

	HDC m_hSkinDC;
	HDC m_hCacheDC;
	HBITMAP m_hSkinBitmap;
	HBITMAP m_hSkinOldBitmap;
	HBITMAP m_hCacheBitmap;	
	HBITMAP m_hCacheOldBitmap;
	HFONT m_hFont;
	HFONT m_hBoldFont;
	COLORREF m_CaptionColor;
	int m_nHoverIndex;
	int m_nDownIndex;
	BOOL m_bMainWindow;
	BOOL m_bAutoClose;
	BOOL m_bTracking;
	CString m_strURL;
private:
	BOOL DrawWindow();
	BOOL DrawWindowEx();
	void DrawButton(HDC hDC,LPNEWSCONTROL pControl);
	void DrawStatic(HDC hDC,LPNEWSCONTROL pControl);
	void DrawNineRect(HDC hdcDest,RECT DestRect,RECT SrcRect,RECT NineRect,UINT crTransparent=0xFF00FF);
	RECT CreateRect(LONG left,LONG top,LONG right,LONG bottom);
	void CreateControl(LPNEWSCONTROL pControl,int nType,int x,int y,int nWidth,int nHeight,CString strText=_T(""));
	int ControlFromPoint(POINT pt);
	int ControlFromPoint(LPARAM lParam);
	void SetCursor(HCURSOR hCursor);
public:
	LRESULT OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnEraseBkgnd(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseHover(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseLeave(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnControlClick(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnDestroy(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnSetCursor(UINT message, WPARAM wParam, LPARAM lParam);
};

