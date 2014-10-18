
#include <windows.h>
#include <tchar.h>
#include <wininet.h>
#include <Shlobj.h>
#include "DataManager.h"
#include "resource.h"
#include "wallpaper_util.h"
#include "ErrorReport.h"

using namespace std;

// Globle Data ///////////////////////////////////////////////////////////
CDataManager DataManager;

// constructor and destructor ///////////////////////////////////////////
// Init all parameter before any operation 
CDataManager::CDataManager()
{
    InitWithDefaultValue();
    InitWithRegistry();
}

// destructor ////////////////////////////////////////////////////////////
// save any parameter changed back to registry
CDataManager::~CDataManager()
{
    SaveBackToRegistry();
}

// data operation ////////////////////////////////////////////////////////
// default value for parameters
void CDataManager::InitWithDefaultValue(void)
{
    bRandomPic      = TRUE;
    udwOption		= WPSTYLE_CENTER;
    bWithOSStartup  = TRUE;												
    bTimelySwitch   = FALSE;
    udwHours        = 0;
    udwMinutes      = 30;
	sCurrentFile.clear();
    vsSearchDirectories.clear();

	bLastTimeActiveDesktopEnabled	= TRUE;
	bQuestionAboutActiveDesktop		= TRUE;

}
//////////////////////////////////////////////////////////////////////////
// get value from registry
void CDataManager::InitWithRegistry(void)
{
	HKEY hKey = NULL;
    LONG lRes = ERROR_SUCCESS;
	DWORD dwValue = 0;
	DWORD dwCount = sizeof(DWORD);

    if(ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, LoadStringFromRes(IDS_SYS_REGISTER_KEY).c_str(), 0, KEY_READ, &hKey))
	{
	
		if(ERROR_SUCCESS == ::RegQueryValueEx(hKey, _T("bRandomPic"), NULL, NULL,(LPBYTE)&dwValue, &dwCount))
			bRandomPic = (BOOL) dwValue;

		if(ERROR_SUCCESS == ::RegQueryValueEx(hKey, _T("udwOption"), NULL, NULL,(LPBYTE)&dwValue, &dwCount))
			udwOption = dwValue;

		if(ERROR_SUCCESS == ::RegQueryValueEx(hKey, _T("bTimelySwitch"), NULL, NULL,(LPBYTE)&dwValue, &dwCount))
			bTimelySwitch = (BOOL) dwValue;

		if(ERROR_SUCCESS == ::RegQueryValueEx(hKey, _T("udwHours"), NULL, NULL,(LPBYTE)&dwValue, &dwCount))
			udwHours = dwValue;
		
		if( ERROR_SUCCESS == ::RegQueryValueEx(hKey, _T("udwMinutes"), NULL, NULL,(LPBYTE)&dwValue, &dwCount))
			udwMinutes = dwValue;

		if( ERROR_SUCCESS == ::RegQueryValueEx(hKey, _T("bLastTimeActiveDesktopEnabled"), NULL, NULL,(LPBYTE)&dwValue, &dwCount))
			bLastTimeActiveDesktopEnabled = (BOOL) dwValue;

		if( ERROR_SUCCESS == ::RegQueryValueEx(hKey, _T("bQuestionAboutActiveDesktop"), NULL, NULL,(LPBYTE)&dwValue, &dwCount))
			bQuestionAboutActiveDesktop = (BOOL) dwValue;


		if( ERROR_SUCCESS == ::RegQueryValueEx(hKey, _T("sCurrentFile"), NULL, NULL,NULL, &dwCount))
		{
			BYTE* pByte = new BYTE[dwCount];
			memset(pByte, 0, dwCount);
			::RegQueryValueEx(hKey, _T("sCurrentFile"), NULL, NULL, pByte, &dwCount);
			sCurrentFile = (LPCTSTR)pByte;
			delete pByte;
		}
		
		if(ERROR_SUCCESS == ::RegQueryValueEx(hKey, _T("vsSearchDirectories"), NULL, NULL, NULL, &dwCount))
		{
			if(dwCount > 0) 
			{
				vsSearchDirectories.clear();

			    BYTE* pByte = new BYTE[dwCount];
		        if( ERROR_SUCCESS == ::RegQueryValueEx(hKey, _T("vsSearchDirectories"), NULL, NULL, pByte, &dwCount))
				{
			        LPCTSTR pChar = (LPCTSTR)pByte;
		            while(*pChar != NULL)
	                {
						vsSearchDirectories.push_back(pChar);
						pChar += _tcslen(pChar) + 1;
					}
				}
				delete pByte;
			}
		}
		::RegCloseKey(hKey);
	} // end of RegOpenKeyEx

	// read SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run
    if(ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, LoadStringFromRes(IDS_SYS_REGISTER_RUN).c_str(), 0, KEY_READ, &hKey))
	{

		TCHAR szCmdLine[MAX_PATH + 7] = {0};
		dwCount = MAX_PATH + 7;
	    if(ERROR_SUCCESS == ::RegQueryValueEx(hKey, LoadStringFromRes(IDS_SYS_APP_NAME).c_str(), 
								NULL, NULL,(LPBYTE)&szCmdLine, &dwCount))
		{
			bWithOSStartup = (0 == _tcscmp(FormatCmdLine().c_str(), szCmdLine));
		}
		else
		{
			bWithOSStartup = FALSE;
		}

		::RegCloseKey(hKey);
	}// end of RegOpenKeyEx
}

//////////////////////////////////////////////////////////////////////////
// write parameter back to registry
void CDataManager::SaveBackToRegistry(void)
{
    HKEY hKey = NULL;
    LONG lRes = ERROR_SUCCESS;

    if(ERROR_SUCCESS == ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, LoadStringFromRes(IDS_SYS_REGISTER_KEY).c_str(), NULL, NULL, 
                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL))
	{
		::RegSetValueEx(hKey, _T("bRandomPic"), NULL, REG_DWORD,
			(BYTE * const)&bRandomPic, sizeof(DWORD));

		::RegSetValueEx(hKey, _T("udwOption"), NULL, REG_DWORD,
			(BYTE * const)&udwOption, sizeof(DWORD));

		::RegSetValueEx(hKey, _T("bTimelySwitch"), NULL, REG_DWORD,
			(BYTE * const)&bTimelySwitch, sizeof(DWORD));

		::RegSetValueEx(hKey, _T("udwHours"), NULL, REG_DWORD,
			(BYTE * const)&udwHours, sizeof(DWORD));

		::RegSetValueEx(hKey, _T("udwMinutes"), NULL, REG_DWORD,
			(BYTE * const)&udwMinutes, sizeof(DWORD));

		::RegSetValueEx(hKey, _T("bLastTimeActiveDesktopEnabled"), NULL, REG_DWORD,
			(BYTE * const)&bLastTimeActiveDesktopEnabled, sizeof(DWORD));

		::RegSetValueEx(hKey, _T("bQuestionAboutActiveDesktop"), NULL, REG_DWORD,
			(BYTE * const)&bQuestionAboutActiveDesktop, sizeof(DWORD));

		::RegSetValueEx(hKey, _T("sCurrentFile"), NULL, REG_DWORD,
			(BYTE * const)sCurrentFile.c_str(), sizeof(DWORD));


		std::vector<std::basic_string<TCHAR> >::iterator it;
		DWORD udwCount = 0;
		for(it = vsSearchDirectories.begin(); it != vsSearchDirectories.end(); it++)
			udwCount += (DWORD)sizeof(TCHAR) * ((*it).length() + 1);
	    
		BYTE* pByte = new BYTE[udwCount + sizeof(TCHAR)];
        TCHAR* pChar = (TCHAR*)pByte;
		for(it = vsSearchDirectories.begin(); it != vsSearchDirectories.end(); it++)
		{
			_tcscpy(pChar, (*it).c_str());
//			memset(pChar + (*it).length() * sizeof(TCHAR), 0, sizeof(TCHAR));
			pChar += (*it).length() + 1;
		}
		memset(pChar, 0, sizeof(TCHAR));

        ::RegSetValueEx(hKey, _T("vsSearchDirectories"), NULL, REG_MULTI_SZ,
			(BYTE * const)pByte, udwCount + sizeof(TCHAR));

		::RegCloseKey(hKey);
	}


	// write SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run
	if(ERROR_SUCCESS == ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, LoadStringFromRes(IDS_SYS_REGISTER_RUN).c_str(), NULL, NULL, 
                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL))
	{
		if(bWithOSStartup)
		{
		
			std::basic_string<TCHAR>& cmdline = FormatCmdLine();
			::RegSetValueEx(hKey, LoadStringFromRes(IDS_SYS_APP_NAME).c_str(), NULL, REG_SZ,
				(BYTE * const)cmdline.c_str(), (DWORD)cmdline.length() * sizeof(TCHAR) + sizeof(TCHAR));
		}
		else
		{
			::RegDeleteValue(hKey, LoadStringFromRes(IDS_SYS_APP_NAME).c_str());
		}
		::RegCloseKey(hKey);
	}

}
//////////////////////////////////////////////////////////////////////////
// format command line with current exe path
std::basic_string<TCHAR> CDataManager::FormatCmdLine(void)
{
	TCHAR szFormatCmdLine[MAX_PATH + 7] = {0};
	DWORD len = GetModuleFileName(NULL, szFormatCmdLine, MAX_PATH);
	_tcscat_s(szFormatCmdLine, _T(" -auto"));
	return std::basic_string<TCHAR> (szFormatCmdLine);
}

