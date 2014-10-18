#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <Windows.h>
#include <vector>
#include <string>
#include <tchar.h>
//////////////////////////////////////////////////////////////////////////
// DataManager is a singletone. You may access any value as following
// CDatamanager.udwSomeData
// and you may call SaveBackToRegistry() explicitly to save data 
//////////////////////////////////////////////////////////////////////////

class CDataManager{
// Local data  declaration ///////////////////////////////////////////////
public: // Declare your data here
    BOOL	bRandomPic;
	DWORD	udwOption; 
    BOOL	bWithOSStartup;
    BOOL	bTimelySwitch;
    DWORD   udwHours;
    DWORD   udwMinutes;

	std::basic_string<TCHAR> sCurrentFile;
    std::vector<std::basic_string<TCHAR> > vsSearchDirectories;

	BOOL bLastTimeActiveDesktopEnabled;
	BOOL bQuestionAboutActiveDesktop;

//  constructor and destructor ///////////////////////////////////////////
public:
    CDataManager();
    ~CDataManager();
    void SaveBackToRegistry(void);

// data operation ////////////////////////////////////////////////////////
private:
    void InitWithDefaultValue(void);
    void InitWithRegistry(void);
	std::basic_string<TCHAR> FormatCmdLine(void);
};

extern CDataManager DataManager;

#endif  // DATA_MANAGER_H