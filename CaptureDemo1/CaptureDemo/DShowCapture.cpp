#include "stdafx.h"
#include "DShowCapture.h"

#include <atlconv.h>

//
// From: https://blog.csdn.net/ett_qin/article/details/86691479
// See: https://www.cnblogs.com/linuxAndMcu/p/12068978.html
//

DShowCapture::DShowCapture(HWND hwndPreview /* = NULL */)
{
    hwndPreview_ = hwndPreview;

    pVideoFilterGraph_ = NULL;
    pVideoCaptureGraph_ = NULL;
    pVideoFilter_ = NULL;
    pVideoMux_ = NULL;
    pVideoWindow_ = NULL;
    pVideoMediaControl_ = NULL;
    pVideoMediaEvent_ = NULL;
}

DShowCapture::~DShowCapture(void)
{
    SAFE_INTERFACE_RELEASE(pVideoMux_);
    SAFE_INTERFACE_RELEASE(pVideoWindow_);
    SAFE_INTERFACE_RELEASE(pVideoMediaControl_);
    SAFE_INTERFACE_RELEASE(pVideoMediaEvent_);
    SAFE_INTERFACE_RELEASE(pVideoFilter_);
    SAFE_INTERFACE_RELEASE(pVideoCaptureGraph_);
    SAFE_INTERFACE_RELEASE(pVideoFilterGraph_);
}

HRESULT DShowCapture::Init()
{
    HRESULT hr;

    // 创建 filter graph manager
    SAFE_INTERFACE_RELEASE(pVideoFilterGraph_);
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pVideoFilterGraph_);
	if (FAILED(hr))
		return hr;
    
    // 创建 capture graph manager
    SAFE_INTERFACE_RELEASE(pVideoCaptureGraph_);
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
                          IID_ICaptureGraphBuilder2, (void**)&pVideoCaptureGraph_);
	if (FAILED(hr))
		return hr;

    // 创建视频播放窗口
    SAFE_INTERFACE_RELEASE(pVideoWindow_);
    hr = pVideoFilterGraph_->QueryInterface(IID_IVideoWindow, (void **)&pVideoWindow_);
	if (FAILED(hr))
		return hr;

    // 创建摄像头流媒体的控制开关
    SAFE_INTERFACE_RELEASE(pVideoMediaControl_);
    hr = pVideoFilterGraph_->QueryInterface(IID_IMediaControl, (void **)&pVideoMediaControl_);
	if (FAILED(hr))
		return hr;

    // 创建摄像头流媒体的控制事件
    SAFE_INTERFACE_RELEASE(pVideoMediaEvent_);
    hr = pVideoFilterGraph_->QueryInterface(IID_IMediaEvent, (void **)&pVideoMediaEvent_);
	if (FAILED(hr))
		return hr;

    // 为 capture graph 指定要使用的 filter graph
    if (pVideoCaptureGraph_ != NULL && pVideoFilterGraph_ != NULL) {
        hr = pVideoCaptureGraph_->SetFiltergraph(pVideoFilterGraph_);
	    if (FAILED(hr))
		    return hr;
    }
    return hr;
}

HWND DShowCapture::SetPreviewHwnd(HWND hwndPreview, bool bAttachTo /* = false */)
{
    HWND oldHwnd = hwndPreview_;
    hwndPreview_ = hwndPreview;
    if (bAttachTo) {
        AttachToVideoWindow(hwndPreview);
    }
    return oldHwnd;
}

bool DShowCapture::AttachToVideoWindow(HWND hwndPreview)
{
    // 检查视频播放窗口
    if (pVideoWindow_ == NULL)
        return false;

    if (hwndPreview != NULL && ::IsWindow(hwndPreview)) {
        // 视频流放入 "Picture控件" 中来预览视频
        if (pVideoWindow_->put_Owner((OAHWND)hwndPreview) < 0)
            return false;

        if (pVideoWindow_->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN) < 0)
            return false;

        // 让图像充满整个窗口
        CRect rc;
        ::GetClientRect(hwndPreview, &rc);
        pVideoWindow_->SetWindowPosition(0, 0, rc.right, rc.bottom);

        if (pVideoWindow_->put_Visible(OATRUE) < 0)
            return false;
    }

    return true;
}

// 枚举视频采集设备
int DShowCapture::ListVideoDevices()
{
    HRESULT hr = S_OK;

    // 创建系统设备枚举
    ICreateDevEnum * pCreateDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (FAILED(hr) || pCreateDevEnum == NULL) {
        return hr;
    }

    // 创建一个指定视频采集设备的枚举
    IEnumMoniker * pEnumMoniker = NULL;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
    if (FAILED(hr) || pEnumMoniker == NULL) {
        SAFE_INTERFACE_RELEASE(pCreateDevEnum);
        return hr;
    }

    ULONG cFetched = 0;
    IMoniker * pMoniker = NULL;
    int video_dev_total = 0;
    int video_dev_count = 0;
    mDeviceNameList.clear();

    // 枚举设备
    while (hr = pEnumMoniker->Next(1, &pMoniker, &cFetched), hr == S_OK) {
        // 设备属性信息
        IPropertyBag * pPropBag = NULL;
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
        if (SUCCEEDED(hr)) {
            VARIANT var;
            var.vt = VT_BSTR;
            hr = pPropBag->Read(L"Description", &var, NULL);
            if (FAILED(hr)) {
                hr = pPropBag->Read(L"FriendlyName", &var, NULL);
            }
            if (SUCCEEDED(hr)) {
                char deviceName[256] = { '\0' };
                // 获取设备名称
                WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, deviceName, sizeof(deviceName), "", NULL);
                SysFreeString(var.bstrVal);
                // 尝试用当前设备绑定到 video filter
                IBaseFilter * pVideoFilter = NULL;
                hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void **)&pVideoFilter);
                if (SUCCEEDED(hr)) {
                    // 绑定成功则添加到设备列表
                    mDeviceNameList.push_back(deviceName);
                    video_dev_count++;
                    if (pVideoFilter != NULL) {
                        pVideoFilter->Release();
                    }
                }
                else {
                    OutputDebugString(_T("BindToObject Failed\n"));
                }                
            }
            pPropBag->Release();
        }
        pMoniker->Release();
        pMoniker = NULL;
        video_dev_total++;
    }

    pEnumMoniker->Release();
    pCreateDevEnum->Release();

    return video_dev_count;
}

// 枚举音频采集设备
BOOL DShowCapture::ListAudioDevices()
{
    return 0;
}

// 枚举视频压缩格式
BOOL DShowCapture::ListVideoCompressFormat()
{
    return 0;
}

// 枚举音频压缩格式
BOOL DShowCapture::ListAudioCompressFormat()
{
    return 0;
}

// 根据选择的设备获取 Video Capture Filter
bool DShowCapture::CreateVideoFilter(const char * selectedDevice)
{
    // 创建系统设备枚举
    ICreateDevEnum * pCreateDevEnum = NULL;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (FAILED(hr) || pCreateDevEnum == NULL) {
        return false;
    }

    // 创建一个指定视频采集设备的枚举
    IEnumMoniker * pEnumMoniker = NULL;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
    if (FAILED(hr) || pEnumMoniker == NULL) {
        SAFE_INTERFACE_RELEASE(pCreateDevEnum);
        return false;
    }

    bool result = false;
    ULONG cFetched = 0;
    IMoniker * pMoniker = NULL;

    // 枚举设备
    while (hr = pEnumMoniker->Next(1, &pMoniker, &cFetched), hr == S_OK) {
        // 设备属性信息
        IPropertyBag *pPropBag = NULL;
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
        if (SUCCEEDED(hr)) {
            VARIANT var;
            var.vt = VT_BSTR;
            hr = pPropBag->Read(L"Description", &var, NULL);
            if (FAILED(hr)) {
                hr = pPropBag->Read(L"FriendlyName", &var, NULL);
            }
            if (SUCCEEDED(hr)) {
                char deviceName[256] = { '\0' };
                // 获取设备名称
                WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, deviceName, sizeof(deviceName), "", NULL);
                SysFreeString(var.bstrVal);
                if (selectedDevice != NULL && strcmp(selectedDevice, deviceName) == 0) {
                    SAFE_INTERFACE_RELEASE(pVideoFilter_);
                    // 尝试用当前设备绑定到 video filter
                    hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void **)&pVideoFilter_);
                    if (SUCCEEDED(hr) && pVideoFilter_ != NULL) {
                        result = true;
                    }
                    else {
                        OutputDebugString(_T("BindToObject Failed\n"));
                    }
                }
            }
            pPropBag->Release();
        }
        pMoniker->Release();
        pMoniker = NULL;
    }

    pEnumMoniker->Release();
    pCreateDevEnum->Release();
    return result;
}

// 渲染摄像头预览视频
bool DShowCapture::Render(int mode, TCHAR * videoPath, const char * selectedDevice)
{
    HRESULT hr;

    // 检查 Video filter graph (管理器)
    if (pVideoFilterGraph_ == NULL)
        return false;

    if (mode != UVC_LOCAL_VIDEO) {
        // 创建 Video filter
        bool result = CreateVideoFilter(selectedDevice);
        if (!result)
            return false;

        // 检查 Video capture graph (管理器)
        if (pVideoCaptureGraph_ == NULL)
            return false;

        // 将获取到的 Video filter 加入 Video filter graph
        if (hr = pVideoFilterGraph_->AddFilter(pVideoFilter_, L"Video Filter"), FAILED(hr))
            return false;
    }

    if (mode == UVC_PREVIEW_VIDEO) {     // 预览视频
        if (pVideoCaptureGraph_->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pVideoFilter_, NULL, NULL) < 0)
            return false;
    }
    else if (mode == UVC_RECORD_VIDEO) { // 录制视频
        USES_CONVERSION;
        LPCOLESTR OLE_PathName = T2OLE(videoPath); // 类型转化

        // 设置输出文件
        SAFE_INTERFACE_RELEASE(pVideoMux_);
        if (pVideoCaptureGraph_->SetOutputFileName(&MEDIASUBTYPE_Avi, OLE_PathName, &pVideoMux_, NULL) < 0)
            return false;

        // 录制的时候也需要预览
        if (pVideoCaptureGraph_->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pVideoFilter_, NULL, NULL) < 0)
            return false;

        // 视频流写入 avi 文件
        if (pVideoMux_ != NULL && pVideoCaptureGraph_->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pVideoFilter_, NULL, pVideoMux_) < 0)
            return false;
    }
    else if (mode == UVC_LOCAL_VIDEO) {   // 播放本地视频
        USES_CONVERSION;
        LPCOLESTR OLE_PathName = T2OLE(videoPath); // 类型转化
        pVideoFilterGraph_->RenderFile(OLE_PathName, NULL);
    }

    /******* 设置视频播放窗口 *******/
    if (!AttachToVideoWindow(hwndPreview_))
        return false;

    // 检查摄像头流媒体的控制开关
    if (pVideoMediaControl_ == NULL)
        return false;

    // 开始预览或录制
    if (pVideoMediaControl_->Run() < 0)
        return false;

    return true;
}

// 关闭摄像头
bool DShowCapture::StopCurrentOperating(int stop_type)
{
    if (pVideoMediaControl_->Stop() < 0)
        return false;

    if (stop_type != UVC_STOP_LOCAL_VIDEO) {
        pVideoFilter_->Release();
        pVideoCaptureGraph_->Release();
    }

    pVideoWindow_->Release();
    pVideoMediaControl_->Release();
    pVideoFilterGraph_->Release();
    return true;
}

// 暂停播放本地视频
bool DShowCapture::PausePlayingLocalVideo()
{
    if (pVideoMediaControl_->Stop() < 0)
        return false;
    else
        return true;
}

// 继续播放本地视频
bool DShowCapture::ContinuePlayingLocalVideo()
{
    if (pVideoMediaControl_->Run() < 0)
        return false;
    else
        return true;
}
