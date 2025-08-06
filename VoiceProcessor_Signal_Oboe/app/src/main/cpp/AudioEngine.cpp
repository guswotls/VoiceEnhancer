#include "AudioEngine.h"
#include "AudioConfig.h"  // 매크로 포함
#include <android/log.h>
#include <cmath>
#include <cstring>
#include <algorithm>      // std::clamp

#define LOG_TAG "AudioEngine"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)

AudioEngine::AudioEngine()
        : mAmplificationMode(false),
          mSignalGenerator(SAMPLE_RATE, 500.0f),
          mSampleRate(SAMPLE_RATE),
          mBufferSize(BUFFER_SIZE),
          mTotalFrames(SAMPLE_RATE * 10)
{
    mLeftBuffer.reserve(mTotalFrames);
    mRightBuffer.reserve(mTotalFrames);
}

AudioEngine::~AudioEngine() {
    stop();
}

oboe::Result AudioEngine::init(bool amplificationMode, int32_t sampleRate, int32_t bufferSize) {
    // AudioConfig.h에 정의된 값 사용
    mAmplificationMode = amplificationMode;
    mSampleRate       = SAMPLE_RATE;
    mBufferSize       = BUFFER_SIZE;
    mTotalFrames      = SAMPLE_RATE * 10;
    mLeftBuffer.clear();
    mRightBuffer.clear();
    mSignalGenerator = SignalGenerator(SAMPLE_RATE, 1500.0f);

    // 출력 스트림 생성 (스테레오, I16 PCM) – 콜백에서 500Hz 톤 생성
    oboe::AudioStreamBuilder outputBuilder;
    outputBuilder.setDirection(oboe::Direction::Output)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setSampleRate(SAMPLE_RATE)
            ->setChannelCount(1) // ← 수정: 2채널로 변경
            ->setFormat(oboe::AudioFormat::I16)
            ->setCallback(this);
    ManagedStream managedOutput;
    oboe::Result result = outputBuilder.openManagedStream(managedOutput);
    if (result != oboe::Result::OK) {
        LOGE("Failed to open output stream: %s", oboe::convertToText(result));
        return result;
    }
    mOutputStream = std::move(managedOutput);

    // 입력 스트림 생성 (모노, I16 PCM), 마이크 입력 콜백으로 수정함
    oboe::AudioStreamBuilder inputBuilder;
    inputBuilder.setDirection(oboe::Direction::Input)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setSampleRate(SAMPLE_RATE)
            ->setChannelCount(1)
            ->setFormat(oboe::AudioFormat::I16)
            // 콜백 사용하여 마이크 입력 받을 시, 내부 버퍼 사이즈를 코드에서 지정하면 하드웨어 burst 크기의 배수로 올림되어 사용되어
            // 지연시간 증가 발생
            //->setBufferCapacityInFrames(mBufferSize)   // 내부 버퍼 총 크기 지정
            //->setFramesPerCallback(mBufferSize) // 한 번에 읽을 샘플 수 지정

            ->setCallback(this);
    ManagedStream managedInput;
    result = inputBuilder.openManagedStream(managedInput);
    if (result != oboe::Result::OK) {
        LOGE("Failed to open input stream: %s", oboe::convertToText(result));
        return result;
    }
    mInputStream = std::move(managedInput);

    // 실제 하드웨어 샘플레이트 재확인
    mSampleRate  = mInputStream->getSampleRate();
    mTotalFrames = mSampleRate * 10;
    mSignalGenerator = SignalGenerator(mSampleRate, 1500.0f);

    LOGI("CURRENT USE SAMPLERATE : %d", mSampleRate);
    LOGI("TOTAL SAMPLES (mTotalFrames) : %d", mTotalFrames);
    return oboe::Result::OK;
}

oboe::Result AudioEngine::start() {
    mLeftBuffer.clear();
    mRightBuffer.clear();

    oboe::Result result = mOutputStream->requestStart();
    if (result != oboe::Result::OK) {
        LOGE("Failed to start output stream: %s", oboe::convertToText(result));
        return result;
    }

    result = mInputStream->requestStart();
    if (result != oboe::Result::OK) {
        LOGE("Failed to start input stream: %s", oboe::convertToText(result));
        return result;
    }

    return oboe::Result::OK;
}

oboe::Result AudioEngine::stop() {
    if (mInputStream) {
        mInputStream->requestStop();
        mInputStream->close();
        mInputStream.reset();
    }
    if (mOutputStream) {
        mOutputStream->requestStop();
        mOutputStream->close();
        mOutputStream.reset();
    }
    return oboe::Result::OK;
}

oboe::DataCallbackResult AudioEngine::onAudioReady(oboe::AudioStream *oboeStream,
                                                   void *audioData,
                                                   int32_t numFrames) {
    // 출력 스트림: 톤 생성 및 mLeftBuffer에 저장
    if (oboeStream == mOutputStream.get()) {
        int16_t* outputBuffer = static_cast<int16_t*>(audioData);
        for (int i = 0; i < numFrames; i++) {
            float   sample     = mSignalGenerator.generate();
            int16_t toneSample = static_cast<int16_t>(sample * 32767);
            outputBuffer[i * 2]   = toneSample;  // 좌
            outputBuffer[i * 2+1] = 0;           // 우
            if (mLeftBuffer.size() < static_cast<size_t>(mTotalFrames)) {
                mLeftBuffer.push_back(toneSample);
            }
        }
    }
        // 입력 스트림: 마이크 입력을 I16 그대로 읽어 mRightBuffer에 저장
    else if (oboeStream == mInputStream.get()) {
        int16_t* inputBuffer = static_cast<int16_t*>(audioData);
        for (int i = 0; i < numFrames; i++) {
            int16_t intSample = inputBuffer[i];     // ← 수정: 이미 I16 PCM이므로 그대로 사용

            if (mAmplificationMode) {
                int amplified = intSample * 2;
                intSample = static_cast<int16_t>(
                        std::clamp(amplified, -32768, 32767)
                );
            }
            if (mRightBuffer.size() < static_cast<size_t>(mTotalFrames)) {
                mRightBuffer.push_back(intSample);
            }
        }
    }
    return oboe::DataCallbackResult::Continue;
}

std::vector<int16_t> AudioEngine::getRecordedData() {
    size_t frames = std::min(mLeftBuffer.size(), mRightBuffer.size());
    std::vector<int16_t> stereoData;
    stereoData.reserve(frames * 2);
    for (size_t i = 0; i < frames; i++) {
        stereoData.push_back(mLeftBuffer[i]);
        stereoData.push_back(mRightBuffer[i]);
    }
    return stereoData;
}
