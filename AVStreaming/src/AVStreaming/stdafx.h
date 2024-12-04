
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ

#include <afxdisp.h>        // MFC �Զ�����

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#ifndef SAFE_INTERFACE_RELEASE
#define SAFE_INTERFACE_RELEASE(pInterface) \
    do {                            \
        if (pInterface != NULL) {   \
            pInterface->Release();  \
            pInterface = NULL;      \
        }                           \
    } while (0)
#endif // SAFE_INTERFACE_RELEASE

#ifndef SAFE_OBJECT_DELETE
#define SAFE_OBJECT_DELETE(pObject) \
    do {                            \
        if (pObject != NULL) {      \
            delete pObject;         \
            pObject = NULL;         \
        }                           \
    } while (0)
#endif // SAFE_OBJECT_RELEASE

#include <string>

#ifdef _UNICODE
namespace std {
    using tstring = wstring;
}
#else
namespace std {
    using tstring = string;
}
}
#endif // _UNICODE

#ifdef _UNICODE
std::wstring Ansi2Unicode(const std::string & ansiStr);
std::string Unicode2Ansi(const std::wstring & unicodeStr);
#else
std::string Ansi2Unicode(const std::string & str)
std::string Unicode2Ansi(const std::string & str);
#endif // _UNICODE
