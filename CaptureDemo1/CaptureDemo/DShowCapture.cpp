#include "stdafx.h"
#include "DShowCapture.h"

#include "Mtype.h"

#include <atlconv.h>

//
// From: https://blog.csdn.net/ett_qin/article/details/86691479
// See: https://www.cnblogs.com/linuxAndMcu/p/12068978.html
//

DShowCapture::DShowCapture(HWND hwndPreview /* = NULL */)
{
    hwndPreview_ = hwndPreview;

    pFilterGraph_ = NULL;
    pCaptureBuilder_ = NULL;
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
    SAFE_INTERFACE_RELEASE(pCaptureBuilder_);
    SAFE_INTERFACE_RELEASE(pFilterGraph_);
}

HRESULT DShowCapture::Init()
{
    HRESULT hr;

    // ���� filter graph manager
    SAFE_INTERFACE_RELEASE(pFilterGraph_);
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pFilterGraph_);
	if (FAILED(hr))
		return hr;

    // ���� capture graph manager
    SAFE_INTERFACE_RELEASE(pCaptureBuilder_);
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
                          IID_ICaptureGraphBuilder2, (void**)&pCaptureBuilder_);
	if (FAILED(hr))
		return hr;

    // ������Ƶ���Ŵ���
    SAFE_INTERFACE_RELEASE(pVideoWindow_);
    hr = pFilterGraph_->QueryInterface(IID_IVideoWindow, (void **)&pVideoWindow_);
	if (FAILED(hr))
		return hr;

    // ��������ͷ��ý��Ŀ��ƿ���
    SAFE_INTERFACE_RELEASE(pVideoMediaControl_);
    hr = pFilterGraph_->QueryInterface(IID_IMediaControl, (void **)&pVideoMediaControl_);
	if (FAILED(hr))
		return hr;

    // ��������ͷ��ý��Ŀ����¼�
    SAFE_INTERFACE_RELEASE(pVideoMediaEvent_);
    hr = pFilterGraph_->QueryInterface(IID_IMediaEvent, (void **)&pVideoMediaEvent_);
	if (FAILED(hr))
		return hr;

    // Ϊ capture graph ָ��Ҫʹ�õ� filter graph
    if (pCaptureBuilder_ != NULL && pFilterGraph_ != NULL) {
        hr = pCaptureBuilder_->SetFiltergraph(pFilterGraph_);
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
    // �����Ƶ���Ŵ���
    if (pVideoWindow_ == NULL)
        return false;

    if (hwndPreview != NULL && ::IsWindow(hwndPreview)) {
        // ��Ƶ������ "Picture�ؼ�" ����Ԥ����Ƶ
        if (pVideoWindow_->put_Owner((OAHWND)hwndPreview) < 0)
            return false;

        if (pVideoWindow_->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN) < 0)
            return false;

        // ��ͼ�������������
        CRect rc;
        ::GetClientRect(hwndPreview, &rc);
        pVideoWindow_->SetWindowPosition(0, 0, rc.right, rc.bottom);

        if (pVideoWindow_->put_Visible(OATRUE) < 0)
            return false;
    }

    return true;
}

int DShowCapture::ListVideoConfigures()
{
	if (pCaptureBuilder_ != NULL) {
		videoConfigures_.clear();
		IAMStreamConfig * pStreamConfig = NULL;
		// &MEDIATYPE_Video, �����������ý������, �ڶ�����������Ϊ NULL, ��ʾ Any media type.
		HRESULT hr = pCaptureBuilder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
										             pVideoFilter_, IID_IAMStreamConfig, (void **)&pStreamConfig);
        if (FAILED(hr)) {
            return -1;
        }

		int nCount = 0, nSize = 0;
		hr = pStreamConfig->GetNumberOfCapabilities(&nCount, &nSize);
        if (FAILED(hr)) {
            SAFE_INTERFACE_RELEASE(pStreamConfig);
            return -1;
        }

		// Check the size to make sure we pass in the correct structure.
		if (nSize == sizeof(VIDEO_STREAM_CONFIG_CAPS)) {
            // Use the video capabilities structure.
            for (int nFormat = 0; nFormat < nCount; nFormat++) {
                VIDEO_STREAM_CONFIG_CAPS vscc;
                AM_MEDIA_TYPE * pAmtConfig = NULL;
                hr = pStreamConfig->GetStreamCaps(nFormat, &pAmtConfig, (BYTE *)&vscc);
                if (SUCCEEDED(hr)) {
                    /* Examine the format, and possibly use it. */
                    CamVideoConfig config;
                    config.InputSize = vscc.InputSize;
                    config.MinOutputSize = vscc.MinOutputSize;
                    config.MaxOutputSize = vscc.MaxOutputSize;
                    config.MinFPS = (double)kFrameIntervalPerSecond / vscc.MinFrameInterval;
                    config.MaxFPS = (double)kFrameIntervalPerSecond / vscc.MaxFrameInterval;
                    config.cbFormat = pAmtConfig->cbFormat;
                    config.FormatIndex = nFormat;

                    videoConfigures_.push_back(config);

                    // Delete the media type when you are done.
                    _DeleteMediaType(pAmtConfig);
                }
            }
        }
    }

    return 0;
}

// ö����Ƶ�ɼ��豸
int DShowCapture::ListVideoDevices()
{
    HRESULT hr = S_OK;

    // ����ϵͳ�豸ö��
    ICreateDevEnum * pCreateDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (FAILED(hr) || pCreateDevEnum == NULL) {
        return hr;
    }

    // ����һ��ָ����Ƶ�ɼ��豸��ö��
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
    videoDeviceList_.clear();

    // ö���豸
    while (hr = pEnumMoniker->Next(1, &pMoniker, &cFetched), hr == S_OK) {
        // �豸������Ϣ
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
                // ��ȡ�豸����
                WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, deviceName, sizeof(deviceName), "", NULL);
                SysFreeString(var.bstrVal);
                // �����õ�ǰ�豸�󶨵� video filter
                IBaseFilter * pVideoFilter = NULL;
                hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void **)&pVideoFilter);
                if (SUCCEEDED(hr)) {
                    // �󶨳ɹ�����ӵ��豸�б�
                    videoDeviceList_.push_back(deviceName);
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

// ö����Ƶ�ɼ��豸
int DShowCapture::ListAudioDevices()
{
    return 0;
}

// ö����Ƶѹ����ʽ
int DShowCapture::ListVideoCompressFormat()
{
    return 0;
}

// ö����Ƶѹ����ʽ
int DShowCapture::ListAudioCompressFormat()
{
    return 0;
}

// ����ѡ����豸��ȡ Video Capture Filter
bool DShowCapture::CreateVideoFilter(const char * selectedDevice)
{
    // ����ϵͳ�豸ö��
    ICreateDevEnum * pCreateDevEnum = NULL;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (FAILED(hr) || pCreateDevEnum == NULL) {
        return false;
    }

    // ����һ��ָ����Ƶ�ɼ��豸��ö��
    IEnumMoniker * pEnumMoniker = NULL;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
    if (FAILED(hr) || pEnumMoniker == NULL) {
        SAFE_INTERFACE_RELEASE(pCreateDevEnum);
        return false;
    }

    bool result = false;
    ULONG cFetched = 0;
    IMoniker * pMoniker = NULL;

    // ö���豸
    while (hr = pEnumMoniker->Next(1, &pMoniker, &cFetched), hr == S_OK) {
        // �豸������Ϣ
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
                // ��ȡ�豸����
                WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, deviceName, sizeof(deviceName), "", NULL);
                SysFreeString(var.bstrVal);
                if (selectedDevice != NULL && strcmp(selectedDevice, deviceName) == 0) {
                    SAFE_INTERFACE_RELEASE(pVideoFilter_);
                    // �����õ�ǰ�豸�󶨵� video filter
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

// ��Ⱦ����ͷԤ����Ƶ
bool DShowCapture::Render(int mode, TCHAR * videoPath, const char * selectedDevice)
{
    HRESULT hr;

    // ��� Video filter graph (������)
    if (pFilterGraph_ == NULL)
        return false;

    if (mode != UVC_LOCAL_VIDEO) {
        // ���� Video filter
        bool result = CreateVideoFilter(selectedDevice);
        if (!result)
            return false;

        // ��� Video capture graph (������)
        if (pCaptureBuilder_ == NULL)
            return false;

        // ����ȡ���� Video filter ���� Video filter graph
        if (hr = pFilterGraph_->AddFilter(pVideoFilter_, L"Video Filter"), FAILED(hr))
            return false;
    }

    if (mode == UVC_PREVIEW_VIDEO) {     // Ԥ����Ƶ
        if (pCaptureBuilder_->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pVideoFilter_, NULL, NULL) < 0)
            return false;
    }
    else if (mode == UVC_RECORD_VIDEO) { // ¼����Ƶ
        USES_CONVERSION;
        LPCOLESTR OLE_PathName = T2OLE(videoPath); // ����ת��

        // ��������ļ�
        SAFE_INTERFACE_RELEASE(pVideoMux_);
        if (pCaptureBuilder_->SetOutputFileName(&MEDIASUBTYPE_Avi, OLE_PathName, &pVideoMux_, NULL) < 0)
            return false;

        // ¼�Ƶ�ʱ��Ҳ��ҪԤ��
        if (pCaptureBuilder_->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pVideoFilter_, NULL, NULL) < 0)
            return false;

        // ��Ƶ��д�� avi �ļ�
        if (pVideoMux_ != NULL && pCaptureBuilder_->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pVideoFilter_, NULL, pVideoMux_) < 0)
            return false;
    }
    else if (mode == UVC_LOCAL_VIDEO) {   // ���ű�����Ƶ
        USES_CONVERSION;
        LPCOLESTR OLE_PathName = T2OLE(videoPath); // ����ת��
        pFilterGraph_->RenderFile(OLE_PathName, NULL);
    }

    /******* ������Ƶ���Ŵ��� *******/
    if (!AttachToVideoWindow(hwndPreview_))
        return false;

    // �������ͷ��ý��Ŀ��ƿ���
    if (pVideoMediaControl_ == NULL)
        return false;

    // ��ʼԤ����¼��
    if (pVideoMediaControl_->Run() < 0)
        return false;

    return true;
}

// �ر�����ͷ
bool DShowCapture::StopCurrentOperating(int stop_type)
{
    if (pVideoMediaControl_->Stop() < 0)
        return false;

    if (stop_type != UVC_STOP_LOCAL_VIDEO) {
        pVideoFilter_->Release();
        pCaptureBuilder_->Release();
    }

    pVideoWindow_->Release();
    pVideoMediaControl_->Release();
    pFilterGraph_->Release();
    return true;
}

// ��ͣ���ű�����Ƶ
bool DShowCapture::PausePlayingLocalVideo()
{
    if (pVideoMediaControl_->Stop() < 0)
        return false;
    else
        return true;
}

// �������ű�����Ƶ
bool DShowCapture::ContinuePlayingLocalVideo()
{
    if (pVideoMediaControl_->Run() < 0)
        return false;
    else
        return true;
}
