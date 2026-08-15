# CubeFX Galaxy Watch Companion — Compatibility Notes

Samsung states that it moved Galaxy Watch from Tizen to Wear OS Powered by Samsung starting with **Galaxy Watch4**. New watch applications for Watch4 and later are distributed through Google Play, while new or updated Tizen watch apps can no longer be registered. [1]

Android's BLE platform APIs support the needed central/GATT-client flow: scan for an advertising peripheral, connect to its GATT server, discover the CubeFX service, and write compact control values to its command characteristic. [2]

Samsung's current Galaxy Watch8 Classic is offered as a **46 mm** model and returns the physical rotating bezel, which Samsung describes as a precise app-control input. The Watch8 Classic page lists a stainless-steel case; if the owner's unit is described as titanium, this may refer to its finish, band, or a different Galaxy Watch model and should be checked before installation. [3]

The CubeFX watch controller should therefore be a native Wear OS application for the 46 mm Galaxy Watch8 Classic. Its preferred interaction is a bezel-driven pattern browser, with touch controls for brightness, speed, playback mode, Banner, and pattern-specific actions such as Pong left/right.

## References

[1]: https://developer.samsung.com/galaxy-watch-tizen/notice.html "Samsung Developer: Galaxy Watch for Tizen"
[2]: https://developer.android.com/develop/connectivity/bluetooth/ble/ble-overview "Android Developers: Bluetooth Low Energy overview"
[3]: https://www.samsung.com/us/watches/galaxy-watch8-classic/ "Samsung: Galaxy Watch8 Classic"
