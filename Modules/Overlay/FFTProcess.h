#pragma once

#include <QDebug>
#include <QObject>
#include "mkl.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <numbers>
#include <vector>
#include "mkl_service.h"
#include "mkl_dfti.h"


class FFTProcess : public QObject {
    Q_OBJECT

private:
    const MKL_LONG N = 1024;
    std::vector<float>* weights;
    DFTI_DESCRIPTOR_HANDLE handle = nullptr;
public:

    explicit FFTProcess(QObject *parent = nullptr) : QObject(parent) {
        weights = new std::vector<float>(N);
        for (auto i=0;i<N;i++) {
            weights->at(i) = 0.5 - 0.5 * std::cos((2* std::numbers::pi*i)/(N-1));
        }
        qDebug() << "weights";
    }

    ~FFTProcess() override {
        cleanUp();
    }

    [[nodiscard]] MKL_LONG establishHandle() {
        MKL_LONG status = 0;

        status = DftiCreateDescriptor(&handle, DFTI_SINGLE, DFTI_REAL, 1, N);
        if (status != DFTI_NO_ERROR) {
            return status;
        }
        status = DftiSetValue(handle, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
        if (status != DFTI_NO_ERROR) {
            return status;
        }
        status = DftiCommitDescriptor(handle);
        if (status != DFTI_NO_ERROR) {
            return status;
        }
        return DFTI_NO_ERROR;
    }



    int AFFT(std::vector<float> input, std::vector<float>& output) {

        MKL_LONG status = 0;

        if (true) {
            status = establishHandle();
            if (status != DFTI_NO_ERROR) {
                cleanUp();
                return 1;
            }
        }

        assert(input.size() == N && output.size()==N/2+1);

        // window
        for (auto i=0;i<N;i++) {
            input[i] *= weights->at(i);
        }

        // real
        float *pInput = input.data();

        // complex
        MKL_Complex8 *pOutputRaw = nullptr;
        pOutputRaw = static_cast<MKL_Complex8 *>(mkl_malloc((N / 2 + 1) * sizeof(MKL_Complex8), 64));
        if (pInput == nullptr || pOutputRaw == nullptr) {
            cleanUp(pOutputRaw);
            return 1;
        }
        status = DftiComputeForward(handle, pInput, pOutputRaw);
        if (status != DFTI_NO_ERROR) {
            cleanUp(pOutputRaw);
            return 1;
        }
        // https://stackoverflow.com/questions/4364823/how-do-i-obtain-the-frequencies-of-each-value-in-an-fft
        const auto numBins = N / 2 + 1;

        for (auto i=0; i<numBins;i++) {
            auto re = pOutputRaw[i].real;
            auto im = pOutputRaw[i].imag;
            output[i] = std::sqrt(re * re + im * im);
        }

        mkl_free(pOutputRaw);
        return 0;
    }

    void cleanUp(MKL_Complex8* pOutputRaw = nullptr) {
        if (handle) {
            DftiFreeDescriptor(&handle);
            handle = nullptr;
        }
        if (pOutputRaw) {
            mkl_free(pOutputRaw);
            pOutputRaw = nullptr;
        }
    }
};