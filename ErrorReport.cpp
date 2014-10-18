
#include "ErrorReport.h"
#include "resource.h"
#include <tchar.h>

std::basic_string<TCHAR> sReport;
std::basic_string<TCHAR> sApplication;


//////////////////////////////////////////////////////////////////////////
// format error report string in edit box
std::basic_string<TCHAR> FormatErrorReport(LPCTSTR description, LPCTSTR file, long line)
{
	TCHAR aBuf[256] = {0};
	_stprintf(aBuf, _T("ERROR\t: %s\r\r\nFILE\t: %s \r\r\nLINE\t: %d\n\r"), description, file, line);
	return std::basic_string<TCHAR>(aBuf);
}

//////////////////////////////////////////////////////////////////////////
// Dialog process
LRESULT CALLBACK DlgProcErrorReport(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId;																																
	int wmEvent;

	HWND hwndParent = GetParent(hDlg);

	switch (message)
	{
	case WM_INITDIALOG:
		{
			// set window title
			std::basic_string<TCHAR> sTitle;
			sTitle = _T("Error Report - ") + sApplication;
			::SetWindowText(hDlg, sTitle.c_str());

			// Set error report in edit
			HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_REPORT);
			::SetWindowText(hEdit, sReport.c_str());

			// Set dialog to center
			//::SetWindowPos()

			return TRUE;
		}

	case WM_CLOSE:
		{
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}

	case WM_COMMAND:
		{
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 

			switch (wmId)
			{
			case IDOK:
				{
					EndDialog(hDlg, wmId);
					return TRUE;
				}

			case ID_BUTTON_SEND:
				{
					return TRUE;
				}
			}
		}//case WM_COMMAND

	}//switch message;

	return FALSE;
}

