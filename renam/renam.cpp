#include "renam.h"
#include "trace.h"
#include "resource.h"
#include <tchar.h>
#include <commctrl.h>
#pragma comment(lib, "Comctl32.lib")

TCHAR szTitle[100] = _T("My Program");
static int sb_size[] = { 100, 200, -1 };
globaldata g;

void set_font(HWND hWnd, long ID, long nHeight) {
	HWND hStatic = GetDlgItem(hWnd, ID);

	HFONT hFont = CreateFont(nHeight,/* nHeight */
		0,/* nWidth */
		0,/* nEscapement */
		0,/* nOrientatioon */
		FW_DEMIBOLD,/* fnWeight */
		(DWORD)FALSE,/* fdwItalic */
		(DWORD)FALSE,/* fdwUnderline */
		(DWORD)FALSE,/* fdwStrikeOut */
		(DWORD)ANSI_CHARSET,/* fdwCharSet */
		(DWORD)OUT_DEFAULT_PRECIS,/* fdwOutputPrecision */
		(DWORD)CLIP_DEFAULT_PRECIS,/* fdwClipPrecision */
		(DWORD)PROOF_QUALITY,/* fdwQuality */
		(DWORD)DEFAULT_PITCH,/* fdwPitchAndFamily */
		_T("ＭＳ ゴシック"));/* lpszFace */

	SendMessage(hStatic, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void set_windowsize(void) {

	RECT rt;
	long wx, wy, cx, cy, sx, sy;

	GetWindowRect(g.hMain, &rt);                            // Windowの外枠サイズ取得
	wx = rt.right - rt.left;
	wy = rt.bottom - rt.top;

	GetClientRect(g.hMain, &rt);							// Windowの枠内のサイズ取得
	cx = rt.right - rt.left;
	cy = rt.bottom - rt.top;

	GetClientRect(g.hSbar, &rt);							// ステータスバーのサイズ取得
	sx = rt.right - rt.left;
	sy = rt.bottom - rt.top;

	GetWindowRect(GetDesktopWindow(), &rt);					// Desktopのサイズ取得
	g.deskx = rt.right - rt.left;
	g.desky = rt.bottom - rt.top;

	if (!g.count++) {										// 初回だけ
		g.mainx = g.dlg0x + (wx - cx);						// WindowサイズをDialogに合わせる
		g.mainy = g.dlg0y + (wy - cy) + sy;
		MoveWindow(g.hMain, (g.deskx - g.mainx) / 2, (g.desky - g.mainy) / 2, g.mainx, g.mainy, TRUE);       //Windowの大きさをDialogに合わせる
		MoveWindow(g.hDlg0, 0, 0, g.dlg0x, g.dlg0y, TRUE);  //左上にDialogを配置
	}
}

BOOL CALLBACK dlg0Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	RECT rt;
	switch (msg) {

	case WM_INITDIALOG:                     // Window の初期化
		GetClientRect(hWnd, &rt);			// Dialogリソースのサイズ取得
		g.dlg0x = rt.right - rt.left;
		g.dlg0y = rt.bottom - rt.top;
		return TRUE;

	case WM_CLOSE:                          // Window を閉じる
		ShowWindow(hWnd, SW_HIDE);			// 実際は見えなくしているだけ
		UpdateWindow(hWnd);
		return TRUE;

	case WM_SIZE:                           // Windowのサイズ変更
		return TRUE;

	case WM_COMMAND:                        // Window のコマンド処理
		switch (LOWORD(wp)) {
		case IDOK:
			return TRUE;
		case IDCANCEL:
			return TRUE;

		default:
			return (DefWindowProc(hWnd, msg, wp, lp));      //処理しないものはシステムに渡す
		}
	}
	return FALSE;
}

// Aboutダイアログ表示
LRESULT CALLBACK VersionProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		g.hVersion = hdlg;
		set_font(hdlg, IDC_PROCNAME,18);
		set_font(hdlg, IDC_VERSION,12);
		return FALSE;
	case WM_LBUTTONDOWN:
		EndDialog(hdlg, IDOK);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {

	case WM_CREATE:												//ウィンドウが開いたら
		InitCommonControls();
		g.count = 0;
		ShowWindow(g.hDlg0, SW_SHOW);
		UpdateWindow(g.hDlg0);
		g.hMenu = GetMenu(hWnd);
		g.hDlg0 = CreateDialog(g.hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)dlg0Proc);
		g.hSbar = CreateStatusWindow(
			WS_CHILD | WS_VISIBLE |
			CCS_BOTTOM | SBARS_SIZEGRIP,
			"sTaTus", hWnd, 1
		);
		SendMessage(g.hSbar, SB_SETPARTS, 3, (LPARAM)sb_size);
		SendMessage(g.hSbar, SB_SETTEXT, 0 | 1, (LPARAM)"test");
		break;

	case WM_SIZE:
		set_windowsize();
		SendMessage(g.hSbar, WM_SIZE, wp, lp);
		break;

	case WM_GETMINMAXINFO:
		MINMAXINFO FAR * lpmm;
		lpmm = (MINMAXINFO FAR *)lp;
		lpmm->ptMaxPosition.x = 0;              // 最大表示時の位置
		lpmm->ptMaxPosition.y = 0;
		lpmm->ptMaxSize.x = 640;                // 最大表示時のサイズ
		lpmm->ptMaxSize.y = 480;
		lpmm->ptMinTrackSize.x = g.mainx;		// 変更可能な最小サイズ
		lpmm->ptMinTrackSize.y = g.mainy;
		lpmm->ptMaxTrackSize.x = 640;			// 変更可能な最大サイズ
		lpmm->ptMaxTrackSize.y = 480;
		break;

	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDM_VERSION:
			DialogBox(g.hInst, MAKEINTRESOURCE(IDD_VERSION), hWnd, (DLGPROC)VersionProc);
			break;

		case IDM_EXIT:
			PostMessage(hWnd, WM_CLOSE, wp, lp);
			break;

		default:
			return DefWindowProc(hWnd, msg, wp, lp);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, msg, wp, lp);
	}
	return 0;
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	HWND hWnd;
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(NULL));           // ICON
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);                // MENU
	wcex.lpszClassName = szTitle;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(NULL));      // small ICON
	if (!RegisterClassEx(&wcex)) return 0;

	hWnd = CreateWindow(szTitle, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, g.hInst, NULL);
	if (!hWnd) {
		return 0;
	}

	g.hInst = hInstance;
	g.hMain = hWnd;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
