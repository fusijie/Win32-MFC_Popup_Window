#include "StdAfx.h"
#include "NewsWindow.h"
#include <wingdi.h>
#pragma comment(lib,"Msimg32.lib")
#include <shellapi.h >
#pragma comment(lib,"shell32.lib")



LRESULT CALLBACK NewsWindow_WndProc(HWND, UINT, WPARAM, LPARAM);

CNewsWindow::CNewsWindow(void)
{
	m_hArrowCursor=LoadCursor (NULL,IDC_ARROW);
	m_hHandCursor=LoadCursor (NULL,IDC_HAND);
	m_hCurCursor=m_hArrowCursor;
	m_hAppSmallIcon=NULL;
	
	m_hWnd=NULL;
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

	m_bMainWindow=FALSE;
	m_bAutoClose=FALSE;
	m_bTracking=FALSE;
	m_strURL=_T("");
}

CNewsWindow::~CNewsWindow(void)
{
	if(m_hSkinDC){
		SelectObject(m_hSkinDC,m_hSkinOldBitmap);
		DeleteObject(m_hSkinBitmap);
		DeleteDC(m_hSkinDC);
	}
}
BOOL CNewsWindow::Create(LPCTSTR lpWindowName,int nWidth,int nHeight)
{
	return this->Create(_T("Comet_MiniNews"),lpWindowName,nWidth,nHeight);
}
BOOL CNewsWindow::Create(LPCTSTR lpClassName,LPCTSTR lpWindowName,int nWidth,int nHeight)
{
	if(IsWindow())return FALSE;
	if(!m_hAppSmallIcon){
		TCHAR szFileName[MAX_PATH]={0};
		GetModuleFileName(NULL,szFileName,MAX_PATH);
		SHFILEINFO shfi;
		SHGetFileInfo (szFileName, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(SHFILEINFO),SHGFI_ICON|SHGFI_SMALLICON);
		m_hAppSmallIcon=shfi.hIcon;
	}

	HINSTANCE hInstance=(HINSTANCE)GetModuleHandle(NULL);
	WNDCLASSEX wcex;
	ZeroMemory(&wcex,sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	if(!GetClassInfoEx(0,lpClassName,&wcex)){
		//注册窗口类
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= NewsWindow_WndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= lpClassName;
		wcex.hIconSm		= m_hAppSmallIcon;

		if(!RegisterClassEx(&wcex))
			return FALSE;
	}
	//----------------------------
	//创建窗口
	DWORD dwStyle	= WS_SYSMENU | WS_POPUP;
	DWORD dwExStyle	= WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
	HWND hWnd = CreateWindowEx(dwExStyle,lpClassName,lpWindowName, dwStyle,0, 0, nWidth, nHeight, NULL, NULL, hInstance, NULL);
	//----------------------------
	if (!hWnd)
	{
		return FALSE;
	}
	//----------------------------
	m_hWnd=hWnd;
	SetProp(m_hWnd,_T("CometClassPtr"),this);
	//----------------------------
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
	SetWindowRgn(m_hWnd,hRgn,FALSE);
	DeleteObject(hRgn);
	//----------------------------
	return TRUE;
}
BOOL CNewsWindow::SetNews(LPCTSTR lpNewsTitle,LPCTSTR lpNewsContent,LPCTSTR lpNewsURL)
{
	if(!m_nControlCount)return FALSE;
	m_pControls[2].strText=lpNewsTitle;
	m_pControls[3].strText=lpNewsContent;
	m_strURL=lpNewsURL;
	DrawWindowEx();
	return TRUE;
}
void CNewsWindow::Show()
{
	//在桌面右下角显示
	RECT rc;
	GetClientRect(m_hWnd,&rc);
	int nWidth=rc.right-rc.left;
	int nHeight=rc.bottom-rc.top;
	//取出桌面工作区
	SystemParametersInfo(SPI_GETWORKAREA,NULL,&rc,NULL);
	SetWindowPos(m_hWnd,NULL,rc.right-nWidth,rc.bottom-nHeight,0,0,SWP_NOZORDER|SWP_NOSIZE|SWP_NOREDRAW);

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);
}
void CNewsWindow::SetMainWindow(BOOL bMainWindow)
{
	m_bMainWindow=bMainWindow;
}
void CNewsWindow::SetAutoClose(BOOL bAutoClose)
{
	m_bAutoClose=bAutoClose;
}
void CNewsWindow::CreateControl(LPNEWSCONTROL pControl,int nType,int x,int y,int nWidth,int nHeight,CString strText)
{
	pControl->nType=nType;
	pControl->strText=strText;
	pControl->x=x;
	pControl->y=y;
	pControl->nWidth=nWidth;
	pControl->nHeight=nHeight;
	pControl->Rect=CreateRect(x,y,x+nWidth,y+nHeight);
}
BOOL CNewsWindow::SetSkin(LPCTSTR lpBitmapName,COLORREF CaptionColor)
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
		HDC hSrcDC=GetDC(0);
		m_hSkinDC=CreateCompatibleDC(hSrcDC);
		ReleaseDC(0,hSrcDC);
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
BOOL CNewsWindow::DrawWindowEx()
{
	if(!IsWindow())return FALSE;
	if(!DrawWindow())return FALSE;
	if(IsWindowVisible(m_hWnd)){
		RECT rc;
		GetClientRect(m_hWnd,&rc);
		int nWidth=rc.right-rc.left;
		int nHeight=rc.bottom-rc.top;
		HDC hDC=GetDC(m_hWnd);
		BitBlt(hDC,0,0,nWidth,nHeight,m_hCacheDC,0,0,SRCCOPY);
		ReleaseDC(m_hWnd,hDC);
	}
	return TRUE;
}
BOOL CNewsWindow::DrawWindow()
{
	if(!m_hSkinDC)return FALSE;
	RECT rc;
	GetClientRect(m_hWnd,&rc);
	int nWidth=rc.right-rc.left;
	int nHeight=rc.bottom-rc.top;
	if(!m_hCacheDC){
		HDC hSrcDC=GetDC(0);
		m_hCacheDC=CreateCompatibleDC(hSrcDC);
		m_hCacheBitmap=CreateCompatibleBitmap(hSrcDC,nWidth,nHeight);
		m_hCacheOldBitmap=(HBITMAP)SelectObject(m_hCacheDC,m_hCacheBitmap);
		ReleaseDC(0,hSrcDC);
		SetBkMode(m_hCacheDC,TRANSPARENT);
	}
	//画出背景 ------------------------------------
	RECT rcSrc=CreateRect(0,0,80,60);
	RECT rcNine=CreateRect(35,25,45,30);
	DrawNineRect(m_hCacheDC,rc,rcSrc,rcNine);
	//画出标题 ------------------------------------
	RECT rcText=CreateRect(8,5,nWidth-50,5+16);
	HICON hIcon=(HICON)SendMessage(m_hWnd,WM_GETICON,ICON_SMALL,NULL);
	if(!hIcon)hIcon=m_hAppSmallIcon;
	if(hIcon){
		DrawIconEx(m_hCacheDC,rcText.left,rcText.top,hIcon,16,16,NULL,NULL,DI_NORMAL);
		rcText.left+=25;
	}
	int nLen=GetWindowTextLength(m_hWnd);
	if(nLen){
		CString strText;
		GetWindowText(m_hWnd,strText.GetBuffer(nLen+1),nLen+1);
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
void CNewsWindow::DrawButton(HDC hDC,LPNEWSCONTROL pControl)
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
void CNewsWindow::DrawStatic(HDC hDC,LPNEWSCONTROL pControl)
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
			uFormat=DT_WORDBREAK|DT_NOPREFIX|DT_WORD_ELLIPSIS;
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
LRESULT CNewsWindow::OnEraseBkgnd(UINT message, WPARAM wParam, LPARAM lParam)
{
	if(!m_hCacheDC){
		if(!DrawWindow())
			return DefWindowProc( message, wParam, lParam);
	}
	RECT rc;
	GetClientRect(m_hWnd,&rc);
	int nWidth=rc.right-rc.left;
	int nHeight=rc.bottom-rc.top;
	BitBlt((HDC)wParam,0,0,nWidth,nHeight,m_hCacheDC,0,0,SRCCOPY);
	return 1;
}
LRESULT CNewsWindow::OnPaint(UINT message, WPARAM wParam, LPARAM lParam)
{
	if(!m_hCacheDC){
		if(!DrawWindow())
			return DefWindowProc( message, wParam, lParam);
	}
	PAINTSTRUCT ps;
	BeginPaint(m_hWnd,&ps);
	int nWidth=ps.rcPaint.right-ps.rcPaint.left;
	int nHeight=ps.rcPaint.bottom-ps.rcPaint.top;
	BitBlt(ps.hdc,ps.rcPaint.left,ps.rcPaint.top,nWidth,nHeight,m_hCacheDC,ps.rcPaint.left,ps.rcPaint.top,SRCCOPY);
	EndPaint(m_hWnd,&ps);
	return 0;
}
//画九宫图(目标DC，目标矩形，来源矩形，九宫矩形，透明颜色)
void CNewsWindow::DrawNineRect(HDC hdcDest,RECT DestRect,RECT SrcRect,RECT NineRect,UINT crTransparent)
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
RECT CNewsWindow::CreateRect(LONG left,LONG top,LONG right,LONG bottom)
{
	RECT rc;
	rc.left=left;
	rc.top=top;
	rc.right=right;
	rc.bottom=bottom;
	return rc;
}
BOOL CNewsWindow::IsWindow()
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
BOOL CNewsWindow::DestroyWindow()
{
	if(IsWindow()){
		return ::DestroyWindow(m_hWnd);
	}
	return FALSE;
}
void CNewsWindow::SetCursor(HCURSOR hCursor)
{
	if(m_hCurCursor!=hCursor){
		m_hCurCursor=hCursor;
		::SetCursor(m_hCurCursor);
	}
}
int CNewsWindow::ControlFromPoint(LPARAM lParam)
{
	POINT pt;
	pt.x=LOWORD(lParam);
	pt.y=HIWORD(lParam);
	return ControlFromPoint(pt);

}
int CNewsWindow::ControlFromPoint(POINT pt)
{
	for(int i=0;i<m_nControlCount;i++){
		if(PtInRect(&m_pControls[i].Rect,pt)){
			return i;
		}
	}
	return -1;
}
LRESULT  CNewsWindow::OnMouseMove(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet=DefWindowProc( message, wParam, lParam);
	//-------------------------------------------------------
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
	int nControlIndex=ControlFromPoint(lParam);
	if(m_nHoverIndex!=nControlIndex){
		m_nHoverIndex=nControlIndex;
		DrawWindowEx();
		if(nControlIndex==-1 || nControlIndex==NCT_CLOSE){
			SetCursor(m_hArrowCursor);
		}else{
			SetCursor(m_hHandCursor);
		}
	}
	return lRet;
}
LRESULT  CNewsWindow::OnLButtonDown(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet=DefWindowProc( message, wParam, lParam);
	//-------------------------------------------------------
	//判断鼠标是否点击了DirectUI子控件
	int nControlIndex=ControlFromPoint(lParam);
	if(m_nDownIndex!=nControlIndex){
		m_nDownIndex=nControlIndex;
		DrawWindowEx();		
	}
	//-------------------------------------------------------
	if(nControlIndex==-1){ //未点击子控件
		ReleaseCapture (); //释放鼠标
		::SendMessage (m_hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);//发送标题栏按下消息
	}
	return lRet;
}
LRESULT  CNewsWindow::OnLButtonUp(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet=DefWindowProc( message, wParam, lParam);
	int nControlIndex=ControlFromPoint(lParam);
	if(m_nDownIndex!=-1){
		if(m_nDownIndex==nControlIndex){
			PostMessage(m_hWnd,WM_CONTROLCLICK,(WPARAM)nControlIndex,lParam);
		}
		m_nDownIndex=-1;
		DrawWindowEx();
	}
	return lRet;	
}
LRESULT CNewsWindow::OnMouseHover(UINT message, WPARAM wParam, LPARAM lParam)
{

	return DefWindowProc( message, wParam, lParam);
}
LRESULT CNewsWindow::OnMouseLeave(UINT message, WPARAM wParam, LPARAM lParam)
{
	if(m_nHoverIndex!=-1 || m_nDownIndex!=-1){
		m_nDownIndex=m_nHoverIndex=-1;
		DrawWindowEx();
	}
	m_bTracking=FALSE;
	return DefWindowProc( message, wParam, lParam);
}
LRESULT  CNewsWindow::OnDestroy(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet=DefWindowProc( message, wParam, lParam);
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
	//----------------------------
	if(m_bMainWindow)
		PostQuitMessage(0);
	//----------------------------
	return lRet;	
}
LRESULT  CNewsWindow::OnSetCursor(UINT message, WPARAM wParam, LPARAM lParam)
{
	::SetCursor(m_hCurCursor);
	return 1;	
}
LRESULT CNewsWindow::OnControlClick(UINT message, WPARAM wParam, LPARAM lParam)
{
	int nControlIndex=(int)wParam;
	if(nControlIndex<0 || nControlIndex>=m_nControlCount)return 0;
	//----------------------------
	if(nControlIndex==NCT_CLOSE){
		PostMessage(m_hWnd,WM_CLOSE,NULL,NULL);
	}else{
		if(m_strURL!=_T("")){
			::ShellExecute(NULL,_T("Open"),m_strURL,NULL,NULL,SW_SHOW);
		}
		if(m_bAutoClose){
			PostMessage(m_hWnd,WM_CLOSE,NULL,NULL);
		}
	}
	//----------------------------
	return 0;
}
LRESULT CNewsWindow::OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ERASEBKGND:
		return OnEraseBkgnd( message, wParam, lParam);
		break;
	case WM_PAINT:
		return OnPaint( message, wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		return OnMouseMove( message, wParam, lParam);
		break;
	case WM_MOUSEHOVER:
		return OnMouseHover( message, wParam, lParam);
		break;
	case WM_MOUSELEAVE:
		return OnMouseLeave( message, wParam, lParam);
		break;
	case WM_LBUTTONDOWN:
		return OnLButtonDown( message, wParam, lParam);
		break;
	case WM_LBUTTONUP:
		return OnLButtonUp( message, wParam, lParam);
		break;
	case WM_CONTROLCLICK:
		return OnControlClick( message, wParam, lParam);
		break;
	case WM_DESTROY:
		return OnDestroy( message, wParam, lParam);
		break;
	case WM_SETCURSOR:
		return OnSetCursor( message, wParam, lParam);
		break;
	}
	return DefWindowProc( message, wParam, lParam);
}
LRESULT CNewsWindow::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return ::DefWindowProc(m_hWnd, message, wParam, lParam);
}
LRESULT CALLBACK NewsWindow_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CNewsWindow* pWnd=(CNewsWindow*)GetProp(hWnd,_T("CometClassPtr"));
	if(pWnd){
		return pWnd->OnMessage(message,wParam,lParam);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}