package com.mystereon.cubefxwatch

import java.util.UUID

object CubeFxProtocol {
    val serviceUuid: UUID = UUID.fromString("6c75a300-7b1d-4f29-a221-000000000001")
    val commandUuid: UUID = UUID.fromString("6c75a300-7b1d-4f29-a221-000000000002")
    val statusUuid: UUID = UUID.fromString("6c75a300-7b1d-4f29-a221-000000000003")
    const val devicePrefix = "CubeFX"

    data class Pattern(val canonicalId: Int, val title: String)

    /**
     * These are the modes embedded in CubeFXWeb. Canonical IDs are
     * intentionally retained because the ESP32 BLE service selects by the
     * 56-pattern library's stable numbering rather than enum position.
     */
    val embeddedPatterns = listOf(
        Pattern(1, "Red Vector Cube"), Pattern(9, "3-D Matrix Rain"),
        Pattern(21, "Neon Plasma"), Pattern(22, "Volume Fire"),
        Pattern(26, "Twin Spirals"), Pattern(28, "Wrapping Comets"),
        Pattern(12, "Single-player Pong"), Pattern(19, "Conway 3-D Life"),
        Pattern(20, "Cloud Volume"), Pattern(11, "White Glitter"),
        Pattern(10, "Corner Cubes"), Pattern(29, "Perimeter Banner"),
        Pattern(30, "Bullet Wall"), Pattern(31, "Padded Cell"),
        Pattern(32, "Block Run"), Pattern(33, "Parallax Starfield"),
        Pattern(34, "Trench Run"), Pattern(35, "Running Legs"),
        Pattern(36, "Fairies in Green Box"), Pattern(37, "Orange Fish Tank"),
        Pattern(38, "Three-Layer Pyramid"), Pattern(39, "Matrix Drift"),
        Pattern(40, "Intense Fire"), Pattern(41, "Magical Blue Fire"),
        Pattern(42, "Explosions"), Pattern(43, "Launching Fireworks"),
        Pattern(44, "Pixel Pasture"), Pattern(45, "Red Matrix Rain"),
        Pattern(46, "Voxel Minesweeper"), Pattern(47, "Big Moon & Stars"),
        Pattern(48, "Nixie Tube"), Pattern(49, "Black Hole Vortex"),
        Pattern(50, "Stargate Dial-Up"), Pattern(51, "3-D Defender"),
        Pattern(52, "3-D Chequerboard"), Pattern(53, "Hellraiser Puzzle Cube"),
        Pattern(54, "3-D Rubik's Cube"), Pattern(55, "Lissajous Layer Ripple"),
        Pattern(56, "Zarch: Voxel Defender")
    )

    fun patternCommand(pattern: Pattern) = "{\"op\":\"pattern\",\"id\":${pattern.canonicalId}}"
    fun engineCommand(brightness: Int, speed: Int, auto: Boolean) =
        "{\"op\":\"engine\",\"brightness\":$brightness,\"speed\":$speed,\"cycle\":30,\"auto\":$auto}"
    fun nextCommand() = "{\"op\":\"next\"}"
    fun reseedCommand() = "{\"op\":\"reseed\"}"
    fun actionCommand(primary: Boolean) = "{\"op\":\"action\",\"primary\":$primary}"
    fun bannerCommand(text: String, font: Int, hue: Int, speed: Int) =
        "{\"op\":\"banner\",\"text\":\"${text.replace("\"", " ")}\",\"font\":$font,\"hue\":$hue,\"speed\":$speed}"
}
