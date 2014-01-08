// MiniNews.cpp : 定义应用程序的入口点。
// Download by http://www.codefans.net

#include "stdafx.h"
#include "MiniNews.h"
#include "NewsWindow.h"

CNewsWindow NewsWindow;
BOOL InitNews();

int APIENTRY _tWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{

	if(!InitNews())
		return 0;


	// 主消息循环:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}
BOOL InitNews()
{
	LPCTSTR lpNewsTitle		= _T("测试新闻标题");
	LPCTSTR lpNewsContent	= _T("测试新闻内容,彗星科技发展有限公司(彗星网络)有着多年网站制作、软件开发经验，并有经验丰富的开发团队。为您提供专业的网站建设、软件开发等服务。");
	LPCTSTR lpNewsURL		= _T("");

	NewsWindow.SetSkin(MAKEINTRESOURCE(IDB_SKIN_QQ));
	//NewsWindow.SetSkin(MAKEINTRESOURCE(IDB_SKIN_WANGWANG));
	//NewsWindow.SetSkin(MAKEINTRESOURCE(IDB_SKIN_XUNLEI),0xFFFFFF);
	if(!NewsWindow.Create(_T("彗星网络新闻")))
		return FALSE;

	NewsWindow.SetNews(lpNewsTitle,lpNewsContent,lpNewsURL);
	NewsWindow.Show();
	NewsWindow.SetMainWindow();
	return TRUE;
}