
// WebcamProtectorDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "WebcamProtector.h"
#include "WebcamProtectorDlg.h"
#include "afxdialogex.h"

#include <setupapi.h> // Device setup APIs
#pragma comment( lib, "setupapi.lib" )

#include <devguid.h>

#include <devpropdef.h>
#define INITGUID

#include <DEVPKEY.H>
#include <cfgmgr32.h>   // for MAX_DEVICE_ID_LEN, CM_Get_Parent and CM_Get_Device_ID

#include <winioctl.h>
#include <winsvc.h>

CWebcamProtectorDlg *g_pParent;

SC_HANDLE hScm, hSrv;
CWinThread*	pThreadWaitEvent = NULL;

// Device type
#define SIOCTL_TYPE 40000

// The IOCTL function codes from 0x800 to 0xFFF are for customer use.
#define IOCTL_START_PROTECTION CTL_CODE( SIOCTL_TYPE, 0x800, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)
#define IOCTL_STOP_PROTECTION CTL_CODE( SIOCTL_TYPE, 0x801, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)
#define IOCTL_UPDATE_DEVICE CTL_CODE( SIOCTL_TYPE, 0x802, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)
#define IOCTL_EVENT_ACCPID CTL_CODE( SIOCTL_TYPE, 0x803, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)
#define IOCTL_CLEAR_EXAPP CTL_CODE( SIOCTL_TYPE, 0x804, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)
#define IOCTL_ADD_EXAPP CTL_CODE( SIOCTL_TYPE, 0x805, METHOD_BUFFERED, FILE_READ_DATA|FILE_WRITE_DATA)

#define ARRAY_SIZE(arr)     (sizeof(arr)/sizeof(arr[0]))

#define DBT_DEVNODES_CHANGED			0x0007
#define DBT_DEVICEARRIVAL               0x8000  // system detected a new device
#define DBT_DEVICEQUERYREMOVE           0x8001  // wants to remove, may fail
#define DBT_DEVICEQUERYREMOVEFAILED     0x8002  // removal aborted
#define DBT_DEVICEREMOVEPENDING         0x8003  // about to remove, still avail.
#define DBT_DEVICEREMOVECOMPLETE        0x8004  // device is gone
#define DBT_DEVICETYPESPECIFIC          0x8005  // type specific event

typedef BOOL (WINAPI *FN_SetupDiGetDevicePropertyW)(
  __in       HDEVINFO DeviceInfoSet,
  __in       PSP_DEVINFO_DATA DeviceInfoData,
  __in       const DEVPROPKEY *PropertyKey,
  __out      DEVPROPTYPE *PropertyType,
  __out_opt  PBYTE PropertyBuffer,
  __in       DWORD PropertyBufferSize,
  __out_opt  PDWORD RequiredSize,
  __in       DWORD Flags
);

typedef struct _sInitPrtInfo{
	unsigned int mainPid;
	void* cbFunc;
} INIT_PRT_INFO;

typedef struct _appRule{
	int pid;
	int rule; // 0: block, 1: allow
} APP_RULE;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWebcamProtectorDlg ��ȭ ����




CWebcamProtectorDlg::CWebcamProtectorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWebcamProtectorDlg::IDD, pParent)
	, m_checkProtection(0)
	, m_labelStatus(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWebcamProtectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Device, ctr_listDevice);
	DDX_Check(pDX, IDC_CHECK_Protection, m_checkProtection);
	DDX_Control(pDX, IDC_CHECK_Protection, ctr_checkProtection);
	DDX_Text(pDX, IDC_STATIC_Status, m_labelStatus);
}

BEGIN_MESSAGE_MAP(CWebcamProtectorDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DEVICECHANGE()
	ON_BN_CLICKED(IDC_CHECK_Protection, &CWebcamProtectorDlg::OnBnClickedCheckProtection)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_Exception, &CWebcamProtectorDlg::OnNMClickSyslinkException)
	ON_REGISTERED_MESSAGE(WM_ALERT_ACCESSWEBCAM, AlertAccessWebcamPopup) // WM_ALERT_ACCESSWEBCAM
END_MESSAGE_MAP()


// CWebcamProtectorDlg �޽��� ó����

BOOL CWebcamProtectorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	DWORD pid;
	CString strMsg;

	g_pParent = this; // �θ� ��ü ����

	ctr_listDevice.DeleteAllItems(); // ��� ������ ����
	ctr_listDevice.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER); // List Control ��Ÿ�� ����
	
	ctr_listDevice.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 150, -1);
	ctr_listDevice.InsertColumn(1, _T("Device Name"), LVCFMT_LEFT, 150, -1);
	ctr_listDevice.InsertColumn(2, _T("Hardware IDs"), LVCFMT_LEFT, 250, -1);
	
	m_isRunningService = false;
	m_enableProtection = false;

	m_labelStatus.SetString("OFF");
	UpdateData(false);

	// ���� ���α׷� PID ���ϱ�
	GetWindowThreadProcessId(AfxGetMainWnd()->m_hWnd, &pid);
	m_myPID = pid;

	// ��ġ ��� ������Ʈ
	const GUID *pDevClass;
	pDevClass = &GUID_DEVCLASS_IMAGE; // &GUID_DEVCLASS_IMAGE
	UpdateDeviceList(pDevClass);

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CWebcamProtectorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CWebcamProtectorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CWebcamProtectorDlg::OnDeviceChange(UINT nEventType, DWORD dwData)
{
	const GUID *pDevClass;
	CString strMsg;

	switch(nEventType)
	{
		case DBT_DEVICEARRIVAL: // ��ġ ����
		case DBT_DEVICEREMOVECOMPLETE: // ��ġ ����
		case DBT_DEVNODES_CHANGED:
			pDevClass = &GUID_DEVCLASS_IMAGE; // &GUID_DEVCLASS_IMAGE
			UpdateDeviceList(pDevClass); // ��ġ ��� ������Ʈ
			break;
	}

	return TRUE;
}

void CWebcamProtectorDlg::UpdateDeviceList(const GUID *pDevClass)
{
	HDEVINFO hDev;
	SP_DEVINFO_DATA devInfo;
	DWORD devIndex=0;
	PCHAR *DeviceDesc;
	PCHAR *HardwareId;
	CString strMsg;
	
	CONFIGRET status;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD dwSize, dwPropertyRegDataType;
	WCHAR szDesc[1024], szHardwareIDs[4096];
	WCHAR szBuffer[4096];
	char szTemp[4096] = {0};
	DEVPROPTYPE ulPropertyType;

	LPTSTR pszToken, pszNextToken;
	TCHAR szDeviceInstanceID [MAX_DEVICE_ID_LEN];
	const static LPCTSTR arPrefix[3] = {TEXT("VID_"), TEXT("PID_"), TEXT("MI_")};
	TCHAR szVid[MAX_DEVICE_ID_LEN], szPid[MAX_DEVICE_ID_LEN], szMi[MAX_DEVICE_ID_LEN];
	
	int nStatus, probNum;

	FN_SetupDiGetDevicePropertyW fn_SetupDiGetDevicePropertyW = (FN_SetupDiGetDevicePropertyW)
        GetProcAddress (GetModuleHandle (TEXT("Setupapi.dll")), "SetupDiGetDevicePropertyW");

	// GUID_DEVCLASS_IMAGE�� ��ġ ����Ʈ ���ϱ�
	hDev= SetupDiGetClassDevs(pDevClass , NULL, NULL, DIGCF_PRESENT) ;

	devInfo.cbSize = sizeof(SP_DEVINFO_DATA) ;

	ctr_listDevice.DeleteAllItems(); // ��� ������ ����

	m_listDeviceName.clear();

	// �� ��ġ�� ������ ����
	for(devIndex=0; SetupDiEnumDeviceInfo(hDev,devIndex,&devInfo); devIndex++)
	{
		int listNum = ctr_listDevice.GetItemCount();
		
		devInfo.cbSize = sizeof (devInfo);
		if (!SetupDiEnumDeviceInfo(hDev, devIndex, &devInfo))
			break;
		
		// Device Instance ID ���ϱ�
		status = CM_Get_Device_ID(devInfo.DevInst, szDeviceInstanceID, MAX_PATH, 0);
		if (status != CR_SUCCESS)
			continue;

		if (SetupDiGetDeviceRegistryProperty (hDev, &devInfo, SPDRP_DEVICEDESC, &dwPropertyRegDataType, (BYTE*)szDesc, sizeof(szDesc), &dwSize)){

			strMsg.Empty();
			if (fn_SetupDiGetDevicePropertyW (hDev, &devInfo, &DEVPKEY_Device_FriendlyName,
				&ulPropertyType, (BYTE*)szBuffer, sizeof(szBuffer), &dwSize, 0))
			{				
					WideCharToMultiByte(CP_ACP, 0, szBuffer, 1024, (char*)szTemp, 1024, 0, FALSE);
					strMsg.Format("%s", szTemp);
					ctr_listDevice.InsertItem(listNum, strMsg);

			}
			else if(fn_SetupDiGetDevicePropertyW (hDev, &devInfo, &DEVPKEY_Device_BusReportedDeviceDesc,
				&ulPropertyType, (BYTE*)szBuffer, sizeof(szBuffer), &dwSize, 0))
			{

					WideCharToMultiByte(CP_ACP, 0, szBuffer, 1024, (char*)szTemp, 1024, 0, FALSE);
					strMsg.Format("%s", szTemp);
					ctr_listDevice.InsertItem(listNum, strMsg);

			}
			if(strMsg.IsEmpty() == true){
				strMsg.Format("%s", szDesc);
				ctr_listDevice.InsertItem(listNum, strMsg);
			}

			if(fn_SetupDiGetDevicePropertyW (hDev, &devInfo, &DEVPKEY_Device_PDOName,
				&ulPropertyType, (BYTE*)szBuffer, sizeof(szBuffer), &dwSize, 0))
			{
				WideCharToMultiByte(CP_ACP, 0, szBuffer, 1024, (char*)szTemp, 1024, 0, FALSE);
				strMsg.Format("%s", szTemp);
				ctr_listDevice.SetItemText(listNum, 1, strMsg);

				m_listDeviceName.push_back(strMsg);
			}
		}
		
		ctr_listDevice.SetItemText(listNum, 2, szDeviceInstanceID);

	}

	// kernel-level ���α׷��� ��ġ ����Ʈ ����
	SendMessageDriverDevice();

}

void CWebcamProtectorDlg::OnBnClickedCheckProtection()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(true);

	if(m_checkProtection == 1){
		if(m_enableProtection == true) return;
		StartWebcamProtection();
		
	}else{
		StopWebcamProtection();
	}

}

void CWebcamProtectorDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	if(m_isRunningService == true){
		if(m_enableProtection == true){
			StopWebcamProtection();
		}
		CloseDriverService();
	}
}

// ��ķ ���� �˸� �̺�Ʈ ������
static UINT AccessWebcamEvent(LPVOID lpParam)
{
	CWebcamProtectorDlg* pParent = (CWebcamProtectorDlg*) lpParam;
	char szCommand[100] = {0};
	char ReadBuffer[50] = {0};
	DWORD dwBytesRead = 0;
	APP_RULE tmpAppRule;

	// �̺�Ʈ �ʱ�ȭ
	ResetEvent(pParent->m_hAccessWebcam);

	do{
		// �̺�Ʈ ���
		WaitForSingleObject(pParent->m_hAccessWebcam, INFINITE);
		if(pParent->m_enableProtection == false)
			break;

		// ���� �õ� ���μ��� ID ��û
		DeviceIoControl(pParent->m_hDevice, IOCTL_EVENT_ACCPID, szCommand, strlen(szCommand)+1, ReadBuffer, sizeof(ReadBuffer), &dwBytesRead, NULL);
		memcpy(&tmpAppRule, ReadBuffer, sizeof(APP_RULE));

		CString strMsg;
		strMsg.Format("%d", tmpAppRule.pid);
		//AfxMessageBox(strMsg);
		
		// ��ķ ���� ���� Dialog ����
		PostMessage(g_pParent->m_hWnd, WM_ALERT_ACCESSWEBCAM, tmpAppRule.pid, NULL); // WM_ALERT_ACCESSWEBCAM

	}while(pParent->m_enableProtection);

	pThreadWaitEvent = NULL;
	return 0;
}


bool CWebcamProtectorDlg::StartWebcamProtection()
{
	int nResult;
	unsigned char szCommand[100] = {0};
	char ReadBuffer[50] = {0};
	DWORD dwBytesRead = 0;

	INIT_PRT_INFO sInitInfo;
	sInitInfo.mainPid = m_myPID;

	if(m_listDeviceName.size() == 0){
		AfxMessageBox("Not Found - Webcam");
		m_checkProtection = 0;
		UpdateData(false);
		return false;
	}
	m_labelStatus.SetString("Init...");
	UpdateData(false);
	
	// ����̹�(wcamprt)�� ���������� ������ ���� ����
	if(m_isRunningService == false){
		nResult = InitDriverService();
		if(nResult == false)
			return false;
	}

	// ���� ���� �˸� �̺�Ʈ ����
	m_hDevice = CreateFile("\\\\.\\wcamprt", GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_hDevice == INVALID_HANDLE_VALUE){
		AfxMessageBox("[Error]");
		m_checkProtection = 0;
		UpdateData(false);
		return false;
	}
	m_enableProtection = true;
	
	// ��ġ ��� ������Ʈ
	UpdateDeviceList(&GUID_DEVCLASS_IMAGE);

	m_hAccessWebcam = CreateEvent(NULL, FALSE, FALSE, "Global\\AccessWebcamEvent");
	pThreadWaitEvent = AfxBeginThread(AccessWebcamEvent, this, THREAD_PRIORITY_NORMAL, 0, 0);
	if(pThreadWaitEvent == NULL){
		AfxMessageBox("[Error] Thread is already running.", true);
		return false;
	}

	// ����̹��� ���� ���ø����̼� ����Ʈ ����
	SendMessageDriverExApp();
	
	// ��ȣ ���� �ɼ� ����
	memcpy(szCommand, &sInitInfo, sizeof(INIT_PRT_INFO));
	DeviceIoControl(m_hDevice, IOCTL_START_PROTECTION, szCommand, sizeof(INIT_PRT_INFO), ReadBuffer, sizeof(ReadBuffer), &dwBytesRead, NULL);
	
	m_labelStatus.SetString("ON");
	UpdateData(false);

	return 0;
}


bool CWebcamProtectorDlg::StopWebcamProtection()
{
	char szCommand[100] = {0};
	char ReadBuffer[50] = {0};
	DWORD dwBytesRead = 0;
	DWORD nExitCode = NULL;
	DWORD dw;
	
	m_enableProtection = false;

	// ���� �˸� �̺�Ʈ ����
	SetEvent(m_hAccessWebcam);
	dw = WaitForSingleObject(pThreadWaitEvent, 1000);
	if(dw == WAIT_TIMEOUT){
		GetExitCodeThread( pThreadWaitEvent->m_hThread, &nExitCode );
		TerminateThread( pThreadWaitEvent->m_hThread, nExitCode );
		pThreadWaitEvent = NULL;
	}
	
	// ��ȣ ���� �ɼ� ����
	DeviceIoControl(m_hDevice, IOCTL_STOP_PROTECTION, szCommand, strlen(szCommand)+1, ReadBuffer, sizeof(ReadBuffer), &dwBytesRead, NULL);
	CloseHandle(m_hDevice);

	m_labelStatus.SetString("OFF");
	UpdateData(false);

	return 0;
}

// wcamprt ����̹� ���� �ʱ�ȭ �� ����
bool CWebcamProtectorDlg::InitDriverService()
{
	BOOL result;

	HRSRC hrResource;
	HGLOBAL hData;
	unsigned char *aFilePtr;
	DWORD dwFileSize, dwNumWritten;;
	HANDLE hFileHandle;
	TCHAR lpDrvPath[MAX_PATH]={0,};

	CString str_pathSysFile;
	TCHAR str_pathSysDirectory[1024];

    SERVICE_STATUS status;
	int err;
	CString strErr;
	
	// �ý��� ���丮 ��� ���ϱ�(system32)
	GetSystemDirectory(str_pathSysDirectory, 1024);
	str_pathSysFile.Format("%s\\wcamprt.sys", str_pathSysDirectory);
	
	// ========== wcamprt.sys ���� ���� ========== //
	hrResource = FindResource(NULL, MAKEINTRESOURCE(IDR_DRIVER_WCP), "DRIVER");
	if(!hrResource)
		return false;
  
	hData = LoadResource(NULL, hrResource);
	if(!hData)
		return false;
     
	aFilePtr = (unsigned char *)LockResource(hData);
	if(!aFilePtr)
		return false;
     
	dwFileSize = SizeofResource(NULL, hrResource);
	wsprintf(lpDrvPath, "%s", str_pathSysFile);
		
	hFileHandle = CreateFile(lpDrvPath,	FILE_ALL_ACCESS, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if(INVALID_HANDLE_VALUE == hFileHandle)
		return false;
   
	while(dwFileSize--){
		WriteFile(hFileHandle, aFilePtr, 1, &dwNumWritten, NULL);
		aFilePtr++;
	}

	CloseHandle(hFileHandle);

	// ========== wcamprt.sys ���� �� ========== //

	// ���� ����
    hScm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	hSrv = OpenService(hScm, "wcamprt", SERVICE_ALL_ACCESS);
    if (hSrv == NULL)
    {
		hSrv = CreateService(   hScm,
								"wcamprt",
								"wcamprt",
								SERVICE_ALL_ACCESS,
								SERVICE_KERNEL_DRIVER,
								SERVICE_SYSTEM_START,
								SERVICE_ERROR_NORMAL,
								str_pathSysFile,
								NULL,
								NULL,
								NULL,
								NULL,
								NULL);
	}

    result = StartService(hSrv, 0, NULL);
	if(result == 0){
		err = GetLastError();
        if (err != ERROR_SERVICE_ALREADY_RUNNING)
        {
			// Failed to start service; clean-up:
			strErr.Format("[Error] ���� ���� ����\n(code : 0x%0.4X)", err);
			AfxMessageBox(strErr, false);
			ControlService(hSrv, SERVICE_CONTROL_STOP, &status);
			DeleteService(hSrv);
			CloseServiceHandle(hSrv);
			hSrv = NULL;
			SetLastError(err);
			return false;
        }
	}

	m_isRunningService = true;

	return true;
}

// wcamprt ����̹� ���� ����
bool CWebcamProtectorDlg::CloseDriverService()
{
	CString str_pathSysFile;
	TCHAR str_pathSysDirectory[1024];

    SERVICE_STATUS status;
	int err;
	CString strErr;

	ControlService(hSrv, SERVICE_CONTROL_STOP, &status);
	DeleteService(hSrv);
	CloseServiceHandle(hSrv);
	CloseServiceHandle(hScm);

	GetSystemDirectory(str_pathSysDirectory, 1024);
	str_pathSysFile.Format("%s\\wcamprt.sys", str_pathSysDirectory);
	
	if(PathFileExists(str_pathSysFile))
		DeleteFile(str_pathSysFile);

	return true;
}

// ��ġ ����Ʈ ���� �Լ�
bool CWebcamProtectorDlg::SendMessageDriverDevice()
{
	char szDeviceList[1024] = {0};
	char ReadBuffer[50] = {0};
	DWORD dwBytesRead = 0;
	
	if(m_enableProtection == false)
		return false;

	// ��ġ ����
	szDeviceList[0] = m_listDeviceName.size() + 0x30;

	// string ������ ����Ʈ �ۼ�
	list<CString>::iterator itor = m_listDeviceName.begin();
	while(itor != m_listDeviceName.end())
	{
		strcat(szDeviceList, (LPSTR)(LPCSTR)*itor);
		strcat(szDeviceList, " ");
		itor++;
	}

	// ����Ʈ ����
	DeviceIoControl(m_hDevice, IOCTL_UPDATE_DEVICE, szDeviceList, strlen(szDeviceList)+1, ReadBuffer, sizeof(ReadBuffer), &dwBytesRead, NULL);

	return true;
}

// ���� ���ø����̼� ���� �Լ�
bool CWebcamProtectorDlg::SendMessageDriverExApp()
{
	char szExAppList[1024] = {0};
	char ReadBuffer[50] = {0};
	DWORD dwBytesRead = 0;
	
	if(m_enableProtection == false)
		return false;

	// Reset Exception App
	DeviceIoControl(m_hDevice, IOCTL_CLEAR_EXAPP, szExAppList, strlen(szExAppList)+1, ReadBuffer, sizeof(ReadBuffer), &dwBytesRead, NULL);
	
	// Add Exception App
	list<EXCEPTION_APP>::iterator itor = m_listExceptionApp.begin();
	while(itor != m_listExceptionApp.end())
	{
		strcpy_s(szExAppList, itor->path);
		DeviceIoControl(m_hDevice, IOCTL_ADD_EXAPP, szExAppList, strlen(szExAppList)+1, ReadBuffer, sizeof(ReadBuffer), &dwBytesRead, NULL);
		itor++;
	}

	return true;
}

// ��ȣ ���� ���� Dialog
void CWebcamProtectorDlg::OnNMClickSyslinkException(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	if(m_pExceptionSettingDlg.GetSafeHwnd() == NULL){
		m_pExceptionSettingDlg.Create(IDD_EXCEPTIONSETTINGDLG);
		m_pExceptionSettingDlg.CenterWindow(CWnd::FromHandle(this->m_hWnd));
	}
	m_pExceptionSettingDlg.ShowWindow(SW_SHOW);

	*pResult = 0;
}

// ���� ���� �˸� Dialog
LRESULT CWebcamProtectorDlg::AlertAccessWebcamPopup(WPARAM wParam, LPARAM lParam)
{
	m_hPidAccessApp = (HANDLE)wParam;

	if(g_pParent->m_pAlertAccessWebcamDlg.GetSafeHwnd() == NULL){
		m_pAlertAccessWebcamDlg.Create(IDD_ALERTACCESSWEBCAMDLG);
		//m_pAlertAccessWebcamDlg.CenterWindow(CWnd::FromHandle(this->m_hWnd));
	}
	m_pAlertAccessWebcamDlg.ShowWindow(SW_SHOW);

	return 0;
}