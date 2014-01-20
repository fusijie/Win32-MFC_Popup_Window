// MsgWindow.cpp : 实现文件
//

#include "stdafx.h"
#include "TestMsgWindow.h"
#include "MsgWindow.h"


// CMsgWindow

IMPLEMENT_DYNAMIC(CMsgWindow, CWnd)

CMsgWindow::CMsgWindow()
{	
	m_hArrowCursor=LoadCursor (NULL,IDC_ARROW);
	m_hHandCursor=LoadCursor (NULL,IDC_HAND);
	m_hCurCursor=m_hArrowCursor;
	m_hAppSmallIcon=NULL;

	m_hSkinDC=m_hCacheDC=NULL;
	m_hSkinBitmap=m_hSkinOldBitmap=m_hCacheBitmap=m_hCacheOldBitmap=NULL;

	m_pControls=NULL;
	m_nControlCount=0;

	m_nHoverIndex=m_nDownIndex=-1;

	m_hFont= (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	LOGFONT lf;
	GetObject (m_hFont,sizeof(LOGFONT), &lf);
	lf.lfWeight=FW_BOLD;
	m_hBoldFont=CreateFontIndirect(&lf);

	m_bClickAutoClose=FALSE;
	m_isFadingOut=FALSE;
	m_bTracking=FALSE;
	m_strURL=_T("");
}

CMsgWindow::~CMsgWindow()
{
	if(m_hSkinDC){
		SelectObject(m_hSkinDC,m_hSkinOldBitmap);
		DeleteObject(m_hSkinBitmap);
		DeleteDC(m_hSkinDC);
	}
}


BEGIN_MESSAGE_MAP(CMsgWindow, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_MESSAGE(WM_CONTROLCLICK,OnControlClick)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
	ON_WM_TIMER()
	ON_WM_MOUSEHOVER()
	ON_WM_NCLBUTTONDOWN()
END_MESSAGE_MAP()

BOOL CMsgWindow::Create( HWND hWndParent,LPCTSTR lpWindowName,int nWidth,int nHeight,bool isAutoClose, bool clickAutoClose)
{
	if(IsWindow())return FALSE;
	if(!m_hAppSmallIcon){
		TCHAR szFileName[MAX_PATH]={0};
		GetModuleFileName(NULL,szFileName,MAX_PATH);
		SHFILEINFO shfi;
		SHGetFileInfo (szFileName, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(SHFILEINFO),SHGFI_ICON|SHGFI_SMALLICON);
		m_hAppSmallIcon=shfi.hIcon;
	}
	DWORD dwStyle	= WS_SYSMENU | WS_POPUP;
	DWORD dwExStyle	= WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
	CString strWndClass=AfxRegisterWndClass(0,AfxGetApp()->LoadStandardCursor(IDC_ARROW),GetSysColorBrush(COLOR_WINDOW),NULL);
	BOOL bRet=CreateEx(dwExStyle,strWndClass,lpWindowName,dwStyle,0,0,nWidth,nHeight,hWndParent,NULL);
	if (!bRet) return FALSE;

	//创建子控件
	m_nControlCount=4;
	m_pControls=(LPNEWSCONTROL)new NEWSCONTROL[m_nControlCount];
	CreateControl(&m_pControls[0],NCT_CLOSE,nWidth-40,1,40,18);
	CreateControl(&m_pControls[1],NCT_VIEW,nWidth-55,nHeight-25,50,20);
	int x=8;
	int y=26;
	int w=nWidth-x*2;
	int h=20;
	CreateControl(&m_pControls[2],NCT_TITLE,x,y,w,h);
	y+=h+5;
	h=nHeight-y-35;
	CreateControl(&m_pControls[3],NCT_CONTENT,x,y,w,h);
	//----------------------------
	//设置窗口圆角
	HRGN hRgn=CreateRoundRectRgn(0,0,nWidth+1, nHeight+1,5,5);
	SetWindowRgn(hRgn,FALSE);
	DeleteObject(hRgn);

	m_Height=nHeight;
	m_Width=nWidth;
	m_bClickAutoClose=clickAutoClose;
	m_isAutoClose=isAutoClose;
	if (m_isAutoClose)
	{
		SetTimer(TIMER_WINDOW_DELAY,DELAY_TIME,NULL);
	}
	return TRUE;
}

BOOL CMsgWindow::SetMsg(LPCTSTR lpNewsTitle,LPCTSTR lpNewsContent,LPCTSTR lpNewsURL/*=""*/)
{
	if(!m_nControlCount)return FALSE;
	m_pControls[2].strText=lpNewsTitle;
	m_pControls[3].strText=lpNewsContent;
	m_strURL=lpNewsURL;
	DrawWindowEx();
	return TRUE;
}

void CMsgWindow::Show()
{
	//在桌面右下角显示
	RECT rc;
	GetClientRect(&rc);
	int nWidth=rc.right-rc.left;
	int nHeight=rc.bottom-rc.top;
	//取出桌面工作区
	SystemParametersInfo(SPI_GETWORKAREA,NULL,&rc,NULL);
	SetWindowPos(NULL,rc.right-nWidth,rc.bottom-nHeight,0,0,SWP_NOZORDER|SWP_NOSIZE|SWP_NOREDRAW);

	ShowWindow(SW_SHOW);
	UpdateWindow();
}

void CMsgWindow::CreateControl(LPNEWSCONTROL pControl,int nType,int x,int y,int nWidth,int nHeight,CString strText/*=""*/)
{
	pControl->nType=nType;
	pControl->strText=strText;
	pControl->x=x;
	pControl->y=y;
	pControl->nWidth=nWidth;
	pControl->nHeight=nHeight;
	pControl->Rect=CreateRect(x,y,x+nWidth,y+nHeight);
}

BOOL CMsgWindow::SetSkin(LPCTSTR lpBitmapName,COLORREF CaptionColor/*=0x000000*/)
{
	HBITMAP hBitmap=LoadBitmap((HINSTANCE)GetModuleHandle(NULL),lpBitmapName);
	if(!hBitmap)return FALSE;
	if(m_hSkinBitmap){
		SelectObject(m_hSkinDC,m_hSkinOldBitmap);
		DeleteObject(m_hSkinBitmap);
		m_hSkinBitmap=m_hSkinOldBitmap=NULL;
	}
	//----------------------------
	if(!m_hSkinDC){
		HDC hSrcDC=::GetDC(0);
		m_hSkinDC=CreateCompatibleDC(hSrcDC);
		::ReleaseDC(0,hSrcDC);
	}
	//----------------------------
	m_hSkinBitmap=hBitmap;
	m_hSkinOldBitmap=(HBITMAP)SelectObject(m_hSkinDC,m_hSkinBitmap);
	m_CaptionColor=CaptionColor;
	//----------------------------
	DrawWindowEx();
	//----------------------------
	return TRUE;
}

BOOL CMsgWindow::DrawWindowEx()
{
	if(!IsWindow())return FALSE;
	if(!DrawWindow())return FALSE;
	if(IsWindowVisible()){
		RECT rc;
		GetClientRect(&rc);
		int nWidth=rc.right-rc.left;
		int nHeight=rc.bottom-rc.top;
		HDC hDC=::GetDC(m_hWnd);
		::BitBlt(hDC,0,0,nWidth,nHeight,m_hCacheDC,0,0,SRCCOPY);
		::ReleaseDC(m_hWnd,hDC);
	}
	return TRUE;
}

BOOL CMsgWindow::DrawWindow()
{
	if(!m_hSkinDC)return FALSE;
	RECT rc;
	GetClientRect(&rc);
	int nWidth=rc.right-rc.left;
	int nHeight=rc.bottom-rc.top;
	if(!m_hCacheDC){
		HDC hSrcDC=::GetDC(0);
		m_hCacheDC=CreateCompatibleDC(hSrcDC);
		m_hCacheBitmap=CreateCompatibleBitmap(hSrcDC,nWidth,nHeight);
		m_hCacheOldBitmap=(HBITMAP)SelectObject(m_hCacheDC,m_hCacheBitmap);
		::ReleaseDC(0,hSrcDC);
		SetBkMode(m_hCacheDC,TRANSPARENT);
	}
	//画出背景 ------------------------------------
	RECT rcSrc=CreateRect(0,0,80,60);
	RECT rcNine=CreateRect(35,25,45,30);
	DrawNineRect(m_hCacheDC,rc,rcSrc,rcNine);
	//画出标题 ------------------------------------
	RECT rcText=CreateRect(8,5,nWidth-50,5+16);
	HICON hIcon=(HICON)SendMessage(WM_GETICON,ICON_SMALL,NULL);
	if(!hIcon)hIcon=m_hAppSmallIcon;
	if(hIcon){
		DrawIconEx(m_hCacheDC,rcText.left,rcText.top,hIcon,16,16,NULL,NULL,DI_NORMAL);
		rcText.left+=25;
	}
	int nLen=GetWindowTextLength();
	if(nLen){
		CString strText;
		GetWindowText(strText.GetBuffer(nLen+1),nLen+1);
		strText.ReleaseBuffer();
		HFONT hOldFont=(HFONT)SelectObject(m_hCacheDC,m_hFont);
		SetTextColor(m_hCacheDC,m_CaptionColor);
		DrawText (m_hCacheDC,strText,-1,&rcText,DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_WORD_ELLIPSIS);
		SelectObject(m_hCacheDC,hOldFont);
	}
	//画出子控件 ----------------------------------
	DrawButton(m_hCacheDC,&m_pControls[0]);
	DrawButton(m_hCacheDC,&m_pControls[1]);
	DrawStatic(m_hCacheDC,&m_pControls[2]);
	DrawStatic(m_hCacheDC,&m_pControls[3]);
	return TRUE;
}

void CMsgWindow::DrawButton(HDC hDC,LPNEWSCONTROL pControl)
{
	int nSrcX=0;
	int nSrcY=0;
	switch (pControl->nType){
	case NCT_CLOSE:
		nSrcX=0;
		nSrcY=60;
		break;
	case NCT_VIEW:
		nSrcX=0;
		nSrcY=80;
		break;
	}
	//-----------------------
	if(m_nDownIndex==pControl->nType){
		nSrcX+=pControl->nWidth*2;
	}else if(m_nHoverIndex==pControl->nType){
		nSrcX+=pControl->nWidth*1;
	}
	//-----------------------
	BitBlt(hDC,pControl->x,pControl->y,pControl->nWidth,pControl->nHeight,m_hSkinDC,nSrcX,nSrcY,SRCCOPY);
}

void CMsgWindow::DrawStatic(HDC hDC,LPNEWSCONTROL pControl)
{
	if(pControl->strText==_T(""))return;
	HFONT hFont=NULL;
	UINT uFormat=0;
	switch (pControl->nType){
	case NCT_TITLE:
		hFont=m_hBoldFont;
		uFormat=DT_SINGLELINE|DT_VCENTER|DT_CENTER|DT_NOPREFIX|DT_WORD_ELLIPSIS;
		break;
	case NCT_CONTENT:
		hFont=m_hFont;
		uFormat=DT_WORDBREAK|DT_EDITCONTROL|DT_NOPREFIX/*|DT_WORD_ELLIPSIS*/;
		break;
	}
	//-----------------------
	if(m_nHoverIndex==pControl->nType){
		SetTextColor(hDC,0xEB7A16);
	}else{
		SetTextColor(hDC,0x7C4E0C);
	}
	//-----------------------
	HFONT hOldFont=(HFONT)SelectObject(m_hCacheDC,hFont);	
	DrawText (hDC,pControl->strText,-1,&pControl->Rect,uFormat);
	SelectObject(hDC,hOldFont);
}

//画九宫图(目标DC，目标矩形，来源矩形，九宫矩形，透明颜色)
void CMsgWindow::DrawNineRect(HDC hdcDest,RECT DestRect,RECT SrcRect,RECT NineRect,UINT crTransparent)
{
	int x=0,y=0,nWidth,nHeight;
	int xSrc=0,ySrc=0,nSrcWidth,nSrcHeight;
	int nDestWidth,nDestHeight;
	nDestWidth = DestRect.right - DestRect.left;
	nDestHeight = DestRect.bottom - DestRect.top;
	// 左上-------------------------------------;
	x = DestRect.left;
	y = DestRect.top;
	nWidth = NineRect.left - SrcRect.left;
	nHeight = NineRect.top - SrcRect.top;
	xSrc = SrcRect.left;
	ySrc = SrcRect.top;
	::TransparentBlt (hdcDest, x, y, nWidth, nHeight, m_hSkinDC, xSrc, ySrc, nWidth, nHeight, crTransparent);
	// 上-------------------------------------;
	x = DestRect.left + NineRect.left - SrcRect.left;
	nWidth = nDestWidth - nWidth - (SrcRect.right - NineRect.right);
	xSrc = NineRect.left;
	nSrcWidth = NineRect.right - NineRect.left;
	nSrcHeight = NineRect.top - SrcRect.top;
	::StretchBlt (hdcDest, x, y, nWidth, nHeight, m_hSkinDC, xSrc, ySrc, nSrcWidth, nSrcHeight,SRCCOPY);
	// 右上-------------------------------------;
	x = DestRect.right - (SrcRect.right - NineRect.right);
	nWidth = SrcRect.right - NineRect.right;
	xSrc = NineRect.right;
	::TransparentBlt (hdcDest, x, y, nWidth, nHeight, m_hSkinDC, xSrc, ySrc, nWidth, nHeight, crTransparent);
	// 左-------------------------------------;
	x = DestRect.left;
	y = DestRect.top + NineRect.top - SrcRect.top;
	nWidth = NineRect.left - SrcRect.left;
	nHeight = DestRect.bottom - y - (SrcRect.bottom - NineRect.bottom);
	xSrc = SrcRect.left;
	ySrc = NineRect.top;
	nSrcWidth = NineRect.left - SrcRect.left;
	nSrcHeight = NineRect.bottom - NineRect.top;
	::StretchBlt (hdcDest, x, y, nWidth, nHeight, m_hSkinDC, xSrc, ySrc, nSrcWidth, nSrcHeight,SRCCOPY);
	// 中-------------------------------------;
	x = DestRect.left + NineRect.left - SrcRect.left;
	nWidth = nDestWidth - nWidth - (SrcRect.right - NineRect.right);
	xSrc = NineRect.left;
	nSrcWidth = NineRect.right - NineRect.left;
	::StretchBlt (hdcDest, x, y, nWidth, nHeight, m_hSkinDC, xSrc, ySrc, nSrcWidth, nSrcHeight, SRCCOPY);
	// 右-------------------------------------;
	x = DestRect.right - (SrcRect.right - NineRect.right);
	nWidth = SrcRect.right - NineRect.right;
	xSrc = NineRect.right;
	nSrcWidth = SrcRect.right - NineRect.right;
	::StretchBlt (hdcDest, x, y, nWidth, nHeight, m_hSkinDC, xSrc, ySrc, nSrcWidth, nSrcHeight,SRCCOPY);
	// 左下-------------------------------------;
	x = DestRect.left;
	y = DestRect.bottom - (SrcRect.bottom - NineRect.bottom);
	nWidth = NineRect.left - SrcRect.left;
	nHeight = SrcRect.bottom - NineRect.bottom;
	xSrc = SrcRect.left;
	ySrc = NineRect.bottom;
	::TransparentBlt (hdcDest, x, y, nWidth, nHeight, m_hSkinDC, xSrc, ySrc, nWidth, nHeight, crTransparent);
	// 下-------------------------------------;
	x = DestRect.left + NineRect.left - SrcRect.left;
	nWidth = nDestWidth - nWidth - (SrcRect.right - NineRect.right);
	xSrc = NineRect.left;
	nSrcWidth = NineRect.right - NineRect.left;
	nSrcHeight = SrcRect.bottom - NineRect.bottom;
	::StretchBlt (hdcDest, x, y, nWidth, nHeight, m_hSkinDC, xSrc, ySrc, nSrcWidth, nSrcHeight,SRCCOPY);
	// 右下-------------------------------------;
	x = DestRect.right - (SrcRect.right - NineRect.right);
	nWidth = SrcRect.right - NineRect.right;
	xSrc = NineRect.right;
	::TransparentBlt (hdcDest, x, y, nWidth, nHeight, m_hSkinDC, xSrc, ySrc, nWidth, nHeight, crTransparent);
}

RECT CMsgWindow::CreateRect(LONG left,LONG top,LONG right,LONG bottom)
{
	RECT rc;
	rc.left=left;
	rc.top=top;
	rc.right=right;
	rc.bottom=bottom;
	return rc;
}

BOOL CMsgWindow::IsWindow()
{
	if(!m_hWnd){
		return FALSE;
	}else if(!::IsWindow(m_hWnd)){
		m_hWnd=NULL;
		return FALSE;
	}else{
		return TRUE;
	}
}

// CMsgWindow 消息处理程序

BOOL CMsgWindow::OnEraseBkgnd(CDC* pDC)
{
	if(!m_hCacheDC){
		if(!DrawWindow())
			return CWnd::OnEraseBkgnd(pDC);
	}
	RECT rc;
	GetClientRect(&rc);
	int nWidth=rc.right-rc.left;
	int nHeight=rc.bottom-rc.top;
	::BitBlt(pDC->m_hDC,0,0,nWidth,nHeight,m_hCacheDC,0,0,SRCCOPY);
	return TRUE;
}


void CMsgWindow::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	if(!m_hCacheDC){
		if(!DrawWindow())
			return ;
	}
	PAINTSTRUCT ps;
	BeginPaint(&ps);
	int nWidth=ps.rcPaint.right-ps.rcPaint.left;
	int nHeight=ps.rcPaint.bottom-ps.rcPaint.top;
	::BitBlt(ps.hdc,ps.rcPaint.left,ps.rcPaint.top,nWidth,nHeight,m_hCacheDC,ps.rcPaint.left,ps.rcPaint.top,SRCCOPY);
	EndPaint(&ps);
}

void CMsgWindow::SetAutoClose(BOOL bAutoClose)
{
	m_bClickAutoClose=bAutoClose;
}

void CMsgWindow::OnLButtonUp(UINT nFlags, CPoint point)
{
	int nControlIndex=ControlFromPoint(point);
	if(m_nDownIndex!=-1){
		if(m_nDownIndex==nControlIndex){
			PostMessage(WM_CONTROLCLICK,(WPARAM)nControlIndex,0);
		}
		m_nDownIndex=-1;
		DrawWindowEx();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

LRESULT CMsgWindow::OnControlClick( WPARAM wParam, LPARAM lParam )
{
	int nControlIndex=(int)wParam;
	if(nControlIndex<0 || nControlIndex>=m_nControlCount)return 0;
	//----------------------------
	if(nControlIndex==NCT_CLOSE){
		PostMessage(WM_CLOSE,NULL,NULL);
	}else{
		if(m_strURL!=_T("")){
			::ShellExecute(NULL,_T("Open"),m_strURL,NULL,NULL,SW_SHOW);
		}
		if(m_bClickAutoClose){
			PostMessage(WM_CLOSE,NULL,NULL);
		}
	}
	return 0;
}

int CMsgWindow::ControlFromPoint(POINT pt)
{
	for(int i=0;i<m_nControlCount;i++){
		if(PtInRect(&m_pControls[i].Rect,pt)){
			return i;
		}
	}
	return -1;
}

void CMsgWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
	//判断鼠标是否点击了DirectUI子控件
	int nControlIndex=ControlFromPoint(point);
	if(m_nDownIndex!=nControlIndex){
		m_nDownIndex=nControlIndex;
		DrawWindowEx();		
	}
	//-------------------------------------------------------
	if(nControlIndex==-1){ //未点击子控件
		ReleaseCapture (); //释放鼠标
		SendMessage (WM_NCLBUTTONDOWN, HTCAPTION, 0);//发送标题栏按下消息
	}
	CWnd::OnLButtonDown(nFlags, point);
}


void CMsgWindow::OnMouseHover(UINT nFlags, CPoint point)
{
	if (m_isAutoClose && !m_isFadingOut)
	{
		KillTimer(TIMER_WINDOW_DELAY);
	}
	CWnd::OnMouseHover(nFlags, point);
}

void CMsgWindow::OnMouseLeave()
{
	if(m_nHoverIndex!=-1 || m_nDownIndex!=-1){
		m_nDownIndex=m_nHoverIndex=-1;
		DrawWindowEx();
	}
	m_bTracking=FALSE;

	if (m_isAutoClose && !m_isFadingOut)
	{
		SetTimer(TIMER_WINDOW_DELAY,DELAY_TIME,NULL);
	}
	CWnd::OnMouseLeave();
}


void CMsgWindow::OnMouseMove(UINT nFlags, CPoint point)
{
	if   (!m_bTracking) 
	{ 
		TRACKMOUSEEVENT   tme; 
		tme.cbSize   =   sizeof(tme); 
		tme.hwndTrack   =   m_hWnd; 
		tme.dwFlags   =   TME_LEAVE | TME_HOVER;
		tme.dwHoverTime   =   1; 
		m_bTracking   =   TrackMouseEvent(&tme); 
	} 
	//-------------------------------------------------------
	int nControlIndex=ControlFromPoint(point);
	if(m_nHoverIndex!=nControlIndex){
		m_nHoverIndex=nControlIndex;
		DrawWindowEx();
		if(nControlIndex==-1 || nControlIndex==NCT_CLOSE){
			SetCursor(m_hArrowCursor);
		}else{
			SetCursor(m_hHandCursor);
		}
	}
	CWnd::OnMouseMove(nFlags, point);
}

void CMsgWindow::SetCursor(HCURSOR hCursor)
{
	if(m_hCurCursor!=hCursor){
		m_hCurCursor=hCursor;
		::SetCursor(m_hCurCursor);
	}
}

void CMsgWindow::OnDestroy()
{
	CWnd::OnDestroy();
	//----------------------------
	if(m_pControls)delete []m_pControls;
	m_pControls=NULL;
	m_nControlCount=0;
	m_nHoverIndex=m_nDownIndex=-1;
	//----------------------------
	if(m_hCacheDC){
		SelectObject(m_hCacheDC,m_hCacheOldBitmap);
		DeleteObject(m_hCacheBitmap);
		DeleteDC(m_hCacheDC);
		m_hCacheOldBitmap=m_hCacheBitmap=NULL;
		m_hCacheDC=NULL;
	}
	//----------------------------
	m_strURL=_T("");
	m_bTracking=FALSE;
}


BOOL CMsgWindow::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	::SetCursor(m_hCurCursor);
	return TRUE;
}


void CMsgWindow::PostNcDestroy()
{
	delete this;
	CWnd::PostNcDestroy();
}

void CMsgWindow::OnTimer(UINT_PTR nIDEvent)
{	
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0);
	int y=rect.bottom;
	int x=rect.right-m_Width;
	switch(nIDEvent)
	{
	case TIMER_WINDOW_DELAY:
		KillTimer(TIMER_WINDOW_DELAY);
		SetTimer(TIMER_WINDOW_FADEOUT,20,NULL);
		m_isFadingOut=true;
		break;
	case TIMER_WINDOW_FADEOUT:
		if(m_Height>=0)
		{
			m_Height-=4;
			MoveWindow(x,y-m_Height,m_Width,m_Height);
		}
		else
		{
			KillTimer(TIMER_WINDOW_FADEOUT);
			PostMessage(WM_CLOSE,NULL,NULL);
		}
		break;
	default:
		break;
	}
	CWnd::OnTimer(nIDEvent);
}

//屏蔽标题栏拖动
void CMsgWindow::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	if (nHitTest==HTCAPTION) return;
	CWnd::OnNcLButtonDown(nHitTest, point);
}
