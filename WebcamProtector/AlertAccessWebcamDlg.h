#pragma once
#include "afxcmn.h"


// CAlertAccessWebcamDlg ��ȭ �����Դϴ�.

class CAlertAccessWebcamDlg : public CDialog
{
	DECLARE_DYNAMIC(CAlertAccessWebcamDlg)

public:
	CAlertAccessWebcamDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CAlertAccessWebcamDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ALERTACCESSWEBCAMDLG };

	CString m_strTargetAppPath;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonAllow();
	afx_msg void OnBnClickedButtonBlock();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	CLinkCtrl ctr_linkProcPath;
};
