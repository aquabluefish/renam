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
	long		count;
} globaldata;

