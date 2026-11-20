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
#include <QChronoTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QFutureWatcher>

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
    Q_PROPERTY(unsigned resolution WRITE set_resolution)
    Q_PROPERTY(QAbstractItemModel* model READ model CONSTANT)
private:
    bool mStatus = true;
    unsigned mResolution;
    bool mInitialized = false;

    QChronoTimer* timer = nullptr;
    FFTProcess* fftp = nullptr;

    QMutex mutex;
    std::vector<float> pool;
    std::vector<std::vector<float>> yx;
    Adapter mAdapter = Adapter(std::cref(yx));

    QFutureWatcher<void> watcher;

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
    void uninitialized();
public:

    explicit AudioCapture(QObject *parent = nullptr) : QObject(parent) {}

    ~AudioCapture() override {
        exit(true);
    }

    template<class T>
    static void safe_release(T *&punk) {
        if (punk != nullptr) {
            punk->Release();
            punk = nullptr;
        }
    }

    void exit(bool destructor = false) {
        // handles null

        CoTaskMemFree(pwfx);
        pwfx = nullptr;
        safe_release(pEnumerator);
        safe_release(pDevice);
        safe_release(pAudioClient);
        safe_release(pCaptureClient);

        // does not handle null
        if (!handle) {
            CloseHandle(handle);
        }

        // only on thread that initialized
        if (!destructor) {
            CoUninitialize();
        }
    }

    void set_status(bool status) {
        mStatus = status;
    }

    void set_resolution(const unsigned resolution) {
        qDebug() << resolution;
        assert(std::has_single_bit(resolution) && resolution>=1024);
        mResolution = resolution;
        if (!mInitialized) {
            initialize();
            mInitialized = true;
        }
    }

    void initialize() {
        fftp = new FFTProcess(this, mResolution);
        pool = std::vector<float>(mResolution, 0);

        // GUI thread
        timer = new QChronoTimer(std::chrono::nanoseconds(30000000), this);
        connect(timer, &QChronoTimer::timeout, this, &AudioCapture::updateSeries);
        timer->start();

        // separate thread
        connect(&watcher, &QFutureWatcher<void>::finished, this, &AudioCapture::uninitialize);
        QFuture<void> future = QtConcurrent::run(&AudioCapture::captureAudio, this);
        watcher.setFuture(future);
    }

    HRESULT establishSimplification() {
        auto outputSize = mResolution/2+1;
        yx = std::vector<std::vector<float>>(3);
        yx[0] = std::vector<float>(outputSize, 0);
        yx[1] = std::vector<float>(outputSize, 0);
        float counter = 1;
        for (auto &i : yx[1]) {
            i = counter;
            counter++;
        }

        auto sr = pwfx->nSamplesPerSec;
        auto cutOff = 4500 / (sr/mResolution);
        assert(cutOff < 250);
        if (!cutOff%2) {
            cutOff++;
        }

        auto index = cutOff;
        auto skips = 1;
        while (index<yx[1].size()) {
            index++;
            if (index >= yx[1].size()) {
                break;
            }
            if (index+skips >= yx[1].size()) {
                yx[1].erase(yx[1].begin()+index,yx[1].end()-1);
                break;
            }
            yx[1].erase(yx[1].begin()+index,yx[1].begin()+index+skips);
        }


        return S_OK;
    }

    void uninitialize() {
        emit uninitialized();
    }

    [[nodiscard]] QAbstractItemModel* model() const {
        return mAdapter.model();
    }

    void updateSeries() {
        QMutexLocker locker(&mutex);
        fftp->AFFT(pool, yx[0]);

        // readjust db level
        //
        // my design to keep lower frequencies more detailed
        // max of 300 points to support good spline performance

        // sample rate / resolution = bin

        // resolution/2 + 1, current
        // is even
        // keep first 4500/(sr/resolution) make even
        // rest is even
        // 1, rid 1
        // 3, rid 2 ...


        mAdapter.assign(std::cref(yx));
    }

    HRESULT processData() {

        QMutexLocker locker(&mutex);
        // only works for current device
        if (pwfx->wBitsPerSample!=32) {
            return E_FAIL;
        }
        const auto nChannels = pwfx->nChannels;
        const auto numBytes = numFramesAvailable*pwfx->nBlockAlign;
        const auto numSamples = numBytes / (pwfx->wBitsPerSample / 8);
        const auto numCombinedSamples = numSamples / nChannels;

        // in buffer > held buffer
        if (numCombinedSamples >= mResolution) {
            std::vector<float> tmp(nChannels*mResolution);
            memcpy(tmp.data(), pData+numSamples-nChannels*mResolution, nChannels*mResolution*sizeof(float));
            for (auto i=0;i<mResolution;i++) {
                float sum = 0;
                for (auto j=0;j<nChannels;j++) {
                    sum += tmp[nChannels*i+j];
                }
                pool[i] = sum/nChannels;
            }
            return S_OK;
        }

        std::move(pool.begin() + numCombinedSamples, pool.end(), pool.begin());
        if (nChannels > 1) {
            std::vector<float> tmp(numSamples);
            memcpy(tmp.data(), pData, numBytes);
            for (auto i=0;i<numCombinedSamples;i++) {
                float sum = 0;
                for (auto j=0;j<nChannels;j++) {
                    sum += tmp[nChannels*i+j];
                }
                pool[pool.size()-numCombinedSamples+i] = sum/nChannels;
            }
        } else {
            memcpy(pool.data() + pool.size() - numCombinedSamples, pData, numBytes);
        }

        return S_OK;
    }

    [[nodiscard]] HRESULT captureAudio() {
        HRESULT hr;

        // never uninitialized
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

        hr = establishSimplification();
        if (FAILED(hr)) {
            exit();
            return hr;
        }

        while (mStatus) {
            DWORD waitResult;

            waitResult = WaitForSingleObject(
                handle,
                200);

            // on timeout restart wait
            // so that when mStatus changes
            // the function returns
            if (waitResult == WAIT_TIMEOUT) {
                continue;
            }

            // other errors
            if (waitResult != WAIT_OBJECT_0) {
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
