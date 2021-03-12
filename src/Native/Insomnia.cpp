#include "stdafx.h"
#include "Insomnia.h"

// Constants
#define WINDOW_BACKGROUND (RGB(220, 220, 220))
#define WM_NOTIFYICON (WM_USER + 10)
#define MAX_LOADSTRING (80)
#define IDEAL_SIZE_STATIC (TEXT("IdealSizeStatic"))
#define WM_GETIDEALSIZE (WM_USER)
#define INDEX_TEXT (0)
#define INDEX_FONT (sizeof(LONG_PTR))

// Global variables
TCHAR szTitle[MAX_LOADSTRING] = {0};
TCHAR szText[MAX_LOADSTRING] = {0};
TCHAR szVersion[MAX_LOADSTRING] = {0};
TCHAR szLink[MAX_LOADSTRING] = {0};
TCHAR szStesError[MAX_LOADSTRING] = {0};
HINSTANCE g_hInstance = NULL;
HWND g_hWnd = NULL;
HWND hWndText = NULL;
HWND hWndVersion = NULL;
HWND hWndLink = NULL;
HWND hWndStatusArea = NULL;

LRESULT CALLBACK WndProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
	static BOOL fNotifyBalloonShown = FALSE;

	switch (message)
	{
	case WM_CREATE:
		{
			// Center window on the desktop
			const HWND hWndDesktop = GetDesktopWindow();
			RECT ownerRect = {0};
			if (GetWindowRect(hWndDesktop, &ownerRect))
			{
				const CREATESTRUCT* const pCreateStruct = (CREATESTRUCT*)lParam;
				SetWindowPos(hWnd, NULL, ((ownerRect.right - ownerRect.left) - pCreateStruct->cx) / 2, ((ownerRect.bottom - ownerRect.top) - pCreateStruct->cy) / 2, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
			}
		}
		return 0;
	case WM_SIZE:
		{
			static HICON hIcon = NULL;
			NOTIFYICONDATA notifyIconData = {0};
			notifyIconData.cbSize = sizeof(notifyIconData);
			notifyIconData.uFlags = NIF_MESSAGE;
			notifyIconData.hWnd = hWnd;
			notifyIconData.uCallbackMessage = WM_NOTIFYICON;
			switch(wParam)
			{
			case SIZE_RESTORED:
				{
					// Layout controls
					const LONG margin = 4;
					const WORD availableWidth = LOWORD(lParam);
					WORD availableHeight = HIWORD(lParam);
					SIZE idealVersionSize = {0};
					(void)SendMessage(hWndVersion, WM_GETIDEALSIZE, 0, (LPARAM)&idealVersionSize);
					MoveWindow(hWndVersion, availableWidth - idealVersionSize.cx - margin, availableHeight - idealVersionSize.cy - margin, idealVersionSize.cx, idealVersionSize.cy, TRUE);
					SIZE idealLinkSize = {0};
					(void)SendMessage(hWndLink, WM_GETIDEALSIZE, 0, (LPARAM)&idealLinkSize);
					MoveWindow(hWndLink, margin, availableHeight - idealLinkSize.cy - margin, idealLinkSize.cx, idealLinkSize.cy, TRUE);
					const LONG statusHeight = ((idealVersionSize.cy <= idealLinkSize.cy) ? idealLinkSize.cy : idealVersionSize.cy) + margin + margin;
					availableHeight -= (WORD)statusHeight;
					MoveWindow(hWndStatusArea, 0, availableHeight, availableWidth, statusHeight, TRUE);
					SIZE idealTextSize = {0};
					(void)SendMessage(hWndText, WM_GETIDEALSIZE, 0, (LPARAM)&idealTextSize);
					MoveWindow(hWndText, (availableWidth - idealTextSize.cx) / 2, (availableHeight - idealTextSize.cy) / 2, idealTextSize.cx, idealTextSize.cy, TRUE);

					// Remove notification icon (if present)
					(void)Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
					if (NULL != hIcon)
					{
						(void)DestroyIcon(hIcon);
						hIcon = NULL;
					}
				}
				break;
			case SIZE_MINIMIZED:
				// Add notification icon
				hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_INSOMNIA), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, 0);
				if (NULL != hIcon)
				{
					notifyIconData.hIcon = hIcon;
					notifyIconData.uFlags |= NIF_ICON;
				}
				if (SUCCEEDED(StringCchCopy(notifyIconData.szTip, ARRAYSIZE(notifyIconData.szTip), szTitle)))
				{
					notifyIconData.uFlags |= NIF_TIP;
				}
				if (!fNotifyBalloonShown && SUCCEEDED(StringCchCopy(notifyIconData.szInfo, ARRAYSIZE(notifyIconData.szInfo), szTitle)))
				{
					notifyIconData.uFlags |= NIF_INFO;
					fNotifyBalloonShown = true;
				}
				(void)Shell_NotifyIcon(NIM_ADD, &notifyIconData);

				// Remove taskbar button
				(void)ShowWindow(hWnd, SW_HIDE);
				break;
			}
		}
		return 0;
	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == hWndText)
		{
			// Change color of text message to red
			const HDC hdc = (HDC)wParam;
			(void)SetTextColor(hdc, RGB(255, 0, 0));
			(void)SetBkMode(hdc, TRANSPARENT);
			return (INT_PTR)CreateSolidBrush(WINDOW_BACKGROUND);
		}
		break;
	case WM_NOTIFYICON:
		{
			const WORD msg = LOWORD(lParam);
			if ((WM_LBUTTONDOWN == msg) || (WM_RBUTTONDOWN == msg))
			{
				// Handle left/right click on notify icon by restoring the window
				(void)ShowWindow(hWnd, SW_NORMAL);
			}
		}
		return 0;
	case WM_DESTROY:
		// Exit the application
		(void)PostQuitMessage(0);
		return 0;
	}

	// Default processing
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK IdealSizeStaticWndProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		{
			const CREATESTRUCT* const pCreateStruct = (CREATESTRUCT*)lParam;
			(void)SetWindowLongPtr(hWnd, INDEX_TEXT, (LONG_PTR)(_tcsdup(pCreateStruct->lpszName)));
			(void)SetWindowLongPtr(hWnd, INDEX_FONT, (LONG_PTR)GetStockObject(DEFAULT_GUI_FONT));
		}
		return 0;
	case WM_SETFONT:
		(void)SetWindowLongPtr(hWnd, INDEX_FONT, (LONG_PTR)wParam);
		if (LOWORD(lParam))
		{
			(void)InvalidateRect(hWnd, NULL, TRUE);
		}
		return 0;
	case WM_GETIDEALSIZE:
		if (NULL != lParam)
		{
			SIZE* const pIdealSize = (SIZE*)lParam;
			(void)ZeroMemory(pIdealSize, sizeof(*pIdealSize));
			const HDC hdc = GetDC(hWnd);
			if (NULL != hdc)
			{
				LPTSTR pText = (LPTSTR)GetWindowLongPtr(hWnd, INDEX_TEXT);
				if ((NULL != pText) && (TEXT('_') == *pText))
				{
					pText++;
				}
				const HFONT hFont = (HFONT)GetWindowLongPtr(hWnd, INDEX_FONT);
				const HGDIOBJ originalObject = SelectObject(hdc, hFont);
				RECT rc = {0};
				if (0 != DrawTextEx(hdc, pText, -1, &rc, DT_CALCRECT, NULL))
				{
					pIdealSize->cx = rc.right - rc.left;
					pIdealSize->cy = rc.bottom - rc.top;
				}
				(void)SelectObject(hdc, originalObject);
				(void)ReleaseDC(hWnd, hdc);
			}
			return 0;
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps = {0};
			const HDC hdc = BeginPaint(hWnd, &ps);
			if (NULL != hdc)
			{
				const HWND hWndParent = GetParent(hWnd);
				if (NULL != hWndParent)
				{
					(void)SendMessage(hWndParent, WM_CTLCOLORSTATIC, (WPARAM)hdc, (LPARAM)hWnd);
				}
				LPTSTR pText = (LPTSTR)GetWindowLongPtr(hWnd, INDEX_TEXT);
				if ((NULL != pText) && (TEXT('_') == *pText))
				{
					(void)SetTextColor(hdc, RGB(0, 0, 255));
					pText++;
				}
				const HFONT hFont = (HFONT)GetWindowLongPtr(hWnd, INDEX_FONT);
				const HGDIOBJ originalObject = SelectObject(hdc, hFont);
				RECT rc = {0};
				(void)GetClientRect(hWnd, &rc);
				(void)DrawTextEx(hdc, pText, -1, &rc, 0, NULL);
				if (hWnd == GetFocus())
				{
					(void)DrawFocusRect(hdc, &rc);
				}
				(void)SelectObject(hdc, originalObject);
				(void)EndPaint(hWnd, &ps);
			}
		}
		return 0;
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		(void)InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_LBUTTONDOWN:
	case WM_KEYDOWN:
		{
			LPTSTR pText = (LPTSTR)GetWindowLongPtr(hWnd, INDEX_TEXT);
			if ((NULL != pText) && (TEXT('_') == *pText))
			{
				pText++;
				if ((WM_LBUTTONDOWN == message) || (VK_SPACE == wParam))
				{
					SHELLEXECUTEINFO sei = {0};
					sei.cbSize = sizeof(sei);
					sei.hwnd = hWnd;
					sei.lpFile = pText;
					sei.nShow = SW_NORMAL;
					(void)ShellExecuteEx(&sei);
					return 0;
				}
			}
		}
		break;
	case WM_DESTROY:
		{
			const LPTSTR pText = (LPTSTR)GetWindowLongPtr(hWnd, INDEX_TEXT);
			(void)free(pText);
		}
		return 0;
	}

	// Default processing
	return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL InitInstance(const HINSTANCE hInstance, const int nCmdShow)
{
	// Create application window
	const HWND hWnd = CreateWindowEx(WS_EX_TOPMOST, szTitle, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 200, NULL, NULL, hInstance, NULL);
	if (NULL == hWnd)
	{
		return FALSE;
	}
	g_hWnd = hWnd;

	// Create child windows
	hWndText = CreateWindowEx(0, IDEAL_SIZE_STATIC, szText, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, hInstance, NULL);
	hWndStatusArea = CreateWindowEx(0, TEXT("Static"), NULL, WS_CHILD | WS_VISIBLE | SS_SUNKEN, 0, 0, 0, 0, hWnd, NULL, hInstance, NULL);
	hWndVersion = CreateWindowEx(0, IDEAL_SIZE_STATIC, szVersion, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, hInstance, NULL);
	hWndLink = CreateWindowEx(0, IDEAL_SIZE_STATIC, szLink, WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, hWnd, NULL, hInstance, NULL);
	if ((NULL == hWndText) || (NULL == hWndStatusArea) || (NULL == hWndVersion) || (NULL == hWndLink))
	{
		return FALSE;
	}

	// Customize control fonts
	const HFONT hFontText = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, NULL);
	(void)SendMessage(hWndText, WM_SETFONT, (WPARAM)hFontText, FALSE);
	const HFONT hFontVersionLink = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Calibri"));
	(void)SendMessage(hWndVersion, WM_SETFONT, (WPARAM)hFontVersionLink, FALSE);
	(void)SendMessage(hWndLink, WM_SETFONT, (WPARAM)hFontVersionLink, FALSE);

	// Show application window
	(void)ShowWindow(hWnd, nCmdShow);
	(void)UpdateWindow(hWnd);
	return TRUE;
}

int APIENTRY _tWinMain(const HINSTANCE hInstance, const HINSTANCE hPrevInstance, const LPTSTR lpCmdLine, const int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	g_hInstance = hInstance;

	//Load resource strings
	if ((0 == LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING)) ||
		(0 == LoadString(hInstance, IDS_APP_TEXT, szText, MAX_LOADSTRING)) ||
		(0 == LoadString(hInstance, IDS_APP_VERSION, szVersion, MAX_LOADSTRING)) ||
		(0 == LoadString(hInstance, IDS_APP_LINK, szLink, MAX_LOADSTRING)) ||
		(0 == LoadString(hInstance, IDS_STES_ERROR, szStesError, MAX_LOADSTRING)))
	{
		return FALSE;
	}

	// Window class info (main window)
	WNDCLASSEX wcex = {0};
	wcex.cbSize = sizeof(wcex);
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_INSOMNIA));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(WINDOW_BACKGROUND);
	wcex.lpszClassName = szTitle;

	// Window class info (IdealSizeStatic)
	WNDCLASSEX wcexisl = {0};
	wcexisl.cbSize = sizeof(wcexisl);
	wcexisl.lpfnWndProc = IdealSizeStaticWndProc;
	wcexisl.hInstance = hInstance;
	wcexisl.lpszClassName = IDEAL_SIZE_STATIC;
	wcexisl.cbWndExtra = 2 * sizeof(LONG_PTR);

	// Initialize
	if ((0 == RegisterClassEx(&wcex)) ||
		(0 == RegisterClassEx(&wcexisl)) ||
		!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	MSG msg = {0};
	// Prevent the system from entering sleep mode
	const EXECUTION_STATE previousExecutionState = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
	if (NULL != previousExecutionState)
	{
		// Message loop
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (!IsDialogMessage(g_hWnd, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		// Restore previous state
		(void)SetThreadExecutionState(previousExecutionState);
	}
	else
	{
		// Display error
		(void)MessageBox(g_hWnd, szStesError, szTitle, MB_OK | MB_ICONERROR);
	}

	return (int)msg.wParam;
}
