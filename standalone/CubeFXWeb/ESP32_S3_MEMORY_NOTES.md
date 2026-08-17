# ESP32-S3 Zero memory notes for CubeFX

The Waveshare ESP32-S3-Zero board used for the CubeFX S3 profile is built around ESP32-S3FH4R2 and provides **4 MB flash**, **2 MB PSRAM**, and **512 KB internal SRAM**. GPIO33–GPIO37 are reserved by its octal PSRAM, while GPIO21 drives the onboard RGB LED. The CubeFX enclosure profile uses GPIO6 for matrix/mood-LED data, GPIO2 for the primary switch, and GPIO4 for the secondary switch. [1]

CubeFX should keep the 126-pixel FastLED transmit buffer, active pattern state, RMT-sensitive output data, and small per-frame lookup tables in internal RAM. The optional S3 PSRAM is appropriate for low-frequency cinematic timelines, cached scene descriptions, optional replay/diagnostic records, or other large non-DMA data. It should not hold ISR-critical data, task stacks, or memory accessed while flash cache may be disabled. Espressif documents that external RAM is mapped through the cache and has restrictions around flash cache, DMA, and task stacks. [2]

## Source references

[1]: https://docs.waveshare.com/ESP32-S3-Zero "Waveshare ESP32-S3-Zero documentation"
[2]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/external-ram.html "Espressif ESP32-S3 external RAM guide"
