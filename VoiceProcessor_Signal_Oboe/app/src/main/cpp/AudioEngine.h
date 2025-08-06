#pragma once
#include "AudioConfig.h"   // 매크로 포함
#include <oboe/Oboe.h>
#include <vector>
#include <atomic>
#include <thread>
#include "SignalGenerator.h"

// Oboe 샘플에서는 ManagedStream을 다음과 같이 typedef 합니다.
using ManagedStream = std::unique_ptr<oboe::AudioStream, oboe::StreamDeleterFunctor>;

class AudioEngine : public oboe::AudioStreamCallback {
public:
    AudioEngine();
    ~AudioEngine();

    // amplificationMode: true이면 입력 신호에 2배 증폭 적용
    // 파라미터는 AudioConfig.h의 값을 기본값으로 사용합니다.
    oboe::Result init(bool amplificationMode, int32_t sampleRate = SAMPLE_RATE, int32_t bufferSize = BUFFER_SIZE);
    oboe::Result start();
    oboe::Result stop();

    // 좌우 채널을 인터리브한 스테레오 데이터를 반환
    std::vector<int16_t> getRecordedData();

    // 출력 스트림 콜백 (500Hz 톤 생성)
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *oboeStream,
                                          void *audioData,
                                          int32_t numFrames) override;

    // 입력 녹음 스레드 함수 (마이크 입력)
    void inputRecordingThread();

    // 실제 사용된 샘플 레이트를 반환 (WAV 파일 저장 등에 사용)
    int getSampleRate() const { return mSampleRate; }

private:
    ManagedStream mInputStream;
    ManagedStream mOutputStream;

    std::atomic<bool> mKeepRecording;
    std::thread mInputThread;

    std::vector<int16_t> mLeftBuffer;
    std::vector<int16_t> mRightBuffer;

    // 녹음할 총 프레임 수 (예: 10초 동안, sampleRate * 10)
    int32_t mTotalFrames;

    bool mAmplificationMode;
    SignalGenerator mSignalGenerator;

    int32_t mSampleRate;
    int32_t mBufferSize;
};
