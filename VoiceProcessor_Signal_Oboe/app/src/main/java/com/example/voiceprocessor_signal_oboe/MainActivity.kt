package com.example.voiceprocessor_signal_oboe

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import java.io.File
import java.io.FileOutputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder

class MainActivity : AppCompatActivity() {

    private val PERMISSION_REQUEST_CODE = 1001
    private val nativeAudio = NativeAudio()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        if (!hasPermissions()) {
            requestPermissions()
        }

        val bypassButton = findViewById<Button>(R.id.buttonBypass)
        val amplificationButton = findViewById<Button>(R.id.buttonAmplification)
        val stopSaveButton = findViewById<Button>(R.id.buttonStopSave)

        // AudioConfig.h의 SAMPLE_RATE, BUFFER_SIZE는 내부에서 사용되므로 여기서는 초기화 호출만 합니다.
        nativeAudio.init(16000, 48)

        bypassButton.setOnClickListener {
            Toast.makeText(this, "Bypass 모드 시작", Toast.LENGTH_SHORT).show()
            nativeAudio.startBypass()
        }

        amplificationButton.setOnClickListener {
            Toast.makeText(this, "2배 Amplification 모드 시작", Toast.LENGTH_SHORT).show()
            nativeAudio.startAmplification()
        }

        stopSaveButton.setOnClickListener {
            Toast.makeText(this, "녹음 종료 및 저장", Toast.LENGTH_SHORT).show()
            nativeAudio.stopAudio()
            val recordedData: ShortArray = nativeAudio.getRecordedData()
            // 실제 사용된 샘플레이트를 네이티브에서 받아와서 사용
            val actualSampleRate = nativeAudio.getSampleRate()
            saveWavFile(recordedData, actualSampleRate)
        }
    }

    private fun hasPermissions(): Boolean {
        val recordPermission = ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO)
        val storagePermission = ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
        return recordPermission == PackageManager.PERMISSION_GRANTED &&
                storagePermission == PackageManager.PERMISSION_GRANTED
    }

    private fun requestPermissions() {
        ActivityCompat.requestPermissions(
            this,
            arrayOf(Manifest.permission.RECORD_AUDIO, Manifest.permission.WRITE_EXTERNAL_STORAGE),
            PERMISSION_REQUEST_CODE
        )
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<String>, grantResults: IntArray) {
        if (requestCode == PERMISSION_REQUEST_CODE) {
            if (grantResults.isNotEmpty() && grantResults.all { it == PackageManager.PERMISSION_GRANTED }) {
                Toast.makeText(this, "모든 권한 승인", Toast.LENGTH_SHORT).show()
            } else {
                Toast.makeText(this, "권한이 필요합니다.", Toast.LENGTH_SHORT).show()
            }
        }
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
    }

    private fun saveWavFile(audioData: ShortArray, sampleRate: Int) {
        val numChannels = 2
        val bitsPerSample = 16
        val dataSize = audioData.size * 2  // Short는 2바이트
        val chunkSize = 36 + dataSize
        val byteRate = sampleRate * numChannels * bitsPerSample / 8

        try {
            val folder = File(getExternalFilesDir(null), "VoiceProcessor")
            if (!folder.exists() && !folder.mkdirs()) {
                Toast.makeText(this, "폴더 생성 실패", Toast.LENGTH_SHORT).show()
                return
            }
            val file = File(folder, "recorded_Signal.wav")
            val outputStream = FileOutputStream(file)

            val header = ByteBuffer.allocate(44)
            header.order(ByteOrder.LITTLE_ENDIAN)
            header.put("RIFF".toByteArray(Charsets.US_ASCII))
            header.putInt(chunkSize)
            header.put("WAVE".toByteArray(Charsets.US_ASCII))
            header.put("fmt ".toByteArray(Charsets.US_ASCII))
            header.putInt(16) // Subchunk1Size (PCM)
            header.putShort(1.toShort()) // AudioFormat PCM
            header.putShort(numChannels.toShort())
            header.putInt(sampleRate)
            header.putInt(byteRate)
            header.putShort((numChannels * bitsPerSample / 8).toShort())
            header.putShort(bitsPerSample.toShort())
            header.put("data".toByteArray(Charsets.US_ASCII))
            header.putInt(dataSize)
            outputStream.write(header.array())

            val audioBuffer = ByteBuffer.allocate(dataSize)
            audioBuffer.order(ByteOrder.LITTLE_ENDIAN)
            for (sample in audioData) {
                audioBuffer.putShort(sample)
            }
            outputStream.write(audioBuffer.array())
            outputStream.close()
            Toast.makeText(this, "파일 저장 완료: ${file.absolutePath}", Toast.LENGTH_SHORT).show()
        } catch (e: Exception) {
            Toast.makeText(this, "파일 저장 실패: ${e.message}", Toast.LENGTH_SHORT).show()
        }
    }
}
