package com.example.voiceprocessor_signal_oboe

class NativeAudio {
    companion object {
        init {
            System.loadLibrary("voiceprocessor_signal_oboe")
        }
    }

    external fun init(sampleRate: Int, bufferSize: Int)
    external fun startBypass()
    external fun startAmplification()
    external fun stopAudio()
    external fun getRecordedData(): ShortArray
    external fun getSampleRate(): Int  // 새로 추가한 네이티브 메소드
}
