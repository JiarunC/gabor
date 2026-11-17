#pragma once
#include <QObject>
#include <QQmlEngine>
#include <QDebug>
#include <windows.h>
#include <audioclient.h>
#include <iostream>
#include <mmdeviceapi.h>
#include <QtConcurrentRun>
#include <QFuture>
// https://learn.microsoft.com/en-us/windows/win32/coreaudio/capturing-a-stream

static signed long long ref_times_per_s = 10000000;

static HRESULT hr;
static REFERENCE_TIME hnsRequestedDuration = ref_times_per_s;
static REFERENCE_TIME hnsActualDuration;
static UINT32 bufferFrameCount;
static UINT32 numFramesAvailable;
static IMMDeviceEnumerator *pEnumerator = nullptr;
static IMMDevice *pDevice = nullptr;
static IAudioClient *pAudioClient = nullptr;
static IAudioCaptureClient *pCaptureClient = nullptr;
static WAVEFORMATEX *pwfx = nullptr;
static UINT32 packetLength = 0;
static BOOL bDone = FALSE;
static BYTE *pData;
static DWORD flags;
static HANDLE handle = CreateEvent(
    nullptr,
    true,
    false,
    TEXT("AudioCallback")
);

inline const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
inline const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
inline const IID IID_IAudioClient = __uuidof(IAudioClient);
inline const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

template<class T>
static void safe_release(T *&punk) {
    if (punk != nullptr) {
        punk->Release();
        punk = nullptr;
    }
}

static void exit() {
    CoTaskMemFree(pwfx);
    safe_release(pEnumerator);
    safe_release(pDevice);
    safe_release(pAudioClient);
    safe_release(pCaptureClient);
}

class AudioCapture : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool status WRITE set_status)
private:
    bool m_status = true;
public:
    explicit AudioCapture(QObject *parent = nullptr) : QObject(parent) {
        qDebug() << "Gonna";
        QFuture<void> future = QtConcurrent::run(&AudioCapture::RecordAudioStream, this);
    }

    void set_status(bool status) {
        m_status = status;
    }
    HRESULT RecordAudioStream() const {
        qDebug() << "Running";
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(hr)) {
            exit();
            return hr;
        }

        hr = CoCreateInstance(
            CLSID_MMDeviceEnumerator, NULL,
            CLSCTX_ALL, IID_IMMDeviceEnumerator,
            reinterpret_cast<void **>(&pEnumerator));
        if (FAILED(hr)) {
            exit();
            return hr;
        }

        hr = pEnumerator->GetDefaultAudioEndpoint(
            eRender, eConsole, &pDevice);
        if (FAILED(hr)) {
            exit();
            return hr;
        }

        hr = pDevice->Activate(
            IID_IAudioClient,
            CLSCTX_ALL,
            nullptr,
            reinterpret_cast<void **>(&pAudioClient));
        if (FAILED(hr)) {
            exit();
            return hr;
        }

        hr = pAudioClient->GetMixFormat(&pwfx);
        if (FAILED(hr)) {
            exit();
            return hr;
        }


        hr = pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_EVENTCALLBACK |
            AUDCLNT_STREAMFLAGS_LOOPBACK,
            hnsRequestedDuration,
            0,
            pwfx,
            nullptr);
        if (FAILED(hr)) {
            exit();
            return hr;
        }

        hr = pAudioClient->SetEventHandle(handle);

        // Get the size of the allocated buffer.
        hr = pAudioClient->GetBufferSize(&bufferFrameCount);
        if (FAILED(hr)) {
            exit();
            return hr;
        }

        hr = pAudioClient->GetService(
            IID_IAudioCaptureClient,
            reinterpret_cast<void **>(&pCaptureClient));
        if (FAILED(hr)) {
            exit();
            return hr;
        }

        // Notify the audio sink which format to use.
        //hr = pMySink->SetFormat(pwfx);
        //if (FAILED(hr)) {
        //    onFail();
        //    return hr;
        //}

        // Calculate the actual duration of the allocated buffer.
        hnsActualDuration = static_cast<double>(ref_times_per_s) *
                            bufferFrameCount / pwfx->nSamplesPerSec;

        hr = pAudioClient->Start(); // Start recording.
        if (FAILED(hr)) {
            exit();
            return hr;
        }

        while (true) {
            DWORD waitResult;

            waitResult = WaitForSingleObject(
                handle,
                INFINITE);

            if (waitResult != WAIT_OBJECT_0) {
                std::cout << (GetLastError());
                exit();
                return 0;
            }

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
            if (FAILED(hr)) {
                exit();
                return hr;
            }

            while (packetLength != 0) {
                // Get the available data in the shared buffer.
                hr = pCaptureClient->GetBuffer(
                    &pData,
                    &numFramesAvailable,
                    &flags, NULL, NULL);
                if (FAILED(hr)) {
                    exit();
                    return hr;
                }

                if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                    pData = NULL; // Tell CopyData to write silence.
                } else {
                    qDebug() << numFramesAvailable;
                }

                // Copy the available capture data to the audio sink.

                if (FAILED(hr)) {
                    exit();
                    return hr;
                }

                hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
                if (FAILED(hr)) {
                    exit();
                    return hr;
                }

                hr = pCaptureClient->GetNextPacketSize(&packetLength);
                if (FAILED(hr)) {
                    exit();
                    return hr;
                }
            }
        }

        hr = pAudioClient->Stop(); // Stop recording.
        if (FAILED(hr)) {
            exit();
            return hr;
        }

        exit();
        return hr;
    }

};
