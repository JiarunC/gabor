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
    std::vector<float> fftRes;
    std::vector<std::vector<float>> yx;
    Adapter mAdapter = Adapter(std::cref(yx));

    QFutureWatcher<void> watcher;

    std::vector<std::vector<int>> mapping;
    std::vector<float> tilts;

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

        // log binning
        using namespace std;
        constexpr auto nInitialNewBins = 255.0f;
        const auto nBins = mResolution/2;

        // holds fft resolutions must be N/2+1 (+1 is for 0)
        fftRes = vector<float>(nBins+1, 0);

        // holds tilt mapping
        tilts = std::vector<float>(nBins+1,0);

        // holds the x/y-axis for qt graphs
        yx = std::vector<std::vector<float>>(2);
        // for x coordinates - parallel with mapping
        yx[1] = std::vector<float>();

        const auto base = static_cast<float>(nBins);
        float newBinIndex = 1.0f;
        auto newBinRightBoundary = pow(base, newBinIndex/nInitialNewBins);
        float lastBin = 0.0f;
        // log binning does not contain 0 from fft
        // no access through index to avoid 1-0 indexing problems
        for (auto i = 1; i<=nBins; i++) {
            qDebug() << newBinRightBoundary;
            if (i <= newBinRightBoundary) {
                if (lastBin != newBinIndex) {
                    // create new bin
                    if (!yx[1].empty()) {
                        yx[1].back() -= .5;
                    }
                    yx[1].push_back(static_cast<float>(newBinIndex));
                    mapping.emplace_back(1, i);
                } else {
                    // bin already exists
                    mapping.back().push_back(i);
                }
                lastBin = newBinIndex;
            } else {
                newBinIndex++;
                newBinRightBoundary = pow(base, newBinIndex/nInitialNewBins);
                yx[1].back() += .5;
                i--;
            }
        }
        // for magnitudes
        yx[0] = std::vector<float>(yx[1].size());

        qDebug() << mapping;
        qDebug() << yx[1];

        // GUI thread
        timer = new QChronoTimer(std::chrono::nanoseconds(100000000), this);
        connect(timer, &QChronoTimer::timeout, this, &AudioCapture::updateSeries);
        timer->start();

        // separate thread
        connect(&watcher, &QFutureWatcher<void>::finished, this, &AudioCapture::uninitialize);
        QFuture<void> future = QtConcurrent::run(&AudioCapture::captureAudio, this);
        watcher.setFuture(future);
    }

    void uninitialize() {
        emit uninitialized();
    }

    void initializeTilt() {
        for (int k = 1; k < tilts.size(); ++k) {
            constexpr float pivot = 1000.0f;
            constexpr float slope = 3.0f;
            const float freq = k * (pwfx->nSamplesPerSec / static_cast<float>(mResolution));
            tilts[k] = slope * std::log2(freq / pivot);
        }
    }

    [[nodiscard]] QAbstractItemModel* model() const {
        return mAdapter.model();
    }

    void updateSeries() {

        QMutexLocker locker(&mutex);
        fftp->AFFT(pool, fftRes);

        // apply tilt
        for (auto i=1;i<fftRes.size();i++) {
            fftRes[i] += tilts[i];
        }

        // map to log bins
        for (auto i=0; i<yx[0].size();i++) {
            float mx = 0;
            for (const auto& largeBinIndex : mapping[i]) {
                mx = std::max(mx, fftRes[largeBinIndex]);
            }
            yx[0][i] = mx;
        }
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

        // pwfx initialized
        initializeTilt();

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
