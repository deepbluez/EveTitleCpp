// EVETitle.cpp : 定义应用程序的入口点。
//

#include "atlstr.h"
#include <map>

#include "framework.h"
#include "EVETitle.h"
#include "SystemTraySDK.h"

#define MAX_LOADSTRING 100
#define WM_TRAY_MESSAGE (WM_APP + 101)
#define ICON_ID        IDC_EVETITLE

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

struct Config
{
    CString titleCheck;
    CString windowClass;
    bool autoChange;
    int interval;
    CString prefix;
    CString suffix;
    std::map<CString, CString> nameMap;
};

// 全局配置
Config config;

// 托盘图标类
CSystemTray g_trayIcon;

void loadConfig(HINSTANCE hInstance);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    hInst = hInstance;

    loadConfig(hInstance);

    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), NULL, About);
    return 0;
}

void UpdateTitle(HWND gameWnd)
{
    CString origTitle;
    ::GetWindowText(gameWnd, origTitle.GetBuffer(MAX_PATH), MAX_PATH);
    auto err = ::GetLastError();
    origTitle.ReleaseBuffer();
    if (err != ERROR_SUCCESS)
        return;

    if (origTitle.Find(config.titleCheck) == 0)
    {
        CString charName = origTitle.Mid(config.titleCheck.GetLength());
        if (config.nameMap.count(charName) != 0)
            charName = config.nameMap[charName];
    	
        CString newTitle = config.prefix + charName + config.suffix;
        //::SetWindowText(gameWnd, newTitle);
        ::OutputDebugString(newTitle + _T("\n"));
    }
}

void UpdateAllTitle()
{
    HWND found = NULL;

	do
	{
        found = ::FindWindowEx(NULL, found, TEXT("triuiScreen"), NULL);
        if (found != nullptr)
            UpdateTitle(found);
    } while (found != NULL);
}


// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        if(config.autoChange)
        {
            HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_EVETITLE));
            g_trayIcon.Create(hInst, nullptr, WM_TRAY_MESSAGE, _T("EVETile\nPowered by 故乡的星"), hIcon, ICON_ID);
            g_trayIcon.SetTargetWnd(hDlg);

            SetDlgItemText(hDlg, IDC_STATIC_AUTOMODE, _T("自动导航启动"));
            SetDlgItemText(hDlg, IDOK, _T("隐藏到后台"));
        	
            ::SetTimer(hDlg, 2, config.interval * 1000, NULL);
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
		    switch (LOWORD(wParam))
    		{
            case IDOK:
                UpdateAllTitle();
                if (config.autoChange)
                    ShowWindow(hDlg, SW_HIDE);
                else
                    EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            case IDCANCEL:
                g_trayIcon.RemoveIcon();
                EndDialog(hDlg, LOWORD(wParam));
                break;
            case ID_SHOW_WINDOW:
                ::ShowWindow(hDlg, SW_SHOW);
                break;
            case IDM_EXIT:
                ::PostMessage(hDlg, WM_CLOSE, 0, 0);
                break;
            }
            break;
    case WM_CLOSE:
        g_trayIcon.RemoveIcon();
        EndDialog(hDlg, 0);
        break;

    case WM_TIMER:
        if (wParam == 2)
            UpdateAllTitle();
        break;
    }
    return (INT_PTR)FALSE;
}

void trimString(CString& str)
{
    if (str[0] == _T('\"') && str[str.GetLength() - 1] == _T('\"') && str.GetLength() >= 2)
        str = str.Mid(1, str.GetLength() - 2);
	
    str.Replace(_T('_'), _T(' '));
}

void loadConfig(HINSTANCE hInstance)
{
#define GENERAL_GROUP _T("general")
#define MAP_GROUP _T("map")
	CString exePath, iniPath;
    GetModuleFileName(hInstance, exePath.GetBuffer(MAX_PATH), MAX_PATH);
    exePath.ReleaseBuffer();

    int pos = exePath.ReverseFind(_T('\\'));
	if(pos == -1)
	{
        MessageBox(NULL, _T("无法加载配置文件"), _T("错误"), MB_OK | MB_ICONERROR);
        ExitProcess(1);
	}

    iniPath = exePath.Left(pos) + _T("\\EVETitle.ini");
	
    GetPrivateProfileString(GENERAL_GROUP, _T("title_check"), _T("星战前夜：晨曦 - "), config.titleCheck.GetBuffer(MAX_PATH), MAX_PATH, iniPath);
    config.titleCheck.ReleaseBuffer();
    trimString(config.titleCheck);

	//GetPrivateProfileString(GENERAL_GROUP, _T("class"), _T("triuiScreen"), config.windowClass.GetBuffer(MAX_PATH), MAX_PATH, iniPath);
    //config.windowClass.ReleaseBuffer();
    config.windowClass = _T("triuiScreen");

    config.autoChange = !!GetPrivateProfileInt(GENERAL_GROUP, _T("auto"), 0, iniPath);

	config.interval = GetPrivateProfileInt(GENERAL_GROUP, _T("interval"), 3, iniPath);
    config.interval = config.interval < 3 ? 3 : (config.interval > 99 ? 99 : config.interval);

    GetPrivateProfileString(GENERAL_GROUP, _T("prefix"), _T(""), config.prefix.GetBuffer(MAX_PATH), MAX_PATH, iniPath);
    config.prefix.ReleaseBuffer();
    trimString(config.prefix);

    GetPrivateProfileString(GENERAL_GROUP, _T("suffix"), _T(" - EVE"), config.suffix.GetBuffer(MAX_PATH), MAX_PATH, iniPath);
    config.suffix.ReleaseBuffer();
    trimString(config.suffix);

    const DWORD KEY_BUF_SIZE = 32767;
    CString mapKeys;
    LPTSTR pKeyBuffer = mapKeys.GetBuffer(KEY_BUF_SIZE);
    DWORD keySize = GetPrivateProfileSection(MAP_GROUP, pKeyBuffer, KEY_BUF_SIZE, iniPath);
    if (keySize > KEY_BUF_SIZE)
        return;

	for(LPCTSTR pkey = pKeyBuffer; *pkey != _T('\0'); pkey += _tcslen(pkey) + 1)
	{
        CString mapItem(pkey);
        int pos = mapItem.Find(_T('='));
		if(pos == -1)
            continue;

        CString key = mapItem.Left(pos);
        trimString(key);
        CString nickname = mapItem.Mid(pos + 1);
        trimString(nickname);
        if (!key.IsEmpty() && !nickname.IsEmpty())
            config.nameMap[key] = nickname;
	}
}
