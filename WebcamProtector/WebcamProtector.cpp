
// WebcamProtector.cpp : ���� ���α׷��� ���� Ŭ���� ������ �����մϴ�.
//

#include "stdafx.h"
#include "WebcamProtector.h"
#include "WebcamProtectorDlg.h"

#include <Lm.h>
#pragma comment(lib, "netapi32.lib")

#pragma comment(lib, "winmm")
#pragma comment(lib, "version")

BOOL GetWindowsVersion(DWORD& dwMajor, DWORD& dwMinor); // Check Windows Version
BOOL GetWindowsVersion(DWORD& dwMajor, DWORD& dwMinor, DWORD& dwServicePack);
BOOL IsCurrentProcess64bit(); // Check x64
BOOL IsCurrentProcessWow64();
BOOL Is64BitWindows();
BOOL GetProcessElevation(TOKEN_ELEVATION_TYPE *pElevationType, BOOL *pIsAdmin); // Check Admin

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWebcamProtectorApp

BEGIN_MESSAGE_MAP(CWebcamProtectorApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWebcamProtectorApp ����

CWebcamProtectorApp::CWebcamProtectorApp()
{
	// �ٽ� ���� ������ ����
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	// InitInstance�� ��� �߿��� �ʱ�ȭ �۾��� ��ġ�մϴ�.
}


// ������ CWebcamProtectorApp ��ü�Դϴ�.

CWebcamProtectorApp theApp;


// CWebcamProtectorApp �ʱ�ȭ

BOOL CWebcamProtectorApp::InitInstance()
{
	// ���� ���α׷� �Ŵ��佺Ʈ�� ComCtl32.dll ���� 6 �̻��� ����Ͽ� ���־� ��Ÿ����
	// ����ϵ��� �����ϴ� ���, Windows XP �󿡼� �ݵ�� InitCommonControlsEx()�� �ʿ��մϴ�.
	// InitCommonControlsEx()�� ������� ������ â�� ���� �� �����ϴ�.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ���� ���α׷����� ����� ��� ���� ��Ʈ�� Ŭ������ �����ϵ���
	// �� �׸��� �����Ͻʽÿ�.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ��ȭ ���ڿ� �� Ʈ�� �� �Ǵ�
	// �� ��� �� ��Ʈ���� ���ԵǾ� �ִ� ��� �� �����ڸ� ����ϴ�.
	CShellManager *pShellManager = new CShellManager;

	// ǥ�� �ʱ�ȭ
	// �̵� ����� ������� �ʰ� ���� ���� ������ ũ�⸦ ���̷���
	// �Ʒ����� �ʿ� ���� Ư�� �ʱ�ȭ
	// ��ƾ�� �����ؾ� �մϴ�.
	// �ش� ������ ����� ������Ʈ�� Ű�� �����Ͻʽÿ�.
	// TODO: �� ���ڿ��� ȸ�� �Ǵ� ������ �̸��� ����
	// ������ �������� �����ؾ� �մϴ�.
	SetRegistryKey(_T("���� ���� ���α׷� �����翡�� ������ ���� ���α׷�"));
	
	m_handle = ::CreateEvent(NULL, FALSE, FALSE, _T("wcamprt - �ߺ� ���� ����"));
	if (::GetLastError() == ERROR_ALREADY_EXISTS){
		m_execError = 1; // �ߺ� ����
		return false;
	}

	BOOL bResult64;
	DWORD dwMajor, dwMinor, dwServicePack;
	GetWindowsVersion(dwMajor, dwMinor, dwServicePack);
	if(dwMajor < 6){
		//m_execError = 2; // �������� �ʴ� �ü��
		//return false;
	}else{
		m_winVersion[0] = dwMajor;
		m_winVersion[1] = dwMinor;
		m_winVersion[2] = dwServicePack;
		bResult64 = Is64BitWindows();
		if(bResult64 == TRUE){
			m_execError = 3; // 64��Ʈ �ü��
			m_is64bit = true;
			return false;
		}else{
			m_is64bit = false;
		}
	}

	TOKEN_ELEVATION_TYPE t;
	BOOL bAdmin = FALSE;
	char szUser[0xFF] = {0};
	DWORD dwUser = _countof(szUser);
	GetUserName(szUser, &dwUser);

	if(GetProcessElevation(&t, &bAdmin)){
		if(t == TokenElevationTypeLimited){
			m_execError = 4; // ������ ����
			return false;
		}
	}

	CWebcamProtectorDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: ���⿡ [Ȯ��]�� Ŭ���Ͽ� ��ȭ ���ڰ� ������ �� ó����
		//  �ڵ带 ��ġ�մϴ�.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: ���⿡ [���]�� Ŭ���Ͽ� ��ȭ ���ڰ� ������ �� ó����
		//  �ڵ带 ��ġ�մϴ�.
	}

	// ������ ���� �� �����ڸ� �����մϴ�.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// ��ȭ ���ڰ� �������Ƿ� ���� ���α׷��� �޽��� ������ �������� �ʰ�  ���� ���α׷��� ���� �� �ֵ��� FALSE��
	// ��ȯ�մϴ�.
	return FALSE;
}



int CWebcamProtectorApp::ExitInstance()
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	CString strMsg;
	if (m_handle != nullptr) ::CloseHandle(m_handle); // �ߺ� ���� ���� ����
	
	if(m_execError != 0){
		switch(m_execError)
		{
			case 1:
				AfxMessageBox("���α׷��� �̹� ���� ���Դϴ�.\n(�ߺ� ���� �Ұ���)", MB_ICONSTOP);
				break;
			case 2:
				AfxMessageBox("�������� �ʴ� �ü�� �����Դϴ�.\n\n[�����Ǵ� �ü��]\n - Windows 7 x64\n - Windows 8/8.1 x64 �̻�", MB_ICONSTOP);
				break;
			case 3:
				AfxMessageBox("64bit �ü���� �������� �ʽ��ϴ�.\n(32bit �ü������ �������ּ���.)", MB_ICONSTOP);
				break;
			case 4:
				AfxMessageBox("������ �������� ������� �ʾҽ��ϴ�.\n������ �������� �ٽ� �������ּ���.", MB_ICONSTOP);
				break;
		}
	}

	return CWinApp::ExitInstance();
}

BOOL GetWindowsVersion(DWORD& dwMajor, DWORD& dwMinor)
{
    static DWORD dwMajorCache = 0, dwMinorCache = 0;
    if (0 != dwMajorCache)
    {
        dwMajor = dwMajorCache;
        dwMinor = dwMinorCache;
        return TRUE;
    }

    LPWKSTA_INFO_100 pBuf = NULL;
    if (NERR_Success != NetWkstaGetInfo(NULL, 100, (LPBYTE*)&pBuf))
        return FALSE;

    dwMajor = dwMajorCache = pBuf->wki100_ver_major;
    dwMinor = dwMinorCache = pBuf->wki100_ver_minor;
    NetApiBufferFree(pBuf);

    return TRUE;
}

BOOL GetWindowsVersion(DWORD& dwMajor, DWORD& dwMinor, DWORD& dwServicePack)
{
    if (!GetWindowsVersion(dwMajor, dwMinor))
        return FALSE;

    static DWORD dwServicePackCache = ULONG_MAX;
    if (ULONG_MAX != dwServicePackCache)
    {
        dwServicePack = dwServicePackCache;
        return TRUE;
    }

    const int nServicePackMax = 10;
    OSVERSIONINFOEX osvi;
    DWORDLONG dwlConditionMask = 0;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, VER_EQUAL);
	
    for (int i = 0; i < nServicePackMax; ++i)
    {
        osvi.wServicePackMajor = i;
        if (VerifyVersionInfo(&osvi, VER_SERVICEPACKMAJOR, dwlConditionMask))
        {
            dwServicePack = dwServicePackCache = i;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL IsCurrentProcess64bit() // ���� ���μ����� 32bit ���� 64bit ���� Ȯ��
{
	#if defined(_WIN64)
		return TRUE;
	#else
		return FALSE;
	#endif
}

BOOL IsCurrentProcessWow64() // ���� ���μ����� WOW64 ȯ�濡�� ���������� Ȯ��
{
    BOOL bIsWow64 = FALSE;
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;

    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
    if (!fnIsWow64Process)
        return FALSE;

    return fnIsWow64Process(GetCurrentProcess(), &bIsWow64) && bIsWow64;
}

BOOL Is64BitWindows() // ���� ��ġ�� �����찡 32bit ���� 64bit ���� Ȯ��
{
    if (IsCurrentProcess64bit())
        return TRUE;

    return IsCurrentProcessWow64();
}

BOOL GetProcessElevation(TOKEN_ELEVATION_TYPE *pElevationType, BOOL *pIsAdmin) // ������ ���� Ȯ��
{
    HANDLE hToken = NULL;
    BOOL bResult = FALSE;
    DWORD dwSize = 0;

    // ���� ���μ����� ��ū�� ��´�.
    if ( !OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) )
        return FALSE;

    // ���ѻ�� ���¿� ���� ������ ��´�.
    if ( GetTokenInformation(hToken, TokenElevationType, pElevationType, sizeof(TOKEN_ELEVATION_TYPE), &dwSize) )
    {
        BYTE adminSID[SECURITY_MAX_SID_SIZE];
        dwSize = sizeof(adminSID);
        
        // ������ �׷��� SID ���� �����Ѵ�.
        CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &adminSID, &dwSize);

        if ( *pElevationType == TokenElevationTypeLimited )
        {
            HANDLE hUnfilteredToken = NULL;
            
            // ����� ��ū�� �ڵ��� ��´�.
            GetTokenInformation(hToken, TokenLinkedToken, (void *)&hUnfilteredToken, sizeof(HANDLE), &dwSize);

            // ������ ��ū�� �������� SID�� �����ϰ� �ִ��� ���θ� Ȯ���Ѵ�.
            if ( CheckTokenMembership(hUnfilteredToken, &adminSID, pIsAdmin) )
                bResult = TRUE;
            
            CloseHandle(hUnfilteredToken);
        }
        else
        {
            *pIsAdmin = IsUserAnAdmin();
            bResult = TRUE;
        }
    }

    CloseHandle(hToken);
    return bResult;
}
