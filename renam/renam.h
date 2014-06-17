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
	HWND		hList;						// リストビュー

	HCURSOR		hCur;
	HFONT		hLinkfont;

	long		count;
	char		path[MAX_PATH];
	char		dir[MAX_PATH];
	char		inifile[MAX_PATH];
} globaldata;

typedef struct {
	char	fname[256];
	long	fsize;
	char	fdate[32];
} listdata;

#define x$swap(a,b) do {					\
struct temp_t { char t[sizeof(a)]; } temp;	\
	temp = *(struct temp_t*)&(a);			\
	*(struct temp_t*)&(a) = *(struct temp_t*)&(b);	\
	*(struct temp_t*)&(b) = temp; } while (0)

