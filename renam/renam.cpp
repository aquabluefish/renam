#define _CRT_SECURE_NO_WARNINGS
#include "renam.h"
#include "trace.h"
#include "resource.h"
#include <shlobj.h>
#include <tchar.h>
#include <string.h>
#include <commctrl.h>
#pragma comment(lib, "Comctl32.lib")

char ProgName[100] = "Renum";
char Version[100] = "V.140617";

static int sb_size[] = { 100, 200, -1 };
DWORD dwStyleEx = 0;
NM_LISTVIEW *pNMLV;
listdata d[256];
WNDPROC oldlinkProc = NULL;
globaldata g;

void DebugMsgBox(LPCSTR pszFormat, ...)
{
	va_list	argp;
	char pszBuf[256];
	va_start(argp, pszFormat);
	vsprintf_s(pszBuf, pszFormat, argp);
	va_end(argp);
	MessageBox(NULL, pszBuf, "debug info", MB_OK);
}

char *ptr_char(char *line, char schar)
{
	for (; *line; line++)
		if (*line == schar)
			return(line);
	return(0);
}

char *ptr_char_last(char *line, char schar)
{
	char *i = 0L;
	for (; *line; line++)
		if (*line == schar)
			i = line;
	return(i);
}

char *ptr_string(char *lineBuf, char *wordBuf)
{
	char	*w, *x;

	if (*wordBuf)
		for (; *lineBuf; lineBuf++)
			if (*lineBuf == *wordBuf)
				for (x = lineBuf, w = wordBuf; *x == *w;) {
					if (*++w == NULL) return(lineBuf);
					if (*++x == NULL) break;
				}
	return(0);
}

char *ptr_string_last(char *lineBuf, char *wordBuf)
{
	char	*w, *x;

	char *i = 0L;
	if (*wordBuf)
		for (; *lineBuf; lineBuf++)
			if (*lineBuf == *wordBuf)
				for (x = lineBuf, w = wordBuf; *x == *w;) {
					if (*++w == NULL) {
						i = lineBuf;
						break;
					}
					if (*++x == NULL) break;
				}
	return i;
}

char *charcat(char *line, char c)
{

	strncat(line, &c, 1)[strlen(line) + 1] = NULL;
	return(line);
}

int tran_word(char *dline, char *sline, char *dword, char *sword)
{
	char	line0[1024];
	char	*p;
	int	n;

	n = 0;
	strcpy_s(line0, sline);
	strcpy_s(dline,255,"");

	for (p = line0; *p;)
	if (!strncmp(p, sword, strlen(sword))) {
		strcat(dline, dword);
		p += strlen(sword);
		n++;
	}
	else
		charcat(dline, *p++);

	return(n);
}

void listup(HWND hList)
{
	int nCount, i;
	int mark = 1;

	nCount = ListView_GetItemCount(hList);
	for (i = 0; i < nCount; i++) {
		if (ListView_GetItemState(hList, i, LVIS_SELECTED)) {
			if (!mark) {
				// d[i]��d[i-1]
				x$swap(d[i], d[i - 1]);
				ListView_SetItemState(g.hList, i - 1, LVIS_SELECTED, LVIS_SELECTED)
				ListView_SetItemState(g.hList, i, 0, LVIS_SELECTED)
			}
		}
		else {
			mark = 0;
		}
	}
	return;
}

void listdown(HWND hList)
{
	int nCount, i;
	int mark=1;

	nCount = ListView_GetItemCount(hList);
	for (i = nCount-1; i>=0 ;i--) {
		if (ListView_GetItemState(hList, i, LVIS_SELECTED)) {
			if (!mark) {
				// d[i]��d[i+1]
				x$swap(d[i], d[i + 1]);
				ListView_SetItemState(g.hList, i + 1, LVIS_SELECTED, LVIS_SELECTED)
				ListView_SetItemState(g.hList, i, 0, LVIS_SELECTED)
			}
		}
		else {
			mark = 0;
		}
	}
	return;
}

// �t�@�C�����X�g�̍쐬
int filelist(char *path)
{
	WIN32_FIND_DATA	fd;
	HANDLE		hFind;
	FILETIME	ft;
	SYSTEMTIME	st;
	int i=0;

	hFind = FindFirstFile(path, &fd);
	while (hFind != INVALID_HANDLE_VALUE) {
		if (!(fd.dwFileAttributes&(FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_SYSTEM))) {
			if (fd.cFileName[0] != '.') {
				strcpy_s(d[i].fname, fd.cFileName);						// �t�@�C����
				d[i].fsize = (fd.nFileSizeHigh << 6) + ((fd.nFileSizeLow + 1023) >> 10);	// �t�@�C���T�C�Y
				//d[i].fsize = fd.nFileSizeHigh*MAXDWORD + fd.nFileSizeLow;
				FileTimeToLocalFileTime(&(fd.ftLastWriteTime), &ft);
				FileTimeToSystemTime(&ft, &st);
				wsprintf(d[i].fdate, "%04d/%02d/%02d %02d:%02d",
					st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);		// �t�@�C�����t
				i++;
			}
		}
		if (!FindNextFile(hFind, &fd)) break;
	}
	FindClose(hFind);
	ListView_SetItemCountEx(g.hList, i, LVSICF_NOINVALIDATEALL);
	g.file_n = i;

	return i;
}

void InsColumn(HWND hWnd, char *str, int cx, int iSub)
{
	LV_COLUMN col;

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = cx;
	col.pszText = str;
	col.iSubItem = iSub;
	ListView_InsertColumn(hWnd, iSub, &col);
	return;
}

void init_para(void) {
	char dir[256];
	GetCurrentDirectory(255, dir);                // �J�����g�f�B���N�g���̃p�X���擾
	sprintf_s(g.inifile, "%s\\%s", dir, "renam.ini");    // INI�t�@�C���p�X���쐬
	GetPrivateProfileString("PARAM", "PATH", "", g.path, 255, g.inifile);    // INI�t�@�C���Ǎ���
	SetDlgItemText(g.hDlg0, IDC_DESFILE, g.path);	//�t�H���_���o��
	strcpy_s(g.dir, g.path);
	strcat_s(g.dir, "\\*.*");
}

void save_para(void) {
	GetDlgItemText(g.hDlg0, IDC_DESFILE, g.path, MAX_PATH);	//�����t�H���_�Ǎ���
	WritePrivateProfileString("PARAM", "PATH", g.path, g.inifile);	// INI�t�@�C����������
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED){
		SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);	//�����t�H���_�ݒ�
	}
	return 0;
}

void GetFolder(HWND hdlg)
{
	char dst_file[MAX_PATH];
	BROWSEINFO  binfo;
	LPITEMIDLIST idlist;

	GetDlgItemText(hdlg, IDC_DESFILE, dst_file, MAX_PATH);	//�����t�H���_�Ǎ���
	binfo.hwndOwner = hdlg;
	binfo.pidlRoot = NULL;
	binfo.pszDisplayName = dst_file;
	binfo.lpszTitle = "�t�H���_���w�肵�Ă�������";
	binfo.ulFlags = BIF_RETURNONLYFSDIRS;
	binfo.lpfn = &BrowseCallbackProc;		//�R�[���o�b�N�֐�
	binfo.lParam = (LPARAM)dst_file;		//�R�[���o�b�N�ɓn������
	binfo.iImage = (int)NULL;
	idlist = SHBrowseForFolder(&binfo);
	SHGetPathFromIDList(idlist, dst_file);		//ITEMIDLIST����p�X�𓾂�
	CoTaskMemFree(idlist);				//ITEMIDLIST�̉��
	SetDlgItemText(hdlg, IDC_DESFILE, dst_file);	//�t�H���_���o��
	strcpy_s(g.path, dst_file);
	strcpy_s(g.dir, g.path);
	strcat_s(g.dir, "\\*.*");
}


HFONT SetMyFont(LPCTSTR face, int h, int angle)
{
	HFONT hFont;
	hFont = CreateFont(h,       //�t�H���g����
		0,                      //������
		angle,                  //�e�L�X�g�̊p�x
		0,                      //�x�[�X���C���Ƃ����Ƃ̊p�x
		FW_REGULAR,             //�t�H���g�̏d���i�����j
		FALSE,                  //�C�^���b�N��
		FALSE,                  //�A���_�[���C��
		FALSE,                  //�ł�������
		SHIFTJIS_CHARSET,       //�����Z�b�g
		OUT_DEFAULT_PRECIS,     //�o�͐��x
		CLIP_DEFAULT_PRECIS,    //�N���b�s���O���x
		PROOF_QUALITY,          //�o�͕i��
		FIXED_PITCH | FF_MODERN,        //�s�b�`�ƃt�@�~���[
		face);                  //���̖�
	return hFont;
}

HFONT setFontSetting(HWND hWnd, int ctrlID, int height, int underline)
{
	LOGFONT logfont;
	HFONT	hFont;

	hFont = (HFONT)SendMessage(GetDlgItem(hWnd, ctrlID), WM_GETFONT, 0, 0);
	GetObject(hFont, sizeof(logfont), &logfont);
	logfont.lfHeight = height;
	logfont.lfUnderline = underline;

	hFont = CreateFontIndirect(&logfont);
	SendDlgItemMessage(hWnd, ctrlID, WM_SETFONT, (WPARAM)hFont, 0);

	return hFont;
}

void set_windowsize(void) {

	RECT rt;
	long wx, wy, cx, cy, sx, sy;

	GetWindowRect(g.hMain, &rt);                            // Window�̊O�g�T�C�Y�擾
	wx = rt.right - rt.left;
	wy = rt.bottom - rt.top;

	GetClientRect(g.hMain, &rt);							// Window�̘g���̃T�C�Y�擾
	cx = rt.right - rt.left;
	cy = rt.bottom - rt.top;

	GetClientRect(g.hSbar, &rt);							// �X�e�[�^�X�o�[�̃T�C�Y�擾
	sx = rt.right - rt.left;
	sy = rt.bottom - rt.top;

	GetWindowRect(GetDesktopWindow(), &rt);					// Desktop�̃T�C�Y�擾
	g.deskx = rt.right - rt.left;
	g.desky = rt.bottom - rt.top;

	if (!g.count++) {										// ���񂾂�
		g.mainx = g.dlg0x + (wx - cx);						// Window�T�C�Y��Dialog�ɍ��킹��
		g.mainy = g.dlg0y + (wy - cy) + sy;
		MoveWindow(g.hMain, (g.deskx - g.mainx) / 2, (g.desky - g.mainy) / 2, g.mainx, g.mainy, TRUE);       //Window�̑傫����Dialog�ɍ��킹��
		MoveWindow(g.hDlg0, 0, 0, g.dlg0x, g.dlg0y, TRUE);  //�����Dialog��z�u
	}
}


BOOL CALLBACK dlg0Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	long col, row;
	RECT rt;
	static char szTmp[255] = { "C:\\*.*" };
	LPNMLVCUSTOMDRAW lplvcd;
	char from_name[MAX_PATH], to_name[MAX_PATH];
	char fext[255];
	char *p;
	long retval;

	switch (msg) {

	case WM_INITDIALOG:                     // Window �̏�����
		GetClientRect(hWnd, &rt);			// Dialog���\�[�X�̃T�C�Y�擾
		g.dlg0x = rt.right - rt.left;
		g.dlg0y = rt.bottom - rt.top;

		// ���X�g�n���h���̎擾
		g.hList = GetDlgItem(hWnd, IDC_FLIST);

		// �P�s�I���ƌr���̕\��
		dwStyleEx = ListView_GetExtendedListViewStyle(g.hList);
		dwStyleEx |= (LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE);
		//      dwStyleEx |= ( LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );
		ListView_SetExtendedListViewStyle(g.hList, dwStyleEx);

		// ���ږ��̐ݒ�
		InsColumn(g.hList, "No.", 30, 0);
		InsColumn(g.hList, "Filename",300, 1);
		InsColumn(g.hList, "Size[KB]", 60, 2);
		InsColumn(g.hList, "Date", 120, 3);

		// �s���̐ݒ�
		ListView_SetItemCountEx(g.hList, 0, LVSICF_NOINVALIDATEALL);

		// �Ƃ肠�����A�P�s�ڂ�I����Ԃɂ��Ă���
		ListView_SetItemState(g.hList, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

		return TRUE;

	case WM_NOTIFY:
		LPNMHDR lpnmhdr;
		lpnmhdr = (LPNMHDR)lp;

		switch (((LPNMHDR)lp)->idFrom) {
		case IDC_FLIST:

			switch (lpnmhdr->code) {
			case LVN_GETDISPINFO:  // ���zListView�̕\���f�[�^�⍇��

				LV_DISPINFO *pLvDispInfo;
				pLvDispInfo = (LV_DISPINFO*)lp;
				TCHAR szString[MAX_PATH];

				if (pLvDispInfo->item.mask & LVIF_TEXT) {
					col = pLvDispInfo->item.iSubItem;       // ��ԍ�
					row = pLvDispInfo->item.iItem;          // �s�ԍ�

					switch (col) {
					case 0:
						wsprintf(szString, "%02d", row+1);    // �s�ԍ�
						break;
					case 1:
						wsprintf(szString, "%s", d[row].fname);    // ���O���Z�b�g
						break;
					case 2:
						wsprintf(szString, "%d", d[row].fsize);    // ���e���Z�b�g
						break;
					case 3:
						wsprintf(szString, "%s", d[row].fdate);    // �ݒ�l���Z�b�g
						break;
					}
					if (lstrlen(szString) < pLvDispInfo->item.cchTextMax)
						lstrcpy(pLvDispInfo->item.pszText, szString);    // �\���������Ԃ�
					else
						lstrcpy(pLvDispInfo->item.pszText, _T("****"));  // �����񂪑傫������ꍇ
				}
				break;

//			case LVN_ITEMCHANGED:

			case LVN_ITEMACTIVATE:
				row = ListView_GetNextItem(g.hList, -1, LVIS_SELECTED);   // �I���s�����߂�
				if (row != -1)
					printf(">%d\n", row);
				break;

			case LVN_COLUMNCLICK:   // �J�����w�b�_���̃N���b�N
				pNMLV = (NM_LISTVIEW *)lp;
				printf(">col %d click\n", pNMLV->iSubItem);
				break;

				
			case NM_CUSTOMDRAW:
				// ---- �J�X�^���h���[�ŁAListview�̐F��ݒ�B
				lplvcd = (LPNMLVCUSTOMDRAW)lp;
				switch (lplvcd->nmcd.dwDrawStage) {

				case CDDS_PREPAINT:     // �`��O��ITEM�������N�G�X�g
					SetWindowLong(hWnd, DWL_MSGRESULT, (long)CDRF_NOTIFYITEMDRAW);
					break;
				case CDDS_ITEMPREPAINT:
					row = lplvcd->nmcd.dwItemSpec;  // �s�ԍ�
					if (ListView_GetItemState(g.hList, row, LVIS_SELECTED)) {
						lplvcd->clrTextBk = RGB(0, 255, 255);
						lplvcd->clrText = RGB(0, 0, 0);
					}
					SetWindowLong(hWnd, DWL_MSGRESULT, (long)(CDRF_NEWFONT));
					break;
				}
				break;

			default:
//				SetWindowLong(hWnd, DWL_MSGRESULT, (long)CDRF_DODEFAULT);
				break;
			}
		}
		return TRUE;

	case WM_CLOSE:                          // Window �����
		ShowWindow(hWnd, SW_HIDE);			// ���ۂ͌����Ȃ����Ă��邾��
		UpdateWindow(hWnd);
		return TRUE;

	case WM_SIZE:                           // Window�̃T�C�Y�ύX
		return TRUE;

	case WM_COMMAND:                        // Window �̃R�}���h����
		switch (LOWORD(wp)) {

		case IDC_OK:
			for (row = 0; row < g.file_n; row++) {
				if (p = ptr_char_last(d[row].fname, '.')) {
					strcpy_s(fext, p);								// �g���q�擾
					strcpy_s(from_name, g.path);					// path������
					strcat_s(from_name, "\\");
					strcat_s(from_name, d[row].fname);
					strcpy_s(to_name, from_name);
					if (p = ptr_string_last(to_name, " - ")) {		// �Ō��" - "�ȍ~���폜
						if (ptr_string_last(to_name, " 75 - "))
							p = ptr_string_last(to_name, " 75 - ");	// �Ō��" 75 - "�ȍ~���폜
						*p = '\0';
						tran_word(to_name, to_name, " ", " - ");	// ����ȊO��" - "�͋󔒂ɕϊ�
						strcat_s(to_name, fext);					// �g���q��߂�

						LPVOID lpMsgBuf;
						SetLastError(NO_ERROR);		//�G���[���N���A
						retval = MoveFileEx(from_name, to_name, MOVEFILE_WRITE_THROUGH);
						if (!retval) {
							FormatMessage(				//�G���[�\��������쐬
								FORMAT_MESSAGE_ALLOCATE_BUFFER |
								FORMAT_MESSAGE_FROM_SYSTEM |
								FORMAT_MESSAGE_IGNORE_INSERTS,
								NULL, GetLastError(),
								MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
								(LPTSTR)&lpMsgBuf, 0, NULL);
							DebugMsgBox("No.%02d : MoveFileEx Error : %s", row+1, lpMsgBuf);
							LocalFree(lpMsgBuf);
							break;
						}
					}
				}
			}
			filelist(g.dir);
			InvalidateRect(g.hList, NULL, TRUE);
			return TRUE;

		case IDC_CANCEL:
			return TRUE;

		case IDC_GETFOLDER:
			GetFolder(g.hDlg0);
			filelist(g.dir);
			InvalidateRect(g.hList, NULL, TRUE);
			return TRUE;

		case IDC_UP:
			listup(g.hList);
			InvalidateRect(g.hList, NULL, TRUE);
			return TRUE;

		case IDC_DOWN:
			listdown(g.hList);
			InvalidateRect(g.hList, NULL, TRUE);
			return TRUE;

		case IDC_SELECTALL:
			ListView_SetItemState(g.hList, -1, LVIS_SELECTED, LVIS_SELECTED)
				return TRUE;

		case IDC_UNSELECTALL:
			ListView_SetItemState(g.hList, -1, 0, LVIS_SELECTED)
				return TRUE;

		default:
			return (DefWindowProc(hWnd, msg, wp, lp));      //�������Ȃ����̂̓V�X�e���ɓn��
		}
	}
	return FALSE;
}

LRESULT CALLBACK linkProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	HDC hdc;
	PAINTSTRUCT ps;
	COLORREF cr;
	LOGFONT lf;
	HFONT hFont;
	HCURSOR hCursor;

	switch (msg) {
	case WM_SETCURSOR:		// �}�E�X�|�C���^�̕ύX
		hCursor = LoadCursor(g.hInst, MAKEINTRESOURCE(IDI_CURSOR1));
		SetCursor(hCursor);
		return 0;
	case WM_LBUTTONDOWN:	// �u���E�U��URL���J��
		ShellExecute(hWnd, "open", g.linkurl, NULL, NULL, SW_SHOWDEFAULT);
		return 0;
	case WM_PAINT:			// �����N������ɃA���_�[�o�[��t���Đ���������
		hdc = BeginPaint(hWnd, &ps);
		memset(&lf, 0, sizeof(LOGFONT));
//		strcpy_s(lf.lfFaceName, "system");
		lf.lfUnderline = TRUE;
		hFont = CreateFontIndirect(&lf);
		SelectObject(hdc, (HGDIOBJ)hFont);
		cr = GetSysColor(COLOR_MENU);
		SetBkColor(hdc, cr);
		SetTextColor(hdc, RGB(0, 0, 255));
		SetBkMode(hdc, TRANSPARENT);
		TextOut(hdc, 0, 0, g.linkurl, strlen(g.linkurl));
		DeleteObject(hFont);
		EndPaint(hWnd, &ps);
		return 0;
	}
	return (CallWindowProc(oldlinkProc, hWnd, msg, wp, lp));
}

// About�_�C�A���O�\��
LRESULT CALLBACK VersionProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		g.hAbout = hdlg;
		SetDlgItemText(hdlg, IDC_PROGNAME, ProgName);
		SetDlgItemText(hdlg, IDC_VERSION, Version);
		setFontSetting(hdlg, IDC_PROGNAME, 18, 0);
		setFontSetting(hdlg, IDC_VERSION, 12, 0);
		//URL�����N������̃T�u�N���X��
		oldlinkProc = (WNDPROC)GetWindowLong(GetDlgItem(hdlg, IDC_LINKURL), GWL_WNDPROC);
		SetWindowLong(GetDlgItem(hdlg, IDC_LINKURL), GWL_WNDPROC, (LONG)linkProc);
		GetDlgItemText(hdlg, IDC_LINKURL, g.linkurl, 255);	//URL������
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

	case WM_CREATE:												//�E�B���h�E���J������
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
		init_para();
		filelist(g.dir);
		InvalidateRect(g.hList, NULL, TRUE);
		break;

	case WM_SIZE:
		set_windowsize();
		SendMessage(g.hSbar, WM_SIZE, wp, lp);
		break;

	case WM_GETMINMAXINFO:
		MINMAXINFO FAR * lpmm;
		lpmm = (MINMAXINFO FAR *)lp;
		lpmm->ptMaxPosition.x = 0;              // �ő�\�����̈ʒu
		lpmm->ptMaxPosition.y = 0;
		lpmm->ptMaxSize.x = 640;                // �ő�\�����̃T�C�Y
		lpmm->ptMaxSize.y = 480;
		lpmm->ptMinTrackSize.x = g.mainx;		// �ύX�\�ȍŏ��T�C�Y
		lpmm->ptMinTrackSize.y = g.mainy;
		lpmm->ptMaxTrackSize.x = 640;			// �ύX�\�ȍő�T�C�Y
		lpmm->ptMaxTrackSize.y = 480;
		break;

	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDM_VERSION:
			DialogBox(g.hInst, MAKEINTRESOURCE(IDD_VERSION), hWnd, (DLGPROC)VersionProc);
			break;

		case IDM_GETFOLDER:
			GetFolder(g.hDlg0);
			filelist(g.dir);
			InvalidateRect(g.hList, NULL, TRUE);
			break;

		case IDM_UP:
			listup(g.hList);
			InvalidateRect(g.hList, NULL, TRUE);
			break;

		case IDM_DOWN:
			listdown(g.hList);
			InvalidateRect(g.hList, NULL, TRUE);
			break;

		case IDM_SELECTALL:
			ListView_SetItemState(g.hList, -1, LVIS_SELECTED, LVIS_SELECTED)
			break;

		case IDM_UNSELECTALL:
			ListView_SetItemState(g.hList, -1, 0, LVIS_SELECTED)
			break;

		case IDM_EXIT:
			PostMessage(hWnd, WM_CLOSE, wp, lp);
			break;

		default:
			return DefWindowProc(hWnd, msg, wp, lp);
		}
		break;

	case WM_DESTROY:
		save_para();
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
	wcex.lpszClassName = ProgName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(NULL));      // small ICON
	if (!RegisterClassEx(&wcex)) return 0;

	hWnd = CreateWindow(ProgName, ProgName, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
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
