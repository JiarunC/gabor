#pragma once
#include <QObject>
#include <QQmlEngine>
#include <windows.h>
#include <audioclient.h>
#include <QByteArray>
#include <vector>

using namespace std;
class AudioProcess : public QObject {
    Q_OBJECT
    QML_ELEMENT

private:
    vector<short> pool = vector<short>(2048, 0);

public:
    explicit AudioProcess(QObject *parent = nullptr) : QObject(parent) {}

    HRESULT processData(BYTE *pData, UINT32 frames, WAVEFORMATEX* descriptor) {
        auto numBytes = frames*descriptor->nBlockAlign;
        vector<int16_t> samples(numBytes/2);
        memcpy(samples.data(), pData, numBytes/2);
        vector<short> tmp = vector<short>(2048, 0);
        copy(pool.begin(),pool.end()-numBytes/2,tmp.begin());
        copy(samples.begin(),samples.end(),tmp.end()-numBytes/2);
        pool = tmp;

        return S_OK;
    }

};