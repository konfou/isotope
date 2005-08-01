/*
	Copyright (C) 1999, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GDDraw.h"
#include <windowsx.h>
#include "GDSound.h"
#include "GArray.h"

long PASCAL GGame_WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_ACTIVATEAPP:
//			bActive = wParam;
			break;
		case WM_SETCURSOR:
			SetCursor(NULL);
			return(TRUE);
			break;
		case WM_KEYDOWN:
			switch(wParam)
			{
				case VK_ESCAPE:
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					break;
            default:
				break;
			}
			
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
		break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND GGame_MakeWindow(HINSTANCE hInst, int nCmdShow, int nWidth, int nHeight, bool bFullScreen)
{
	HWND hWnd;

	WNDCLASS WndClass;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = GGame_WinProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInst;
	WndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH__ *)GetStockObject(BLACK_BRUSH);
	WndClass.lpszMenuName = 0;
	WndClass.lpszClassName = "Game Starter";
	RegisterClass(&WndClass);
	hWnd = CreateWindowEx(WS_EX_TOPMOST, "Game Starter", "Game Starter", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInst, NULL);
	if (!hWnd)
		return (hWnd);
   
	if(!bFullScreen)
	{
		RECT rc;
		DWORD dwStyle;

		dwStyle = GetWindowStyle(hWnd);
		dwStyle &= ~WS_POPUP;
		dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX;
		SetWindowLong(hWnd, GWL_STYLE, dwStyle);

		SetRect(&rc, 0, 0, nWidth, nHeight);

		AdjustWindowRectEx(&rc,
		   GetWindowStyle(hWnd),
		   GetMenu(hWnd) != NULL,
		   GetWindowExStyle(hWnd));

		SetWindowPos(hWnd, NULL, 0, 0, rc.right-rc.left, rc.bottom-rc.top,
		   SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

		SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0,
		   SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return(hWnd);
}

GGame::GGame(HINSTANCE hInst, int nCmdShow, int nWidth, int nHeight, bool bFullScreen)
{
	m_nCmdShow = 0;
	m_hInst = NULL;
	m_hWnd = NULL;
	m_pWindow = NULL;
	m_pLevel = NULL;
	m_pBackground = NULL;
	m_pBGMusic = NULL;
	m_pMainGuy = NULL;
	m_pSprites = NULL;
	m_nCmdShow = nCmdShow;
	m_hInst = hInst;
	m_pImages = new GSmallArray(sizeof(GBlock*), 20);
	m_hWnd = GGame_MakeWindow(hInst, nCmdShow, nWidth, nHeight, bFullScreen);
	m_pWindow = new GWindow(m_hWnd, nWidth, nHeight, bFullScreen);
}

GGame::~GGame()
{
	if(m_pBGMusic)
	{
		m_pBGMusic->Stop();
		delete(m_pBGMusic);
	}
	delete(m_pMainGuy);
	delete(m_pSprites);
	int nSize = m_pImages->GetSize();
	int n;
	for(n = 0; n < nSize; n++)
	{
		delete(*(GBitmap**)m_pImages->_GetCellRef(n));
	}
	delete(m_pImages);
	delete(m_pWindow);
/*
	MessageBox(NULL, "Destructing Background", "Yo", MB_OK);
	delete(m_pBackground);
	MessageBox(NULL, "Destructing Level", "Yo", MB_OK);
	delete(m_pLevel);
	MessageBox(NULL, "Finished", "Goodbye", MB_OK);
*/
}

