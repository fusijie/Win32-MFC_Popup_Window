/********************************************************************
	created:	2014/01/08   11:28
	filename: 	MsgWindow.h
	file ext:	h
	author:		JackyFu
	WebSite:    http://www.fusijie.com
	blog:		http://blog.csdn.net/jackystudio
	QQ:			754505629
	E-mail:		Geek.Jacky@Gmail.com
	
	purpose:	仿QQ新闻右下角弹窗，此为MFC版，移植自Win32版。原作者http://blog.csdn.net/cometnet/article/details/7854345
*********************************************************************/
#pragma once

// CMsgWindow

#define WM_CONTROLCLICK		WM_APP+106
#define TIMER_WINDOW_DELAY		10085
#define TIMER_WINDOW_FADEOUT	10086
#define DELAY_TIME				2000

class CMsgWindow : public CWnd
{
	DECLARE_DYNAMIC(CMsgWindow)

public:
	CMsgWindow();
	virtual ~CMsgWindow();

	// 创建窗口(父窗口句柄，窗口标题，长宽，2s后自动关闭，点击内容自动关闭)
	BOOL Create(HWND hWndParent,LPCTSTR lpWindowName,int nWidth=250,int nHeight=180, bool isAutoClose=true, bool clickAutoClose=false);

	// 显示窗口
	void Show();

	// 设置皮肤(位图资源名称,标题栏文本颜色)
	BOOL SetSkin(LPCTSTR lpBitmapName,COLORREF CaptionColor=0x000000);

	// 设置新闻标题、内容、链接
	BOOL SetMsg(LPCTSTR lpNewsTitle,LPCTSTR lpNewsContent,LPCTSTR lpNewsURL="");

	// 设置自动关闭,如果为真,点击链接后窗口自动关闭
	void SetAutoClose(BOOL bAutoClose);

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
	LPNEWSCONTROL m_pControls;
	int m_nControlCount;

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
	BOOL m_bClickAutoClose;
	BOOL m_bTracking;
	CString m_strURL;

	//创建自动消失
	bool m_isAutoClose;
	bool m_isFadingOut;
	int m_Height;
	int m_Width;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnControlClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual void PostNcDestroy();

private:
	void CreateControl(LPNEWSCONTROL pControl,int nType,int x,int y,int nWidth,int nHeight,CString strText="");
	BOOL DrawWindowEx();
	BOOL DrawWindow();
	void DrawButton(HDC hDC,LPNEWSCONTROL pControl);
	void DrawStatic(HDC hDC,LPNEWSCONTROL pControl);
	void DrawNineRect(HDC hdcDest,RECT DestRect,RECT SrcRect,RECT NineRect,UINT crTransparent=0xFF00FF);
	RECT CreateRect(LONG left,LONG top,LONG right,LONG bottom);
	BOOL IsWindow();
	int ControlFromPoint(LPARAM lParam);
	int ControlFromPoint(POINT pt);
	void SetCursor(HCURSOR hCursor);
public:
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
};


