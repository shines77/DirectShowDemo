#pragma once

#include <DShow.h>
#include <tchar.h>

#include <string>
#include <vector>

#if 1
#ifdef _DEBUG
#pragma comment(lib, "strmbased.lib")
#else
#pragma comment(lib, "strmbase.lib")
#endif
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "quartz.lib")
#endif

#define UVC_PREVIEW_VIDEO       1
#define UVC_RECORD_VIDEO        2
#define UVC_LOCAL_VIDEO         3

#define UVC_STOP_PREVIEW        1
#define UVC_STOP_RECORD         2
#define UVC_STOP_LOCAL_VIDEO    3

class DShowCapture
{
public:
    DShowCapture(HWND hwndPreview = NULL);
    ~DShowCapture(void);

    std::vector<std::string> mDeviceNameList;

    HRESULT Init();

    HWND SetPreviewHwnd(HWND hwndPreview, bool bAttachTo = false);
    bool AttachToVideoWindow(HWND hwndPreview);

    // �ɵ��ú�����ö����Ƶ�ɼ��豸,�����豸����
    int ListVideoDevices();

    // �ɵ��ú�����ö����Ƶ�ɼ��豸
    BOOL ListAudioDevices();

    // �ɵ��ú�����ö����Ƶѹ����ʽ
    BOOL ListVideoCompressFormat();

    // �ɵ��ú�����ö����Ƶѹ����ʽ
    BOOL ListAudioCompressFormat();

    // �ɵ��ú���������ѡ����豸����Video Capture Filter
    bool CreateVideoFilter(const char * selectedDevice = NULL);

    // �ɵ��ú�����������Ƶ
    bool Render(int mode, TCHAR * videoPath = NULL,
                const char * selectedDevice = NULL);

    // �ɵ��ú�����ֹͣ��ǰ����
    bool StopCurrentOperating(int _stop_type = 0);

    // �ɵ��ú�������ͣ���ű�����Ƶ
    bool PausePlayingLocalVideo();

    // �ɵ��ú������������ű�����Ƶ
    bool ContinuePlayingLocalVideo();

private:
    HWND                    hwndPreview_;           // Ԥ�����ھ��

    IGraphBuilder *         pVideoFilterGraph_;     // Video filter graph (������)
    ICaptureGraphBuilder2 * pVideoCaptureGraph_;    // Video capture graph (������)
    IBaseFilter *           pVideoFilter_;          // Video filter
    IBaseFilter *           pVideoMux_;             // Video file muxer (���ڱ�����Ƶ�ļ�)
    IVideoWindow *          pVideoWindow_;          // ��Ƶ���Ŵ���
    IMediaControl *         pVideoMediaControl_;    // ����ͷ��ý��Ŀ��ƿ��أ������俪ʼ����ͣ��������
    IMediaEvent *           pVideoMediaEvent_;      // ����ͷ��ý��Ŀ����¼�

};

