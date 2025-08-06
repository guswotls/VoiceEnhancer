#ifndef SIGNAL_GENERATOR_H
#define SIGNAL_GENERATOR_H

#include <math.h>
#define PI 3.14159265358979323846f

class SignalGenerator {
public:
    // 생성자: sampleRate와 frequency를 받아,
    // 첫 1초 무음, 1.5초 사인파, 마지막 1초 무음 샘플 수 계산
    SignalGenerator(int sampleRate, float frequency)
            : mSampleRate(sampleRate),
              mFrequency(frequency),
              mPhase(0.0f),
              mSampleCounter(0)
    {
        mSilentSamplesStart = sampleRate * 1;          // 첫 1초 무음
        mSignalSamples      = static_cast<int>(sampleRate * 1.5f); // 1.5초 사인파
        mSilentSamplesEnd   = sampleRate * 1;          // 마지막 1초 무음

        mTotalSamples = mSignalSamples;
    }

    ~SignalGenerator() {}

    float generate() {
        float sample = 0.0f;

        if (mSampleCounter < mSilentSamplesStart) {
            // 첫 1초 무음
            sample = 0.0f;
        }
        else if (mSampleCounter < mSilentSamplesStart + mSignalSamples) {
            // 1.5초 동안 사인파
            sample = sinf(mPhase * 2.0f * PI);
            mPhase += (mFrequency / mSampleRate);
            if (mPhase >= 1.0f) mPhase -= 1.0f;
        }
        else if (mSampleCounter < mTotalSamples) {
            // 마지막 1초 무음
            sample = 0.0f;
        }
        else {
            // 그 이후에도 계속 무음
            sample = 0.0f;
        }

        mSampleCounter++;
        return sample;
    }

    // 총 샘플 수 조회 (필요 시)
    int getTotalSamples() const {
        return mTotalSamples;
    }

private:
    int   mSampleRate;         // 샘플링 레이트 (Hz)
    float mFrequency;          // 사인파 주파수 (Hz)
    float mPhase;              // 현재 위상 (0.0 ~ 1.0)
    int   mSampleCounter;      // 지금까지 생성된 샘플 수

    int mSilentSamplesStart;   // 첫 무음 구간 샘플 수 (1초)
    int mSignalSamples;        // 사인파 구간 샘플 수 (1.5초)
    int mSilentSamplesEnd;     // 마지막 무음 구간 샘플 수 (1초)
    int mTotalSamples;         // 전체 샘플 수
};

#endif // SIGNAL_GENERATOR_H



//#ifndef SIGNAL_GENERATOR_H
//#define SIGNAL_GENERATOR_H
//
//#include <math.h>
//#define PI 3.14159265358979323846f
//
//class SignalGenerator {
//public:
//    // 생성자: sampleRate와 frequency를 받아,
//    // 첫 1초 무음, 사인파 10주기, 마지막 1초 무음을 위한 샘플 수를 계산
//    SignalGenerator(int sampleRate, float frequency)
//            : mSampleRate(sampleRate),
//              mFrequency(frequency),
//              mPhase(0.0f),
//              mSampleCounter(0)
//
//    {
//        mSilentSamplesStart = sampleRate * 1; // 첫 1초 무음
//        // 사인파 한 주기의 샘플 수 (정확하지 않은 경우가 있을 수 있으므로 정수로 변환)
//        int periodSamples = static_cast<int>(static_cast<float>(sampleRate) / mFrequency);
//        mSineSamples = periodSamples * 10;  // 사인파 10주기 출력
//        mSilentSamplesEnd = sampleRate * 1; // 마지막 1초 무음
//
//        mTotalSamples = mSilentSamplesStart + mSineSamples + mSilentSamplesEnd;
//    }
//
//    ~SignalGenerator() {}
//
//    // 매번 호출 시 현재 위치에 따라 무음 또는 사인파 샘플을 반환
//    float generate() {
//        float sample = 0.0f;
//        // 첫 1초 무음 구간
//        if (mSampleCounter < mSilentSamplesStart) {
//            sample = 0.0f;
//        }
//            // 사인파 생성 구간 (10주기)
//        else if (mSampleCounter < mSilentSamplesStart + mSineSamples) {
//            sample = sinf(mPhase * 2.0f * PI);
//            mPhase += (mFrequency / mSampleRate);
//            if (mPhase >= 1.0f)
//                mPhase -= 1.0f;
//        }
//            // 마지막 1초 무음 구간
//        else if (mSampleCounter < mTotalSamples) {
//            sample = 0.0f;
//        }
//            // 전체 구간이 지난 후에는 계속 무음 반환
//        else {
//            sample = 0.0f;
//        }
//        mSampleCounter++;
//        return sample;
//    }
//
//private:
//    int mSampleRate;
//    float mFrequency;
//    float mPhase;         // 사인파의 현재 위상 (0 ~ 1)
//    int mSampleCounter;   // 지금까지 호출된 샘플 수
//
//    // 각 구간에 해당하는 샘플 수
//    int mSilentSamplesStart;  // 첫 무음 구간 (1초)
//    int mSineSamples;         // 사인파 구간 (10주기)
//    int mSilentSamplesEnd;    // 마지막 무음 구간 (1초)
//    int mTotalSamples;        // 전체 샘플 수
//};
//
//#endif // SIGNAL_GENERATOR_H
