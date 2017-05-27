// ExceptionSettingDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "WebcamProtector.h"
#include "ExceptionSettingDlg.h"
#include "afxdialogex.h"
#include "WebcamProtectorDlg.h"

extern CWebcamProtectorDlg *g_pParent;


// CExceptionSettingDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CExceptionSettingDlg, CDialog)

CExceptionSettingDlg::CExceptionSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExceptionSettingDlg::IDD, pParent)
{

}

CExceptionSettingDlg::~CExceptionSettingDlg()
{
}

void CExceptionSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Exception, ctr_listException);
}


BEGIN_MESSAGE_MAP(CExceptionSettingDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_Add, &CExceptionSettingDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_Del, &CExceptionSettingDlg::OnBnClickedButtonDel)
END_MESSAGE_MAP()


// CExceptionSettingDlg �޽��� ó�����Դϴ�.


BOOL CExceptionSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	ctr_listException.DeleteAllItems(); // ��� ������ ����
	ctr_listException.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER); // List Control ��Ÿ�� ����
	
	ctr_listException.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 120, -1);
	ctr_listException.InsertColumn(1, _T("Path"), LVCFMT_LEFT, 250, -1);

	UpdateExceptionList();

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}


void CExceptionSettingDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
}


void CExceptionSettingDlg::OnClose()
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	DestroyWindow();

	CDialog::OnClose();
}

void CExceptionSettingDlg::UpdateExceptionList()
{
	int listNum;
	list<EXCEPTION_APP>::iterator itor = g_pParent->m_listExceptionApp.begin();
	
	ctr_listException.DeleteAllItems(); // ��� ������ ����
	while(itor != g_pParent->m_listExceptionApp.end())
	{ 
		listNum = ctr_listException.GetItemCount();

		ctr_listException.InsertItem(listNum, itor->name);
		ctr_listException.SetItemText(listNum, 1, itor->path);
		itor++;
	}
	g_pParent->SendMessageDriverExApp();
}

void CExceptionSettingDlg::OnBnClickedButtonAdd()
{
	CString strFilePath;
	CString szFilter, szDefExt;
	EXCEPTION_APP sExceptionApp;

	szFilter = "���� ����(*.exe)|*.exe|";
	szDefExt = "exe";

	CFileDialog fileDlg(TRUE, szDefExt, NULL, OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, szFilter, this);
	if(fileDlg.DoModal() == IDOK)
	{ 
		int listNum;
		bool result = true;
		CString strFileName;
		list<EXCEPTION_APP>::iterator itor = g_pParent->m_listExceptionApp.begin();
		strFilePath = fileDlg.GetPathName();
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
		if(result == false){
			return;
		}else{
			sExceptionApp.name.SetString(strFileName);
			sExceptionApp.path.SetString(strFilePath);
			g_pParent->m_listExceptionApp.push_back(sExceptionApp);
			UpdateExceptionList();
		}
	}

}


void CExceptionSettingDlg::OnBnClickedButtonDel()
{
	int nItem;
	CString strFileName;
	list<EXCEPTION_APP>::iterator itor;
	POSITION pos = ctr_listException.GetFirstSelectedItemPosition();
	if(pos == NULL){
		return;
	}else{
		while(pos){
			nItem = ctr_listException.GetNextSelectedItem(pos);
			strFileName = ctr_listException.GetItemText(nItem, 0);
			
			itor = g_pParent->m_listExceptionApp.begin();
			while(itor != g_pParent->m_listExceptionApp.end())
			{
				if(strFileName.Compare(itor->name) == 0){
					g_pParent->m_listExceptionApp.erase(itor);
					break;
				}
				itor++;
			}
		}
	}
	UpdateExceptionList();

}
