# CubeFX Phone Audio Spectrum Protocol

CubeFX can visualise **live audio analysis from the Android controller** without recording or transmitting raw sound. The phone performs microphone capture and local analysis; the ESP32-S3 receives only a compact envelope packet containing eight already-normalised spectral bands, a loudness value, and a beat estimate.

> **Privacy boundary:** the phone begins capture only after the user explicitly presses **Start Audio Link** and Android grants `RECORD_AUDIO`. No audio samples, recordings, filenames, or speech data are sent to CubeFX or stored by either device.

## Transport

The existing CubeFX BLE command characteristic is used with **write without response**. Audio packets are deliberately binary and only 13 bytes long, so they fit within the ordinary BLE payload floor without relying on a negotiated MTU.

| Byte | Name | Range | Meaning |
|---:|---|---:|---|
| 0 | Magic | `0xA7` | Identifies a spectrum packet rather than the normal JSON command protocol. |
| 1 | Version | `0x01` | Packet-format version. |
| 2 | Sequence | `0–255` | Incremented by the phone for diagnostics and dropped-packet visibility. |
| 3–10 | Bands 0–7 | `0–255` | Low-to-high normalised spectrum magnitudes. |
| 11 | Loudness | `0–255` | Smoothed RMS/envelope measure. |
| 12 | Beat | `0–255` | Short beat/peak impulse, otherwise near zero. |

The app captures mono 16-bit PCM at 16 kHz, analyses a 256-sample window locally, combines FFT bins into eight logarithmically widening bands, applies smoothing/noise gating, then transmits at a maximum **25 packets per second**. This limits the transport to approximately **325 bytes per second** before BLE framing overhead while retaining a responsive visual effect.

## Firmware behaviour

CubeFX keeps only 10 bytes of active audio state: eight bands, loudness, and beat. The original microphone data and FFT buffer never exist on the cube. A packet updates the state timestamp; after **550 ms** without new packets, the firmware decays every value smoothly toward black. This makes an Android stop, loss of Bluetooth connection, or permission denial fail safely rather than leaving a frozen peak displayed.

## Visualiser Modes

| Mode | BLE ID | Cube treatment | Rear ring |
|---|---:|---|---|
| **Phone VU Meter** | 60 | Eight vertical meter columns distributed across the 5×5×5 volume, with falling peaks. | Bass-weighted cyan-to-amber pulse. |
| **Phone Spectrum 3-D** | 61 | Eight spectrum bands mapped onto moving depth lanes; loudness lifts the whole field. | Circular low-frequency chase and white beat flashes. |

Both modes still render correctly with no phone stream: they decay to black, making it visually obvious that the phone link is inactive.

## Android User Flow

The Android controller shows the **Audio Link** panel only as an opt-in control. The first press requests microphone permission through Android’s normal system prompt. Once granted, the app starts its local analyser and makes the Start button read **Stop Audio Link**. Stopping audio, leaving the activity, or losing BLE stops `AudioRecord` immediately.

Created by **Dad (MysterEon) & Manus**.
