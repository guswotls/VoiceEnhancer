#include <jni.h>
#include <memory>
#include "AudioEngine.h"
#include "AudioConfig.h"  // 매크로 포함

static std::unique_ptr<AudioEngine> engine = nullptr;

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_voiceprocessor_1signal_1oboe_NativeAudio_init(JNIEnv* env, jobject /* this */, jint /*sampleRate*/, jint /*bufferSize*/) {
    if(!engine) {
        engine = std::make_unique<AudioEngine>();
    }
    // amplification 모드는 false, sampleRate와 bufferSize는 AudioConfig.h 값 사용
    engine->init(false, SAMPLE_RATE, BUFFER_SIZE);
}

JNIEXPORT void JNICALL
Java_com_example_voiceprocessor_1signal_1oboe_NativeAudio_startBypass(JNIEnv* env, jobject /* this */) {
    if(engine) {
        engine->init(false, SAMPLE_RATE, BUFFER_SIZE);
        engine->start();
    }
}

JNIEXPORT void JNICALL
Java_com_example_voiceprocessor_1signal_1oboe_NativeAudio_startAmplification(JNIEnv* env, jobject /* this */) {
    if(engine) {
        engine->init(true, SAMPLE_RATE, BUFFER_SIZE);
        engine->start();
    }
}

JNIEXPORT void JNICALL
Java_com_example_voiceprocessor_1signal_1oboe_NativeAudio_stopAudio(JNIEnv* env, jobject /* this */) {
    if(engine) {
        engine->stop();
    }
}

JNIEXPORT jshortArray JNICALL
Java_com_example_voiceprocessor_1signal_1oboe_NativeAudio_getRecordedData(JNIEnv* env, jobject /* this */) {
    std::vector<int16_t> data = engine ? engine->getRecordedData() : std::vector<int16_t>();
    jshortArray result = env->NewShortArray(data.size());
    env->SetShortArrayRegion(result, 0, data.size(), data.data());
    return result;
}

// 새로 추가: 실제 사용된 샘플 레이트 반환
JNIEXPORT jint JNICALL
Java_com_example_voiceprocessor_1signal_1oboe_NativeAudio_getSampleRate(JNIEnv* env, jobject /* this */) {
    return engine ? engine->getSampleRate() : SAMPLE_RATE;
}

} // extern "C"
