
// WebcamProtectorDlg.h : ��� ����
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "ExceptionSettingDlg.h"
#include "AlertAccessWebcamDlg.h"

typedef struct _exceptionApp{
	CString name;
	CString path;
} EXCEPTION_APP;

const UINT WM_ALERT_ACCESSWEBCAM = ::RegisterWindowMessage("WM_ALERT_ACCESSWEBCAM");

// CWebcamProtectorDlg ��ȭ ����
class CWebcamProtectorDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CWebcamProtectorDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_WEBCAMPROTECTOR_DIALOG };

	unsigned int m_myPID;
	bool m_enableProtection; // ��ȣ ���� ����
	list<CString> m_listDeviceName; // ��ġ �̸� ����Ʈ
	list<EXCEPTION_APP> m_listExceptionApp; // ���� ���ø����̼� ����Ʈ
	
	HANDLE m_hDevice;
	bool m_isRunningService; // ���� ���� ����

	HANDLE m_hAccessWebcam; // ���� ���� �̺�Ʈ �ڵ�
	HANDLE m_hPidAccessApp; // ���� �õ� ���ø����̼� PID

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	BOOL OnDeviceChange(UINT nEventType, DWORD dwData);
public:
	CListCtrl ctr_listDevice;
	CButton ctr_checkProtection;
	afx_msg void OnBnClickedCheckProtection();
	afx_msg void OnDestroy();
	afx_msg void OnNMClickSyslinkException(NMHDR *pNMHDR, LRESULT *pResult);
	CExceptionSettingDlg m_pExceptionSettingDlg;
	CAlertAccessWebcamDlg m_pAlertAccessWebcamDlg;
	bool StartWebcamProtection();
	bool StopWebcamProtection();
	bool InitDriverService();
	bool CloseDriverService();
	void UpdateDeviceList(const GUID *pDevClass);
	bool SendMessageDriverDevice();
	bool SendMessageDriverExApp();
	LRESULT AlertAccessWebcamPopup(WPARAM wParam, LPARAM lParam);
	int m_checkProtection;
	CString m_labelStatus;
};
