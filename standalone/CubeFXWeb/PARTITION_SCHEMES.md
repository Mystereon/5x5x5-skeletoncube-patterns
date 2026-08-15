# CubeFX ESP32-C3 partition schemes

CubeFXWeb ships with a custom `partitions.csv` selected for a **4 MB ESP32-C3 SuperMini**. The default is deliberately **no OTA and no SPIFFS**: CubeFX stores only small settings in NVS and contains no images, audio, HTML files, or other runtime assets that need a filesystem. The result is a single **0x3f0000-byte application partition** (4,128,768 bytes; approximately 3.94 MiB), which is the best headroom for FastLED, Wi-Fi, BLE, the browser controller, and the expanding procedural pattern library.

| Profile | App capacity | SPIFFS | OTA | Use it when |
|---|---:|---:|---:|---|
| **Default `partitions.csv`** | 0x3f0000 / 3.94 MiB | None | No | Recommended CubeFX profile. Flash upgrades through USB/Arduino IDE. |
| **`partitions_storage_1mb.csv`** | 0x2f0000 / 2.94 MiB | 1 MiB | No | Future version with locally stored presets, assets, or files. |
| **2 MB app / 2 MB SPIFFS** | About 2 MiB | About 2 MiB | No | Not recommended at present: it spends flash on an unused filesystem and constrains firmware growth. |
| **Dual OTA** | About 1.9 MiB per slot | None or very small | Yes | Only after the firmware is comfortably below the per-slot limit and remote upgrades become more valuable than pattern capacity. |

Arduino-ESP32 automatically uses `partitions.csv` placed in the same sketch folder. To move to the storage profile, back up the default file, copy `partitions_storage_1mb.csv` to the name `partitions.csv`, and compile/upload the whole sketch. Changing a partition table requires a USB reflash; it cannot be safely applied through the CubeFX web or BLE control paths.

The table retains a 24 KB NVS partition for Wi-Fi and persisted button pins, plus a 4 KB PHY partition. Espressif describes NVS as the appropriate place for small configuration data and identifies a single factory app with no OTA as the simpler configuration when a second update image is not required. [1]

## References

[1]: https://docs.espressif.com/projects/arduino-esp32/en/latest/tutorials/partition_table.html "Arduino-ESP32 partition tables"
