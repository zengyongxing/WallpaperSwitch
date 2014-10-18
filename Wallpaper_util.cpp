
#include "Wallpaper_util.h"
#include "ErrorReport.h"
#include "DataManager.h"
#include "resource.h"
#include <wininet.h>
#include <Shlobj.h>
#include <time.h>
TCHAR g_szMainWindowClassName[MAX_PATH + 1] = {0};
//////////////////////////////////////////////////////////////////////////
// Check if the specified Mutex have been created
BOOL IsMutexCreated(LPCTSTR szUniqueMutexName) // Mutex name
{
	HANDLE hInstanceEvent = NULL;
	hInstanceEvent = CreateMutex(NULL, FALSE, szUniqueMutexName);
	// we not take the ownership of this mutex, so the mutex will be release 
	// automaticly without release() called
	if (hInstanceEvent != NULL)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//////////////////////////////////////////////////////////////////////////
// Check if there are any instance already exist.
BOOL HasSameInstanceRunning(void)
{
	return IsMutexCreated(LoadStringFromRes(IDS_SYS_INSTANCE_MUTEX_NAME).c_str());
}
//////////////////////////////////////////////////////////////////////////
// Check if is a supported file
// 
BOOL IsSupportedPicFile(LPCTSTR lpFileName,			// point to file name
						BOOL bActiveDesktopEnabled) // Is Active Desktop enabled
{
	if(lpFileName)
	{
		LPCTSTR pExt = _tcsrchr(lpFileName, _T('.'));
		if(pExt)
		{
			pExt++;
			if(bActiveDesktopEnabled)
			{
				if((0 == _tcsicmp(_T("bmp"), pExt))
					|| (0 == _tcsicmp(_T("gif"), pExt))
					|| (0 == _tcsicmp(_T("jpg"), pExt))
					|| (0 == _tcsicmp(_T("jpeg"), pExt))
					|| (0 == _tcsicmp(_T("dib"), pExt))
					|| (0 == _tcsicmp(_T("png"), pExt)))
				{
					return TRUE;
				}

			}
			else
			{
				// only bmp files supported while Active Desktop disabled
				if(0 == _tcsicmp(_T("bmp"), pExt))
				{
					return TRUE;
				}
			}

		}
	}
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////
// search files in directory
void TravelDir(std::vector<std::basic_string<TCHAR> >& vsFiles, 
			   const std::basic_string<TCHAR> sDir)
{
	TCHAR* pExtension = NULL;
    WIN32_FIND_DATA data;
    std::basic_string<TCHAR> str = sDir + _T("\\*.*");
    std::basic_string<TCHAR> sTDir = sDir;

    HANDLE hFind = FindFirstFile(str.c_str(), &data);
    if (INVALID_HANDLE_VALUE != hFind) 
    {
        if(FILE_ATTRIBUTE_DIRECTORY == data.dwFileAttributes )
        {
            if((0 != _tcscmp(_T("."), data.cFileName) )
                && (0 != _tcscmp(_T(".."), data.cFileName) ))
            {
				sTDir = sDir + _T("\\") + data.cFileName;
                TravelDir(vsFiles, sTDir);
            }
        }
        else
        {
			if(IsSupportedPicFile(data.cFileName, IsActiveDesktopEnabled()))
			{
				vsFiles.push_back(sDir + _T("\\") + data.cFileName);
			}
       }

        while(FindNextFile(hFind, &data))
        {
            if(FILE_ATTRIBUTE_DIRECTORY == data.dwFileAttributes)
            {
                if( (0 != _tcscmp(_T("."), data.cFileName) )
                    && (0 != _tcscmp(_T(".."), data.cFileName) ))
                {
					sTDir = sDir + _T("\\") + data.cFileName;
                    TravelDir(vsFiles, sTDir);
                }
            }
            else
            {
				if(IsSupportedPicFile(data.cFileName, IsActiveDesktopEnabled()))
				{
					vsFiles.push_back(sDir + _T("\\") + data.cFileName);
				}
			}
        }// while
    }// if 

}

//////////////////////////////////////////////////////////////////////////
// Get next file from specified directory vector
std::basic_string<TCHAR> GetNextFile(const std::vector<std::basic_string<TCHAR> >& vsDirectories,
									 LPCTSTR pCurrentFile, 
									 BOOL bRandmon)
{
	static BOOL bFirstIn = TRUE;
	if(bFirstIn)
	{
		srand( (unsigned)time( NULL ) );
		bFirstIn = FALSE;
	}

	std::basic_string<TCHAR> returnValue;

	std::vector<std::basic_string<TCHAR> > vsFileList;
	for(DWORD i = 0; i < vsDirectories.size(); i++)
			TravelDir(vsFileList, vsDirectories[i]);

	if(0 == vsFileList.size()) 
	{
		// do nothing
	}
	else if (1 == vsFileList.size())
	{
		returnValue = vsFileList[0];
	}
	else
	{
		if(TRUE == bRandmon)
		{
			size_t i = 0;
			do{
				i = rand() * vsFileList.size() / RAND_MAX;
			}while(0 == _tcscmp(vsFileList[i].c_str(), pCurrentFile));

			returnValue = vsFileList[i];
		}
		else
		{
			if(NULL == pCurrentFile)
			{
				returnValue = vsFileList[0]; 
			}
			else
			{
				for(DWORD j = 0; j < vsFileList.size(); j++)
				{
					if(0 == _tcscmp(vsFileList[j].c_str(), pCurrentFile))
					{
						returnValue = vsFileList[(j + 1) % vsFileList.size()];
					}
				}
			}

			
		}
	}

	return returnValue;
}

//////////////////////////////////////////////////////////////////////////
//
BOOL IsActiveDesktopEnabled(void)
{
	/* check is the active desktop used for current desktop? */
	HRESULT hr;
	IActiveDesktop *pActiveDesktop;

	//Create an instance of the Active Desktop
	hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
		IID_IActiveDesktop, (void**)&pActiveDesktop);

	if (FAILED(hr))
	{
		return FALSE;
	}

	COMPONENTSOPT opt;
	opt.dwSize = sizeof(opt);
	hr = pActiveDesktop->GetDesktopItemOptions(&opt, 0);

	// Call the Release method
	pActiveDesktop->Release();
	if((S_OK  == hr) && opt.fActiveDesktop)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////////
//
BOOL EnableActiveDesktop(BOOL bEnable)
{
	/* check is the active desktop used for current desktop? */
	HRESULT hr;
	IActiveDesktop *pActiveDesktop;

	//Create an instance of the Active Desktop
	hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
		IID_IActiveDesktop, (void**)&pActiveDesktop);

	if (FAILED(hr))
	{
		return FALSE;
	}

	COMPONENTSOPT opt;
	opt.dwSize = sizeof(opt);
	opt.fActiveDesktop = bEnable;
	hr = pActiveDesktop->SetDesktopItemOptions(&opt, 0);
	pActiveDesktop->ApplyChanges(AD_APPLY_ALL);

	// Call the Release method
	pActiveDesktop->Release();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// switch wallpaper to specified file and style
// no chech file parameter, since NULL means remove current wallpaper
BOOL SwitchWallPaper(LPCTSTR file, DWORD style)
{
	BOOL b;
	DWORD dw;
	if (!IsActiveDesktopEnabled())
	{
		//////////////////////////////////////////////////////
		//  there are some error here!!!!!!!!!!!!!!			//
		// no work proper in orginal mode               	//
		//////////////////////////////////////////////////////

		b = SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (PVOID)file, SPIF_SENDCHANGE);
		if(!b)
		{
			dw = GetLastError(); 
			int i = 0;
		}
	}
	else
	{
		// Use active desktop to set the desktop wallpaper
		/* check is the active desktop used for current desktop? */
		HRESULT hr;
		IActiveDesktop *pActiveDesktop;
			
		// Create an instance of the Active Desktop
		hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
								IID_IActiveDesktop, (void**)&pActiveDesktop);
			
		if (FAILED(hr))
		{
			//use the default method to set the wallpaper
			SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (PVOID)file, SPIF_SENDCHANGE);
			return 0;
		}
		//WALLPAPEROPT WallpaperOption;
		//WallpaperOption.dwSize = sizeof(WALLPAPEROPT);
		//pActiveDesktop->GetWallpaperOptions(&WallpaperOption, 0);
#ifndef UNICODE		

		///////////////////////////////////////////////////////
		/// there are some error here!!!!!!!!!!!!!!			///
		// can not convert ascii chinese string to unicode	///
		///////////////////////////////////////////////////////
		LPWSTR pwsWallpaperFile = (LPWSTR)malloc(sizeof(WCHAR) * (_tcslen(file) + 1));
		ZeroMemory(pwsWallpaperFile, sizeof(WCHAR) * (_tcslen(file) + 1));
		mbstowcs(pwsWallpaperFile, file, _tcslen(file));
		hr = pActiveDesktop->SetWallpaper(pwsWallpaperFile, 0);
		free(pwsWallpaperFile);
		pwsWallpaperFile = NULL;
		
#else // UNICODE defined
		hr = pActiveDesktop->SetWallpaper(file,0);
#endif // UNICODE

		WALLPAPEROPT opt;
		opt.dwSize = sizeof(opt);
		opt.dwStyle = style;
		hr = pActiveDesktop->SetWallpaperOptions(&opt,0);


		hr = pActiveDesktop->ApplyChanges(AD_APPLY_ALL);
		pActiveDesktop->Release();
	}

    return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
std::basic_string<TCHAR> LoadStringFromRes(UINT uID)
{
	TCHAR sLoadStringFromResBuf[MAX_PATH + 1];
	sLoadStringFromResBuf[0] = NULL;
	LoadString(NULL, uID, sLoadStringFromResBuf, MAX_PATH);

	return std::basic_string<TCHAR> (sLoadStringFromResBuf);
}


static NOTIFYICONDATA nid; 
//////////////////////////////////////////////////////////////////////////
// add Notify icon
void AddNotifyIconInSysTray(HWND hWnd)
{
    ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_WALLPAPER));
    nid.hWnd = hWnd;
    _tcsncpy_s(nid.szTip, 64, LoadStringFromRes(IDS_SYS_APP_NAME).c_str(), _TRUNCATE);
    nid.uCallbackMessage = UM_NOTIFY_ICON;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

    Shell_NotifyIcon(NIM_ADD, &nid);
}
//////////////////////////////////////////////////////////////////////////
// remove Notify icon
void RemoveNotifyIconInSysTray(void)
{
	Shell_NotifyIcon(NIM_DELETE, &nid);
}
//////////////////////////////////////////////////////////////////////////
//
std::basic_string<TCHAR> OpDirToSelectDir(HWND hDlg, LPCTSTR tip)
{
    TCHAR aPath[MAX_PATH];

    BROWSEINFO bi;
    bi.hwndOwner = hDlg;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = aPath;
    bi.lpszTitle = tip;
    bi.ulFlags = BIF_RETURNONLYFSDIRS  | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON | BIF_RETURNFSANCESTORS ;
    bi.lpfn = NULL;
    bi.lParam = 0;
    bi.iImage = 0;

    LPITEMIDLIST  lpIDList = SHBrowseForFolder(&bi);

    memset(aPath, 0 , MAX_PATH * sizeof(TCHAR));

    if(lpIDList)
    {
        SHGetPathFromIDList(lpIDList, aPath);

        LPMALLOC pMalloc = NULL;
        if(NO_ERROR == SHGetMalloc(&pMalloc))
        {
            pMalloc->Free(lpIDList);
            lpIDList = NULL;
            pMalloc->Release();
        }
    }

    return std::basic_string<TCHAR> (aPath);
}


///////////////////////////////////////////////////////////////////////////////
///

void Switch(void)
{
	DataManager.sCurrentFile =  GetNextFile(DataManager.vsSearchDirectories, 
						DataManager.sCurrentFile.c_str(), DataManager.bRandomPic);
	SwitchWallPaper(DataManager.sCurrentFile.c_str(), DataManager.udwOption);
}

void Delete(void)
{
	std::basic_string<TCHAR> file2Delete =  DataManager.sCurrentFile;

	Switch();
	
	DeleteFile(file2Delete.c_str());
}
