#include <windows.h>

typedef struct  {
	HWND		hMain;						// メインウィンドウのハンドル
	HWND		hVersion;
	HINSTANCE	hInst;						// メインウィンドウのインスタンス
	HWND		hDlg0, hDlg1, hDlg2, hDlg3;	// 子ダイアログのハンドル
	long		mainx, mainy;
	long		deskx, desky;
	long		dlg0x, dlg0y, dlg1x, dlg1y;	// 子ダイアログの大きさ
	HMENU		hMenu;
	HWND		hSbar;						// ステータスバー
	HBITMAP		hBitmap;					// オフスクリーンビットマップ
	HDC			hBuffer;					// ダブルバッファ
	long		count;
} globaldata;

