#include <windows.h>

typedef struct  {
	HWND		hMain;						// ���C���E�B���h�E�̃n���h��
	HWND		hVersion;
	HINSTANCE	hInst;						// ���C���E�B���h�E�̃C���X�^���X
	HWND		hDlg0, hDlg1, hDlg2, hDlg3;	// �q�_�C�A���O�̃n���h��
	long		mainx, mainy;
	long		deskx, desky;
	long		dlg0x, dlg0y, dlg1x, dlg1y;	// �q�_�C�A���O�̑傫��
	HMENU		hMenu;
	HWND		hSbar;						// �X�e�[�^�X�o�[
	HBITMAP		hBitmap;					// �I�t�X�N���[���r�b�g�}�b�v
	HDC			hBuffer;					// �_�u���o�b�t�@
	HWND		hList;						// ���X�g�r���[

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

