//
// CaptureDemo1Dlg.h : 头文件
//

#pragma once

#include "DShowCapture.h"
#include "afxwin.h"

// CCaptureDemo1Dlg 对话框
class CCaptureDemoDlg : public CDialogEx
{
// 构造
public:
	CCaptureDemoDlg(CWnd * pParent = NULL);
    ~CCaptureDemoDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CAPTUREDEMO1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
