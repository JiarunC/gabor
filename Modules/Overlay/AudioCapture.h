#pragma once

#define NOMINMAX

#include <QObject>
#include <QQmlEngine>
#include <QDebug>
#include <windows.h>
#include <audioclient.h>
#include <iostream>
#include <mmdeviceapi.h>
#include <QtConcurrentRun>
#include <QFuture>
#include <bit>
#include <QThread>
#include <QRangeModel>
#include <QRangeModelAdapter>
#include "FFTProcess.h"
#include <algorithm>
#include <chrono>

// https://learn.microsoft.com/en-us/windows/win32/coreaudio/capturing-a-stream

inline const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
inline const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
inline const IID IID_IAudioClient = __uuidof(IAudioClient);
inline const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

class AudioCapture : public QObject {
    Q_OBJECT
    QML_ELEMENT

    using Adapter = decltype(QRangeModelAdapter(std::vector<std::vector<float>>{}));

    Q_PROPERTY(bool status WRITE set_status)
    Q_PROPERTY(QAbstractItemModel* model READ model CONSTANT)
private:
    bool mStatus = true;
    unsigned mResolution;

    // data race (but it's fine)
    std::vector<float> pool;


    std::vector<std::vector<float>> yx;
    Adapter mAdapter = Adapter(std::cref(yx));

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
        false,
        false,
        nullptr
    );
public:
    signals:
public:

    explicit AudioCapture(const unsigned resolution = 1024, QObject *parent = nullptr) : QObject(parent), mResolution(resolution) {
        assert(resolution >= 1024 && std::has_single_bit(resolution));

        pool = std::vector<float>(resolution, 0);
        yx = std::vector<std::vector<float>>(2);
        yx[0] = std::vector<float>(resolution/2+1, .5);
        yx[1] = std::vector<float>(resolution/2+1, 0);
        float counter = 1;
        for (auto &i : yx[1]) {
            i = counter;
            counter++;
        }

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

        CloseHandle(handle);
    }

    void set_status(bool status) {
        mStatus = status;
    }

    [[nodiscard]] QAbstractItemModel* model() const {
        return mAdapter.model();
    }

    void updateSeries() {
        while (true) {
            QThread::sleep(1);
            auto start = std::chrono::high_resolution_clock::now();
            FFTProcess::AFFT(pool, yx[0]);

            auto stop = std::chrono::high_resolution_clock::now();

            auto duration =
                duration_cast<std::chrono::microseconds>(
                    stop - start);

            qDebug() << duration << "micro";

            mAdapter.assign(std::cref(yx));
        }
    }

    HRESULT processData() {
        // only works for current device
        // does not consider Buffer overflow
        if (pwfx->wBitsPerSample!=32) {
            return E_FAIL;
        }
        const auto numBytes = numFramesAvailable*pwfx->nBlockAlign;
        const auto numSamples = numBytes / (pwfx->wBitsPerSample / 8);
        const auto numCombinedSamples = numSamples / pwfx->nChannels;
        std::move(pool.begin() + numCombinedSamples, pool.end(), pool.begin());
        if (pwfx->nChannels == 2) {
            std::vector<float> tmp(numSamples);
            memcpy(tmp.data(), pData, numBytes);
            for (auto i=0;i<numCombinedSamples;i++) {
                pool[pool.size()-numCombinedSamples+i] = (tmp[2*i] + tmp[2*i+1])/2;
            }
        } else {
            assert(numSamples == numCombinedSamples);
            memcpy(pool.data() + pool.size() - numCombinedSamples, pData, numBytes);
        }

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
        if (FAILED(hr)) {
            exit();
            return hr;
        }

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
