
// WebcamProtector.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CWebcamProtectorApp:
// �� Ŭ������ ������ ���ؼ��� WebcamProtector.cpp�� �����Ͻʽÿ�.
//

class CWebcamProtectorApp : public CWinApp
{
public:
	CWebcamProtectorApp();
	
	HANDLE m_handle;
	int m_myVersion[4];
	int m_newVersion[4];
	int m_execError;

	int m_winVersion[3];
	bool m_is64bit;

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CWebcamProtectorApp theApp;