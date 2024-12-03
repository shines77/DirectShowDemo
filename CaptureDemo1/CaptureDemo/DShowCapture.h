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

    // 可调用函数：枚举视频采集设备,返回设备数量
    int ListVideoDevices();

    // 可调用函数：枚举音频采集设备
    BOOL ListAudioDevices();

    // 可调用函数：枚举视频压缩格式
    BOOL ListVideoCompressFormat();

    // 可调用函数：枚举音频压缩格式
    BOOL ListAudioCompressFormat();

    // 可调用函数：根据选择的设备创建Video Capture Filter
    bool CreateVideoFilter(const char * selectedDevice = NULL);

    // 可调用函数：操作视频
    bool Render(int mode, TCHAR * videoPath = NULL,
                const char * selectedDevice = NULL);

    // 可调用函数：停止当前操作
    bool StopCurrentOperating(int _stop_type = 0);

    // 可调用函数：暂停播放本地视频
    bool PausePlayingLocalVideo();

    // 可调用函数：继续播放本地视频
    bool ContinuePlayingLocalVideo();

private:
    HWND                    hwndPreview_;           // 预览窗口句柄

    IGraphBuilder *         pVideoFilterGraph_;     // Video filter graph (管理器)
    ICaptureGraphBuilder2 * pVideoCaptureGraph_;    // Video capture graph (管理器)
    IBaseFilter *           pVideoFilter_;          // Video filter
    IBaseFilter *           pVideoMux_;             // Video file muxer (用于保存视频文件)
    IVideoWindow *          pVideoWindow_;          // 视频播放窗口
    IMediaControl *         pVideoMediaControl_;    // 摄像头流媒体的控制开关（控制其开始、暂停、结束）
    IMediaEvent *           pVideoMediaEvent_;      // 摄像头流媒体的控制事件

};

