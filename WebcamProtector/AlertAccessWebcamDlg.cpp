// AlertAccessWebcamDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "WebcamProtector.h"
#include "AlertAccessWebcamDlg.h"
#include "afxdialogex.h"
#include "WebcamProtectorDlg.h"

extern CWebcamProtectorDlg *g_pParent;


// CAlertAccessWebcamDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CAlertAccessWebcamDlg, CDialog)

CAlertAccessWebcamDlg::CAlertAccessWebcamDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAlertAccessWebcamDlg::IDD, pParent)
{

}

CAlertAccessWebcamDlg::~CAlertAccessWebcamDlg()
{
}

void CAlertAccessWebcamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SYSLINK_ProcPath, ctr_linkProcPath);
}


BEGIN_MESSAGE_MAP(CAlertAccessWebcamDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_Allow, &CAlertAccessWebcamDlg::OnBnClickedButtonAllow)
	ON_BN_CLICKED(IDC_BUTTON_Block, &CAlertAccessWebcamDlg::OnBnClickedButtonBlock)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CAlertAccessWebcamDlg �޽��� ó�����Դϴ�.



BOOL CAlertAccessWebcamDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	CString strTemp;
	HANDLE handle;
	char szPath[1024] = {0};
	DWORD dwLen = 0;

	CString appPath; // PATH
	
	RECT rectWin;
	GetWindowRect(&rectWin);
	
	int m_Desktowidth = GetSystemMetrics(SM_CXSCREEN);
	int m_DesktopHeight = GetSystemMetrics(SM_CYSCREEN);
	
	SetWindowPos(NULL, m_Desktowidth - (rectWin.right - rectWin.left) - 10, m_DesktopHeight - (rectWin.bottom - rectWin.top) - 40, 0, 0, SWP_NOSIZE);

	handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, (DWORD)g_pParent->m_hPidAccessApp);	
	if(GetModuleFileNameEx(handle, NULL, szPath, 1024)){
		m_strTargetAppPath = szPath;
		strTemp.Format("[%d] %s", g_pParent->m_hPidAccessApp, PathFindFileName(szPath));
		GetDlgItem(IDC_STATIC_ProcName)->SetWindowTextA(strTemp);

		appPath = szPath;
		int nPos = appPath.ReverseFind('\\'); // �������� ��ο��� ���ϸ� ����
		if(nPos > 0)
			appPath = appPath.Left(nPos);

		strTemp.Format("(<a>%s</a>)", appPath);
		ctr_linkProcPath.SetWindowTextA(strTemp);
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}


void CAlertAccessWebcamDlg::OnClose()
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	DestroyWindow();

	CDialog::OnClose();
}


void CAlertAccessWebcamDlg::OnBnClickedButtonAllow()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	
	EXCEPTION_APP sExceptionApp;
	list<EXCEPTION_APP>::iterator itor = g_pParent->m_listExceptionApp.begin();
	bool result = true;
	CString strFilePath;
	CString strFileName;
	
	strFilePath = m_strTargetAppPath;
	strFileName = PathFindFileName(strFilePath);

	// �ߺ� �˻�
	while(itor != g_pParent->m_listExceptionApp.end())
	{
		if(strFileName.Compare(itor->name) == 0){
			result = false;
			break;
		}
		itor++;
	}
	if(result == true){
		sExceptionApp.name.SetString(strFileName);
		sExceptionApp.path.SetString(strFilePath);
		g_pParent->m_listExceptionApp.push_back(sExceptionApp);
		g_pParent->SendMessageDriverExApp();
	}
	DestroyWindow();
}


void CAlertAccessWebcamDlg::OnBnClickedButtonBlock()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	DestroyWindow();
}