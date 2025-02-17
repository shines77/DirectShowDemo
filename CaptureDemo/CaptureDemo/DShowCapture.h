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

    // 枚举视频采集设备, 返回设备数量
    int ListVideoDevices();

    // 枚举音频采集设备
    int ListAudioDevices();

    // 枚举视频压缩格式
    int ListVideoCompressFormat();

    // 枚举音频压缩格式
    int ListAudioCompressFormat();

    // 根据选择的设备创建 Video Capture Filter
    bool CreateVideoFilter(const char * selectedDevice = NULL);

    // 渲染预览窗口
    bool Render(int mode, TCHAR * videoPath = NULL,
                const char * selectedDevice = NULL);

    // 停止当前操作
    bool StopCurrentOperating(int _stop_type = 0);

    // 暂停播放本地视频
    bool PausePlayingLocalVideo();

    // 继续播放本地视频
    bool ContinuePlayingLocalVideo();

private:
    HWND                    hwndPreview_;           // 预览窗口句柄

    IGraphBuilder *         pFilterGraph_;          // filter graph (Manager)
    ICaptureGraphBuilder2 * pCaptureBuilder_;       // capture graph (Builder)
    IBaseFilter *           pVideoFilter_;          // Video filter
    IBaseFilter *           pVideoMux_;             // Video file muxer (用于保存视频文件)
    IVideoWindow *          pVideoWindow_;          // 视频播放窗口
    IMediaControl *         pVideoMediaControl_;    // 摄像头流媒体的控制开关（控制其开始、暂停、结束）
    IMediaEvent *           pVideoMediaEvent_;      // 摄像头流媒体的控制事件
};
