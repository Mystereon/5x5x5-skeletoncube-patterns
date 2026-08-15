# CubeFX Android controllers

This directory contains two native Kotlin/Jetpack Compose projects built against the same CubeFX BLE service:

| Project | Target | Core interaction |
|---|---|---|
| [`CubeFXPhone/`](CubeFXPhone/) | Android phone, API 26+ | 54-pattern browser, engine and Banner controls, ESP32 setup, and Watch installation route. |
| [`CubeFXWatch/`](CubeFXWatch/) | Galaxy Watch8 Classic, Wear OS API 30+ | Physical bezel pattern browse/select, touch engine controls, Banner/Auto actions, and Pong left/right. |

Both projects communicate with an ESP32-C3 running the current `standalone/CubeFXWeb` sketch, advertising `CubeFX-5x5x5` over BLE. The phone app identifies all 54 canonical effects but clearly labels the 37 modes presently embedded in CubeFXWeb; the remaining items are uploadable standalone demos.

## Build an APK in Android Studio

Install the current Android Studio release with Android SDK Platform 35. Open either project folder as a Gradle project, allow Gradle to sync its declared Android and Compose dependencies, then select **Build → Build APK(s)**. Android Studio places the debug package under `app/build/outputs/apk/debug/`.

For the Watch8 Classic, enable **Developer options** and **ADB debugging** on the watch, connect through Android Studio’s Device Manager or Wear OS pairing path, then install the generated watch APK. The Wear app must be installed on the watch itself—not merely the phone—because it scans and connects directly to CubeFX over BLE.

> The version in this repository is source-complete but has not been hardware-tested on a specific phone/watch pair from this sandbox. Flash the current CubeFXWeb firmware first, confirm it advertises as `CubeFX-5x5x5`, and test connection, pattern selection, primary/secondary actions, and custom button-pin persistence on your own hardware.
