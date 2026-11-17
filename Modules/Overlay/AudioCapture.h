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
#include "AudioProcess.h"
#include <QThread>

// https://learn.microsoft.com/en-us/windows/win32/coreaudio/capturing-a-stream

inline const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
inline const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
inline const IID IID_IAudioClient = __uuidof(IAudioClient);
inline const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

class AudioCapture : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool status WRITE set_status)
    Q_PROPERTY(QList<float> gY READ gY NOTIFY dataChanged)
    Q_PROPERTY(QList<float> gX READ gX NOTIFY dataChanged)
private:
    bool mStatus = true;
    QList<float> mYL = QList<float>(3,1);
    QList<float> mXL = {1,2,3};
    vector<float> pool = vector<float>(2048, 0);

    /* Used by the capture thread */
    signed long long ref_times_per_s = 10000000;
    REFERENCE_TIME hnsRequestedDuration = 10000000;
    REFERENCE_TIME hnsActualDuration;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    IMMDeviceEnumerator *pEnumerator = nullptr;
    IMMDevice *pDevice = nullptr;
    IAudioClient *pAudioClient = nullptr;
    IAudioCaptureClient *pCaptureClient = nullptr;
    WAVEFORMATEX *pwfx = nullptr;
    UINT32 packetLength = 0;
    BOOL bDone = FALSE;
    BYTE *pData;
    DWORD flags;
    HANDLE handle = CreateEvent(
        nullptr,
        true,
        false,
        TEXT("AudioCallback")
    );
public:
    signals:
    void dataChanged();
public:

    explicit AudioCapture(QObject *parent = nullptr) : QObject(parent) {
        QFuture<void> future1 = QtConcurrent::run(&AudioCapture::captureAudio, this);
        QFuture<void> future2 = QtConcurrent::run(&AudioCapture::updateSeries, this);
    }

    ~AudioCapture() override {
        exit();
    }

    template<class T>
    static void safe_release(T *&punk) {
        if (punk != nullptr) {
            punk->Release();
            punk = nullptr;
        }
    }

    void exit() {
        CoTaskMemFree(pwfx);
        safe_release(pEnumerator);
        safe_release(pDevice);
        safe_release(pAudioClient);
        safe_release(pCaptureClient);
    }

    QList<float> gY() {
        return mYL;
    }

    QList<float> gX() {
        return mXL;
    }

    void set_status(bool status) {
        mStatus = status;
    }

    void updateSeries() {
        while (true) {
            QThread::sleep(1);
            qDebug() << "updating";

            for (int i=0;i<3;i++) {
                mYL[i] = pool[i];
            }

            emit dataChanged();
        }
    }

    HRESULT processData() {
        if (pwfx->wBitsPerSample!=32) {
            return E_FAIL;
        }
        const auto numBytes = numFramesAvailable*pwfx->nBlockAlign;
        const auto numSamples = numBytes / (pwfx->wBitsPerSample / 8);
        std::move(pool.begin() + numSamples, pool.end(), pool.begin());
        memcpy(pool.data() + pool.size() - numSamples, pData, numBytes);

        return S_OK;
    }

    [[nodiscard]] HRESULT captureAudio() {
        HRESULT hr;
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
                    hr = processData();
                    if (FAILED(hr)) {
                        exit();
                        return hr;
                    }
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
