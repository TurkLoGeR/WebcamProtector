#pragma once

// CExceptionSettingDlg ��ȭ �����Դϴ�.

class CExceptionSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CExceptionSettingDlg)

public:
	CExceptionSettingDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CExceptionSettingDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_EXCEPTIONSETTINGDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	CListCtrl ctr_listException;
	void UpdateExceptionList();
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonDel();
};
