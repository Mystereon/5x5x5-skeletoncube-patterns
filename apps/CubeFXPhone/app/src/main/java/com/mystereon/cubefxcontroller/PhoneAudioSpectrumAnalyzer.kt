package com.mystereon.cubefxcontroller

import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import java.util.concurrent.Executors
import java.util.concurrent.atomic.AtomicBoolean
import kotlin.math.PI
import kotlin.math.cos
import kotlin.math.hypot
import kotlin.math.max
import kotlin.math.min
import kotlin.math.sin
import kotlin.math.sqrt

/**
 * Analyses live microphone PCM entirely on the phone and emits only a 13-byte
 * spectrum envelope. No raw audio is retained, written to storage, or sent to
 * the cube. 256 samples at 16 kHz give a compact visualiser-oriented FFT frame.
 */
class PhoneAudioSpectrumAnalyzer(private val onSpectrum: (ByteArray) -> Unit) {
    private val running = AtomicBoolean(false)
    private val executor = Executors.newSingleThreadExecutor()
    private var activeRecord: AudioRecord? = null
    private var sequence = 0
    private val _streaming = MutableStateFlow(false)
    val streaming: StateFlow<Boolean> = _streaming

    fun start(): Boolean {
        if (!running.compareAndSet(false, true)) return true
        executor.execute(::captureLoop)
        return true
    }

    fun stop() {
        running.set(false)
        try { activeRecord?.stop() } catch (_: IllegalStateException) { }
        activeRecord = null
        _streaming.value = false
    }

    fun close() {
        stop()
        executor.shutdownNow()
    }

    private fun captureLoop() {
        val minBytes = AudioRecord.getMinBufferSize(SAMPLE_RATE, CHANNEL, ENCODING)
        if (minBytes <= 0) {
            running.set(false)
            _streaming.value = false
            return
        }
        val record = AudioRecord(
            MediaRecorder.AudioSource.DEFAULT,
            SAMPLE_RATE,
            CHANNEL,
            ENCODING,
            max(minBytes, SAMPLE_COUNT * 2),
        )
        activeRecord = record
        val samples = ShortArray(SAMPLE_COUNT)
        val smoothBands = DoubleArray(BAND_COUNT)
        var smoothLoudness = 0.0
        var previousLoudness = 0.0
        var lastSentAt = 0L
        try {
            record.startRecording()
            _streaming.value = true
            while (running.get()) {
                val count = record.read(samples, 0, samples.size)
                if (count != SAMPLE_COUNT) continue
                val now = android.os.SystemClock.elapsedRealtime()
                if (now - lastSentAt < STREAM_INTERVAL_MS) continue
                lastSentAt = now
                val frame = analyse(samples, smoothBands, smoothLoudness, previousLoudness)
                smoothLoudness = frame.loudness
                previousLoudness = frame.loudness
                onSpectrum(frame.packet)
            }
        } catch (_: SecurityException) {
            // Permission is requested by the activity before start; this keeps
            // a revoked permission from crashing the controller.
        } catch (_: IllegalStateException) {
            // A disconnected or unavailable audio route simply stops the link.
        } finally {
            try { record.stop() } catch (_: IllegalStateException) { }
            record.release()
            activeRecord = null
            running.set(false)
            _streaming.value = false
        }
    }

    private data class Frame(val packet: ByteArray, val loudness: Double)

    private fun analyse(samples: ShortArray, smoothBands: DoubleArray, priorLoudness: Double, previous: Double): Frame {
        val real = DoubleArray(SAMPLE_COUNT)
        val imaginary = DoubleArray(SAMPLE_COUNT)
        var rmsSum = 0.0
        for (i in samples.indices) {
            // Hann window reduces abrupt frame edges before the FFT.
            val normalised = samples[i] / 32768.0
            rmsSum += normalised * normalised
            real[i] = normalised * (0.5 - 0.5 * cos(2.0 * PI * i / (SAMPLE_COUNT - 1)))
        }
        fft(real, imaginary)

        val packet = ByteArray(PACKET_BYTES)
        packet[0] = MAGIC.toByte()
        packet[1] = VERSION.toByte()
        packet[2] = (sequence++ and 0xFF).toByte()
        for (band in 0 until BAND_COUNT) {
            var energy = 0.0
            var contributors = 0
            for (bin in BAND_EDGES[band] until BAND_EDGES[band + 1]) {
                energy += hypot(real[bin], imaginary[bin])
                contributors++
            }
            // Gain/noise floor are aesthetic controls, not calibrated SPL.
            val raw = ((energy / max(1, contributors)) * 10.0 - 0.018).coerceIn(0.0, 1.0)
            smoothBands[band] = smoothBands[band] * 0.66 + raw * 0.34
            packet[3 + band] = (smoothBands[band] * 255.0).toInt().coerceIn(0, 255).toByte()
        }
        val rawLoudness = ((sqrt(rmsSum / SAMPLE_COUNT) - 0.012) * 11.0).coerceIn(0.0, 1.0)
        val loudness = priorLoudness * 0.72 + rawLoudness * 0.28
        val beat = if (loudness > max(0.14, previous * 1.32)) 1.0 else 0.0
        packet[11] = (loudness * 255.0).toInt().coerceIn(0, 255).toByte()
        packet[12] = (beat * 255.0).toInt().toByte()
        return Frame(packet, loudness)
    }

    // In-place radix-2 Cooley-Tukey FFT. The phone does the heavier work so
    // CubeFX only receives eight visible-band values, not sound samples.
    private fun fft(real: DoubleArray, imaginary: DoubleArray) {
        var j = 0
        for (i in 1 until real.size) {
            var bit = real.size shr 1
            while (j and bit != 0) { j = j xor bit; bit = bit shr 1 }
            j = j xor bit
            if (i < j) {
                val tempReal = real[i]; real[i] = real[j]; real[j] = tempReal
                val tempImaginary = imaginary[i]; imaginary[i] = imaginary[j]; imaginary[j] = tempImaginary
            }
        }
        var length = 2
        while (length <= real.size) {
            val angle = -2.0 * PI / length
            val stepReal = cos(angle)
            val stepImaginary = sin(angle)
            for (start in real.indices step length) {
                var weightReal = 1.0
                var weightImaginary = 0.0
                for (offset in 0 until length / 2) {
                    val even = start + offset
                    val odd = even + length / 2
                    val oddReal = real[odd] * weightReal - imaginary[odd] * weightImaginary
                    val oddImaginary = real[odd] * weightImaginary + imaginary[odd] * weightReal
                    real[odd] = real[even] - oddReal
                    imaginary[odd] = imaginary[even] - oddImaginary
                    real[even] += oddReal
                    imaginary[even] += oddImaginary
                    val nextWeightReal = weightReal * stepReal - weightImaginary * stepImaginary
                    weightImaginary = weightReal * stepImaginary + weightImaginary * stepReal
                    weightReal = nextWeightReal
                }
            }
            length = length shl 1
        }
    }

    private companion object {
        const val SAMPLE_RATE = 16_000
        const val SAMPLE_COUNT = 256
        const val BAND_COUNT = 8
        const val STREAM_INTERVAL_MS = 40L // 25 packets per second maximum
        const val CHANNEL = AudioFormat.CHANNEL_IN_MONO
        const val ENCODING = AudioFormat.ENCODING_PCM_16BIT
        const val MAGIC = 0xA7
        const val VERSION = 0x01
        const val PACKET_BYTES = 13
        // FFT bin edges: roughly 63 Hz through 3.4 kHz at 16 kHz / 256 samples.
        val BAND_EDGES = intArrayOf(1, 2, 4, 7, 11, 17, 25, 37, 55)
    }
}
