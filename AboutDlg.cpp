#include "resource.h"
#include "AboutDlg.h"
#include "Wallpaper_util.h"
#include <tchar.h>

//////////////////////////////////////////////////////////////////////////
// Dialog process
LRESULT CALLBACK DlgProcAboutDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hStatic; 
	static HICON hIcon;

	HWND hwndParent = GetParent(hDlg);

	switch (message)
	{
	case WM_INITDIALOG:
		{
            // About dialog title
            ::SetWindowText(hDlg, 
                (LoadStringFromRes(IDS_ABOUT) + LoadStringFromRes(IDS_APP_NAME_LOCAL)).c_str());

            // My Words
            hStatic = GetDlgItem(hDlg, IDC_STATIC_DESCRIPTION);
            ::SetWindowText(hStatic, 
                (LoadStringFromRes(IDS_MY_WORDS)).c_str());

            // Version
			hStatic = GetDlgItem(hDlg, IDC_STATIC_VERSION);
			::SetWindowText(hStatic, 
				(LoadStringFromRes(IDS_APP_NAME_LOCAL) + _T(" V")
					+ LoadStringFromRes(IDS_APP_VERSION) + _T(" ")
					+ LoadStringFromRes(IDS_APP_DESCRIPTION)
					).c_str());

			// Icon
			hStatic = GetDlgItem(hDlg, IDC_STATIC_ICON);
			hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_WALLPAPER));

			SendMessage( hStatic, STM_SETICON, (WPARAM)hIcon, 0);

			return TRUE;
		}

	case WM_CLOSE:
		{
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}

	default: return FALSE;
	}//switch message;
}

