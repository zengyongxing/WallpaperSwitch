
#ifndef WALLPAPER_UTIL_H
#define WALLPAPER_UTIL_H
#include <windows.h>
#include <tchar.h>
#include <vector>
#include <string>


//////////////////////////////////////////////////////////////////////////
// private section

BOOL IsMutexCreated(LPCTSTR szUniqueMutexName);
BOOL IsActiveDesktopEnabled(void);
BOOL EnableActiveDesktop(BOOL bEnable);
BOOL IsSupportedPicFile(LPCTSTR lpFileName, BOOL bActiveDesktopEnabled);

//////////////////////////////////////////////////////////////////////////
// public section

BOOL HasSameInstanceRunning(void);

std::basic_string<TCHAR> GetNextFile(const std::vector<std::basic_string<TCHAR> >& vsDirectories,
									 LPCTSTR pCurrentFile, 
									 BOOL bRandmon);

BOOL SwitchWallPaper(LPCTSTR file, DWORD style);
std::basic_string<TCHAR> LoadStringFromRes(UINT uID);


void AddNotifyIconInSysTray(HWND hWnd);
void RemoveNotifyIconInSysTray(void);
std::basic_string<TCHAR> OpDirToSelectDir(HWND hDlg, LPCTSTR tip);

#define UM_NOTIFY_ICON (WM_USER + 0x1213)
#define ID_TIMER_SWITCHWALLPAPER 0x23

extern TCHAR g_szMainWindowClassName[MAX_PATH + 1];
void TravelDir(std::vector<std::basic_string<TCHAR> >& vsFiles, const std::basic_string<TCHAR> sDir);
void Switch(void);
void Delete(void);



// extend debug macro
#define CHECK_ERROR(error_condition, description) if(error_condition) \
		ERROR_REPORT(LoadStringFromRes(IDS_SYS_APP_NAME).c_str(), description)

#define CHECK_ERROR_DWORD(error_condition, description, value) do{ \
        if(error_condition)  \
        { \
            TCHAR szBuf[256] = {0}; \
    		_stprintf(szBuf, _T("%s(0x%08d)"), description, value); \
		    ERROR_REPORT(LoadStringFromRes(IDS_SYS_APP_NAME).c_str(), szBuf); \
        } \
	} while(0)

#define CHECK_ERROR_STRING(error_condition, description, value) do{ \
        if(error_condition){ \
	        TCHAR szBuf[256] = {0}; \
		    _stprintf(szBuf, _T("%s(%s)"), description, value); \
		    ERROR_REPORT(LoadStringFromRes(IDS_SYS_APP_NAME).c_str(), szBuf); \
        } \
	} while(0)



//////////////////////////////////////////////////////////////////////////



#endif // WALLPAPER_UTIL_H