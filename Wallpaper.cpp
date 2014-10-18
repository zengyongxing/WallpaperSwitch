// SwitchWallpaper.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <time.h>
#include <wininet.h>
#include <shlobj.h>
#include "Wallpaper.h"
#include "wallpaper_util.h"
#include "DataManager.h"
#include "ErrorReport.h"
#include "resource.h"
#include "AboutDlg.h"


//////////////////////////////////////////////////////////////////////////
#define MSG_SHELL_TASKBARCREATED 0xC05C
//////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK ConfigDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void DealParameterChange(HWND hWnd);
void UpdateDisplyWithParameter(HWND hDlg);
void OnExitWindow(HWND hDlg);
//////////////////////////////////////////////////////////////////////////

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	// init com interface 
	CoInitialize(NULL);

	if (HasSameInstanceRunning()) return 0;

	BOOL bIsActiveDesktopEnabled = IsActiveDesktopEnabled();
	if(!bIsActiveDesktopEnabled && DataManager.bLastTimeActiveDesktopEnabled)
	{
		DataManager.bQuestionAboutActiveDesktop = TRUE;
	}

	if(DataManager.bQuestionAboutActiveDesktop && !bIsActiveDesktopEnabled)
	{
		int nRes = ::MessageBox(NULL, LoadStringFromRes(IDS_NO_ACTIVE_DESKTOP_USED).c_str(), 
						LoadStringFromRes(IDS_SYS_APP_NAME).c_str(),MB_YESNOCANCEL | MB_ICONQUESTION);

		if(IDYES == nRes)
		{
			EnableActiveDesktop(TRUE);
			DataManager.bQuestionAboutActiveDesktop = TRUE;
		}
		else if(IDNO == nRes)
		{
			// record this changs.
			DataManager.bQuestionAboutActiveDesktop = FALSE;
		}
		else // IDCANCEL 
		{
			// do nothing
			DataManager.bQuestionAboutActiveDesktop = TRUE;
		}
	}

	DataManager.bLastTimeActiveDesktopEnabled = IsActiveDesktopEnabled();


	// if need run intance once without UI
	if(0 == _tcscmp(LoadStringFromRes(IDS_SYS_AUTO_START_CMD_LINE).c_str(), lpCmdLine) 
                   && !DataManager.bTimelySwitch)
	{
		// 启动后立即退出。
		Switch();
		return 0;
	}
	else
	{
		HWND hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG_CONFIG), NULL, ConfigDlgProc);

		if(!hDlg)
		{
			DWORD re = GetLastError();
			int i = 0;
		}

		if(0 == _tcscmp(LoadStringFromRes(IDS_SYS_AUTO_START_CMD_LINE).c_str(), lpCmdLine))
		{
			ShowWindow(hDlg, SW_HIDE); 
		}
		else
		{
			ShowWindow(hDlg, SW_SHOW); 
		}

		// Main message loop //////////////////////////////////////////////////////////////
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) 
		{
			//if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
			//{
			if(!IsDialogMessage(hDlg, &msg)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			//}
		}

		return (int) msg.wParam;
	}
}

//////////////////////////////////////////////////////////////////////////
//
INT_PTR CALLBACK ConfigDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static NOTIFYICONDATA nid;
	static TCHAR szCurrentFile[MAX_PATH + 1] = {0};

	TCHAR szBuf[MAX_PATH + 1] = {0};				// temp buffer for display window title
	POINT ptCurrent;								// current mouse position 
    static UINT s_uTaskbarRestart = 0;              // Taskbar restart notify message

	switch (message) 
	{
	case WM_COMMAND:
		// Parse the menu selections:
		switch (LOWORD(wParam))
		{
        //////////////////////////////////////////////////////////////////////////
        // button on config dialog

		case IDOK:  // OK button pressed
			{
				DealParameterChange(hDlg);
				// no break here
			}
        case IDCANCEL: // ESC pressed or Close Button clicked
			{
				if(!DataManager.bTimelySwitch)
				{
					OnExitWindow(hDlg);
				}
				else
				{
					ShowWindow(hDlg, SW_HIDE);
				}
				return TRUE;
			}
        case IDC_BUTTON_ADD:
            {
                std::basic_string<TCHAR>& szPath = OpDirToSelectDir(hDlg, 
                                                LoadStringFromRes(IDS_SELECT_A_DIR).c_str());

				if(!szPath.empty())
				{
					HWND hList = GetDlgItem(hDlg, IDC_LIST_DIRECTORIES);
					if(LB_ERR != SendMessage( hList, LB_INSERTSTRING, 
									DataManager.vsSearchDirectories.size(), (WPARAM)szPath.c_str()))
					{
						DataManager.vsSearchDirectories.push_back(szPath);
					}
				}

				return TRUE;
            }
        case IDC_BUTTON_REMOVE:
            {
                HWND hList = GetDlgItem(hDlg, IDC_LIST_DIRECTORIES);
                LRESULT lRes = SendMessage( hList, LB_GETCURSEL, 0, 0);
                if(LB_ERR != lRes)
                {
                    LRESULT lRes2 = SendMessage(hList, LB_DELETESTRING, lRes, 0);
                    if(LB_ERR != lRes2)
                    {
                        for(DWORD i = (DWORD)lRes ; i < DataManager.vsSearchDirectories.size() - 1; i++)
                        {
                            DataManager.vsSearchDirectories[i] = DataManager.vsSearchDirectories[i+1];
                        }
                        DataManager.vsSearchDirectories.pop_back();
                    }
                }

				return TRUE;
            }
        case IDC_BUTTON_SWITCH:
            {
			    DealParameterChange(hDlg);
				Switch();
	            return TRUE;
            }

		case IDC_CHECK_TIMING:
			{
				BOOL bChecked = IsDlgButtonChecked(hDlg, IDC_CHECK_TIMING);
				HWND hEdit;
				hEdit = GetDlgItem(hDlg, IDC_EDIT_MINUTES);
				SendMessage(hEdit,  EM_SETREADONLY, (WPARAM) !bChecked, 0);
				hEdit = GetDlgItem(hDlg, IDC_EDIT_HOURS);
				SendMessage(hEdit,  EM_SETREADONLY, (WPARAM) !bChecked, 0);
				return TRUE;
			}
        //////////////////////////////////////////////////////////////////////////
        // popup menu item 
		case IDM_EXIT: // Exit selected on popup menu
			{
				OnExitWindow(hDlg);
				return TRUE;
			}
		case IDM_CONFIGURE: // Configure selected on popup menu
			{
				if (!IsWindowVisible(hDlg))
				{
					UpdateDisplyWithParameter(hDlg);
					ShowWindow(hDlg, SW_SHOW);
				}
				return TRUE;
			}
		case IDM_SWITCHNOW: // Switch selected on popup menu
			{
				Switch();
				return TRUE;
			}
		case IDM_DELETE_CUR: // Delete Current Wallpaper
			{
				Delete();
				return TRUE;
			}
		case IDM_ABOUT: // show about dialog
			{
				DialogBox(NULL, (LPCTSTR)IDD_DIALOG_ABOUT, NULL, (DLGPROC)DlgProcAboutDlg);
				return TRUE;
			}
		default:
			{
				// CHECK_ERROR_DWORD(1, _T("Is there some menu not processed?"), LOWORD(wParam));
				return FALSE;
			}
		}
		break; // end of WM_COMMAND

	case WM_INITDIALOG :
		{
            s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));

			UpdateDisplyWithParameter(hDlg);
			DealParameterChange(hDlg);

			// update application name
			::SetWindowText(hDlg, 
				(LoadStringFromRes(IDS_APP_NAME_LOCAL) + 
				_T(" - V") + LoadStringFromRes(IDS_APP_VERSION)).c_str());

			// update file location
			if(0 != GetModuleFileName(NULL, szBuf, MAX_PATH))
			{
				::SetDlgItemText(hDlg, IDC_EDIT_LOCATION, szBuf);
			}

			return TRUE;
		}

	case WM_TIMER:
		{
			Switch();
			return TRUE;
		}
	//user defined message
	case UM_NOTIFY_ICON:
		{
			switch (lParam)
			{
			case WM_RBUTTONDOWN:
				{
					HMENU hMainMenu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MENU_TRAY));
					HMENU hPopupMenu = GetSubMenu(hMainMenu, 0);
					if (hPopupMenu == NULL)
					{
						PostQuitMessage(0);
					}

					GetCursorPos(&ptCurrent);
					//ScreenToClient(hWnd, &ptCurrent);
					SetForegroundWindow(hDlg);
					TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, 
									ptCurrent.x, ptCurrent.y, 0, hDlg, NULL);

					PostMessage(hDlg, WM_NULL, 0, 0); 

					return TRUE;
				}
			case WM_LBUTTONDOWN:
				{
					Switch();
					return TRUE;
				}
			case WM_LBUTTONDBLCLK:
				{
					SetForegroundWindow(hDlg);
					if (!IsWindowVisible(hDlg))
					{
						ShowWindow(hDlg, SW_SHOW);
					}
					return TRUE;
				}

			default:
				return FALSE;
			}// end of switch(lParam)
		}
	//end of user defined message
	default:
        {
            if(message == s_uTaskbarRestart)
            {
                // explorer reload for some reason while app is still active.
                AddNotifyIconInSysTray(hDlg);
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
		
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// Update application behaviors with parameters
void DealParameterChange(HWND hDlg)
{
	// Get selection from UI
	if(hDlg)
	{
		TCHAR szBuf[2 + 1] = {0};
		DataManager.bRandomPic = IsDlgButtonChecked(hDlg, IDC_CHECK_RADMON);
		DataManager.bTimelySwitch = IsDlgButtonChecked(hDlg, IDC_CHECK_TIMING);
		DataManager.bWithOSStartup = IsDlgButtonChecked(hDlg, IDC_CHECK_OS_STARTUP);
        DataManager.udwOption = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO3)) ? WPSTYLE_STRETCH: 
                                    (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO2) ? WPSTYLE_TILE : 
                                        WPSTYLE_CENTER);
		

		GetDlgItemText(hDlg, IDC_EDIT_HOURS, szBuf, 3);
		DataManager.udwHours = _tstoi(szBuf);
		GetDlgItemText(hDlg, IDC_EDIT_MINUTES, szBuf, 3);
		DataManager.udwMinutes = _tstoi(szBuf);
        
        // should never Get DataManager.vsSearchDirectories from UI, which will be updated before UI.
	}
	else
	{
		// do nothing, at startup of application 
	}

	// change behaviors consist with parameters 
	if(DataManager.bTimelySwitch)
	{
		SetTimer(hDlg, ID_TIMER_SWITCHWALLPAPER, 
            (0 == DataManager.udwHours + DataManager.udwMinutes) ? 1000 :
			(DataManager.udwHours * 60 + DataManager.udwMinutes) * 1000 * 60, 
			NULL);

        AddNotifyIconInSysTray(hDlg);
	}

	// save parameters back to registry
	DataManager.SaveBackToRegistry();

}

//////////////////////////////////////////////////////////////////////////
// Update UI with parameters
void UpdateDisplyWithParameter(HWND hDlg)
{
	// IDC_CHECK_RADMON
	CheckDlgButton(hDlg, IDC_CHECK_RADMON, DataManager.bRandomPic ? BST_CHECKED : BST_UNCHECKED);

	// IDC_CHECK_OS_STARTUP
	CheckDlgButton(hDlg, IDC_CHECK_OS_STARTUP, DataManager.bWithOSStartup ? BST_CHECKED : BST_UNCHECKED);

	// IDC_CHECK_TIMING
	CheckDlgButton(hDlg, IDC_CHECK_TIMING, DataManager.bTimelySwitch ? BST_CHECKED : BST_UNCHECKED);

    // IDC_RADIO_XXX
    CheckRadioButton( hDlg, IDC_RADIO1, IDC_RADIO3, IDC_RADIO1 + DataManager.udwOption - WPSTYLE_CENTER);

	HWND hEdit;
	TCHAR szBuf[35] = {0};
	
	// IDC_EDIT_HOURS
	hEdit = GetDlgItem(hDlg, IDC_EDIT_HOURS);
	SendMessage(hEdit,  EM_SETREADONLY, (WPARAM) !DataManager.bTimelySwitch, 0);

	_ltot_s(DataManager.udwHours, szBuf, 35, 10);
	if(_tcslen(szBuf) > 0 || _tcslen(szBuf) <= 2)
	{
		SetWindowText(hEdit, szBuf);
	}

	// IDC_EDIT_MINUTES
	hEdit = GetDlgItem(hDlg, IDC_EDIT_MINUTES);
	SendMessage(hEdit,  EM_SETREADONLY, (WPARAM) !DataManager.bTimelySwitch, 0);

	_ltot_s(DataManager.udwMinutes, szBuf, 35, 10);
	if(_tcslen(szBuf) > 0 || _tcslen(szBuf) <= 2)
	{
		SetWindowText(hEdit, szBuf);
	}


    // IDC_LIST_DIRECTORIES
    HWND hList = GetDlgItem(hDlg, IDC_LIST_DIRECTORIES);
	for(DWORD i = (DWORD)SendMessage(hList, LB_GETCOUNT, 0, 0); i-- > 0 ; )
		SendMessage(hList, LB_DELETESTRING, i, 0); 

	std::vector<std::basic_string<TCHAR> >::iterator it;
    for(it = DataManager.vsSearchDirectories.begin(); it != DataManager.vsSearchDirectories.end(); it++)
    {
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM) (*it).c_str()); 
    }

	////////////////////////////////////////////////
	UpdateWindow(hDlg);

}
//////////////////////////////////////////////////////////////////////////
// encapusulate PostQuitMessage
void OnExitWindow(HWND hDlg)
{
    KillTimer(hDlg, ID_TIMER_SWITCHWALLPAPER);
    RemoveNotifyIconInSysTray();
	PostQuitMessage(0);
}

