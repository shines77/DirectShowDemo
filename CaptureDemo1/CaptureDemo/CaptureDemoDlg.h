//
// CaptureDemo1Dlg.h : ͷ�ļ�
//

#pragma once

#include "DShowCapture.h"
#include "afxwin.h"

// CCaptureDemo1Dlg �Ի���
class CCaptureDemoDlg : public CDialogEx
{
// ����
public:
	CCaptureDemoDlg(CWnd * pParent = NULL);
    ~CCaptureDemoDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CAPTUREDEMO1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnCbnSelchangeDeviceList();
	DECLARE_MESSAGE_MAP()

private:
    DShowCapture * camera_;

    CComboBox m_DeviceList;
    CStatic m_CameraPreview;
    
};
