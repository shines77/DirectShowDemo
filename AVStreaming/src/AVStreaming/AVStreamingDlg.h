//
// AVStreamingDlg.h : ͷ�ļ�
//

#pragma once

#include "DShowCapture.h"
#include "afxwin.h"

// CAVStreamingDlg �Ի���
class CAVStreamingDlg : public CDialogEx
{
// ����
public:
	CAVStreamingDlg(CWnd * pParent = NULL);
    ~CAVStreamingDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AVSTREAMING_DIALOG };
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
    afx_msg void OnCbnSelChangeVideoDeviceList();
	DECLARE_MESSAGE_MAP()

    int EnumVideoDeviceList();
    int EnumAudioDeviceList();

private:
    DShowCapture * camera_;

    CStatic     m_wndCameraPreview;
    CComboBox   m_cbxVideoDeviceList;
    CComboBox   m_cbxAudioDeviceList;
};
