
// CaptureDemo1.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CCaptureDemo1App: 
// �йش����ʵ�֣������ CaptureDemo1.cpp
//

class CCaptureDemoApp : public CWinApp
{
public:
	CCaptureDemoApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CCaptureDemoApp theApp;