
// AVStreaming.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CAVStreamingApp: 
// �йش����ʵ�֣������ AVStreaming.cpp
//

class CAVStreamingApp : public CWinApp
{
public:
	CAVStreamingApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CAVStreamingApp theApp;