#ifndef ERROR_REPORT_H
#define ERROR_REPORT_H

#include <Windows.h>
#include <string>
#include <tchar.h>
//////////////////////////////////////////////////////////////////////////
// private section
//////////////////////////////////////////////////////////////////////////


extern std::basic_string<TCHAR> sReport;
extern std::basic_string<TCHAR> sApplication;

std::basic_string<TCHAR> FormatErrorReport(LPCTSTR description, LPCTSTR file, long line);

LRESULT CALLBACK DlgProcErrorReport(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


//////////////////////////////////////////////////////////////////////////
// pubic section :
//////////////////////////////////////////////////////////////////////////
#define ERROR_REPORT(x, y) do{\
	sApplication = x; \
	sReport = FormatErrorReport(y, _T(__FILE__), __LINE__); \
	DialogBox(NULL, (LPCTSTR)IDD_DIALOG_ERROR_REPORT, NULL, (DLGPROC)DlgProcErrorReport); \
	exit(1); \
} while(0)


#endif // ERROR_REPORT_H