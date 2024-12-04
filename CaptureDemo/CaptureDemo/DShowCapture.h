#pragma once

#include <DShow.h>
#include <tchar.h>

#include <string>
#include <vector>

#if 1
//#pragma comment(lib, "winmm.lib")
//#pragma comment(lib, "legacy_stdio_definitions.lib")

#ifdef _DEBUG
//#pragma comment(lib, "vcruntimed.lib")
#pragma comment(lib, "strmbased.lib")
#else
//#pragma comment(lib, "vcruntime.lib")
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

struct CamVideoConfig
{
	SIZE InputSize;
    SIZE MinOutputSize;
    SIZE MaxOutputSize;
    double MinFPS;
    double MaxFPS;
    ULONG cbFormat;
	int FormatIndex;
 
	CamVideoConfig() {
		InputSize.cx = 0;
        InputSize.cy = 0;
        MinOutputSize.cx = 0;
        MinOutputSize.cy = 0;
        MaxOutputSize.cx = 0;
        MaxOutputSize.cy = 0;
        MinFPS = 0.0;
        MaxFPS = 0.0;
        cbFormat = -1;
		FormatIndex = -1;
	}

	CamVideoConfig(const CamVideoConfig & other) {
		*this = other;
	}

	CamVideoConfig & operator = (const CamVideoConfig & other) {
		InputSize = other.InputSize;
		MinOutputSize = other.MinOutputSize;
        MaxOutputSize = other.MaxOutputSize;
        MinFPS = other.MinFPS;
        MaxFPS = other.MaxFPS;
        cbFormat = other.cbFormat;
		FormatIndex = other.FormatIndex;
		return *this;
	}
};

class DShowCapture
{
public:
    DShowCapture(HWND hwndPreview = NULL);
    ~DShowCapture(void);

    std::vector<std::string> videoDeviceList_;
    std::vector<std::string> audioDeviceList_;

    std::vector<CamVideoConfig> videoConfigures_;
    std::vector<int>               videoFPSList_;

    static const int kFrameIntervalPerSecond = 10000000;

    HRESULT Init();

    HWND SetPreviewHwnd(HWND hwndPreview, bool bAttachTo = false);
    bool AttachToVideoWindow(HWND hwndPreview);

    int ListVideoConfigures();

    // ö����Ƶ�ɼ��豸, �����豸����
    int ListVideoDevices();

    // ö����Ƶ�ɼ��豸
    int ListAudioDevices();

    // ö����Ƶѹ����ʽ
    int ListVideoCompressFormat();

    // ö����Ƶѹ����ʽ
    int ListAudioCompressFormat();

    // ����ѡ����豸���� Video Capture Filter
    bool CreateVideoFilter(const char * selectedDevice = NULL);

    // ��ȾԤ������
    bool Render(int mode, TCHAR * videoPath = NULL,
                const char * selectedDevice = NULL);

    // ֹͣ��ǰ����
    bool StopCurrentOperating(int _stop_type = 0);

    // ��ͣ���ű�����Ƶ
    bool PausePlayingLocalVideo();

    // �������ű�����Ƶ
    bool ContinuePlayingLocalVideo();

private:
    HWND                    hwndPreview_;           // Ԥ�����ھ��

    IGraphBuilder *         pFilterGraph_;          // filter graph (Manager)
    ICaptureGraphBuilder2 * pCaptureBuilder_;       // capture graph (Builder)
    IBaseFilter *           pVideoFilter_;          // Video filter
    IBaseFilter *           pVideoMux_;             // Video file muxer (���ڱ�����Ƶ�ļ�)
    IVideoWindow *          pVideoWindow_;          // ��Ƶ���Ŵ���
    IMediaControl *         pVideoMediaControl_;    // ����ͷ��ý��Ŀ��ƿ��أ������俪ʼ����ͣ��������
    IMediaEvent *           pVideoMediaEvent_;      // ����ͷ��ý��Ŀ����¼�
};
