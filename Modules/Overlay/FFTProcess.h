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
public:
    static int AFFT(std::vector<float> input, std::vector<float>& output) {

        const MKL_LONG N = input.size();

        // window
        for (auto i=0;i<input.size();i++) {
            input[i] *= 0.5 - 0.5 * std::cos((2* std::numbers::pi*i)/(N-1));
        }

        assert(output.size()==N/2+1);

        MKL_LONG status = 0;
        // real
        float *pInput = input.data();

        // complex
        MKL_Complex8 *pOutputRaw = nullptr;

        DFTI_DESCRIPTOR_HANDLE handle = nullptr;

        status = DftiCreateDescriptor(&handle, DFTI_SINGLE, DFTI_REAL, 1, N);
        if (status != DFTI_NO_ERROR) {
            cleanUp(handle, pOutputRaw);
            return 1;
        }
        status = DftiSetValue(handle, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
        if (status != DFTI_NO_ERROR) {
            cleanUp(handle, pOutputRaw);
            return 1;
        }
        status = DftiCommitDescriptor(handle);
        if (status != DFTI_NO_ERROR) {
            cleanUp(handle, pOutputRaw);
            return 1;
        }
        pOutputRaw = static_cast<MKL_Complex8 *>(mkl_malloc((N / 2 + 1) * sizeof(MKL_Complex8), 64));
        if (pInput == nullptr || pOutputRaw == nullptr) {
            cleanUp(handle, pOutputRaw);
            return 1;
        }
        status = DftiComputeForward(handle, pInput, pOutputRaw);
        if (status != DFTI_NO_ERROR) {
            cleanUp(handle, pOutputRaw);
            return 1;
        }
        // https://stackoverflow.com/questions/4364823/how-do-i-obtain-the-frequencies-of-each-value-in-an-fft
        const auto numBins = N / 2 + 1;

        for (auto i=0; i<numBins;i++) {
            auto re = pOutputRaw[i].real;
            auto im = pOutputRaw[i].imag;
            output[i] = std::sqrt(re * re + im * im);
        }
        cleanUp(handle, pOutputRaw);
        return 0;
    }

    static void cleanUp(DFTI_DESCRIPTOR_HANDLE handle, MKL_Complex8* pOutputRaw) {
        if (handle)
            DftiFreeDescriptor(&handle);
        if (pOutputRaw)
            mkl_free(pOutputRaw);
    }
};