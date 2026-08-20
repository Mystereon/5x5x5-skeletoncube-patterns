package com.mystereon.cubefxcontroller

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.RowScope
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Text
import androidx.compose.material3.TextField
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.compose.collectAsStateWithLifecycle

private val Graphite = Color(0xFF0B0D0B)
private val Panel = Color(0xFF141814)
private val Lime = Color(0xFFC8FF20)
private val Cyan = Color(0xFF32C8FF)
private val Amber = Color(0xFFFF7A16)
private val Warning = Color(0xFFFF432E)
private val TextMain = Color(0xFFEEF4E8)
private val Muted = Color(0xFF9BA896)

class MainActivity : ComponentActivity() {
    private lateinit var cubeFx: CubeFxBleClient
    private lateinit var audioAnalyzer: PhoneAudioSpectrumAnalyzer
    private val permissionRequest = registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) { result ->
        if (result.values.all { it }) cubeFx.scan()
    }
    private val audioPermissionRequest = registerForActivityResult(ActivityResultContracts.RequestPermission()) { granted ->
        if (granted) audioAnalyzer.start()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        cubeFx = CubeFxBleClient(applicationContext)
        audioAnalyzer = PhoneAudioSpectrumAnalyzer(cubeFx::sendAudioSpectrum)
        setContent {
            CubeFxPhoneApp(
                cubeFx = cubeFx,
                audioAnalyzer = audioAnalyzer,
                requestBluetooth = { permissionRequest.launch(arrayOf(Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT)) },
                requestAudio = {
                    if (checkSelfPermission(Manifest.permission.RECORD_AUDIO) == PackageManager.PERMISSION_GRANTED) audioAnalyzer.start()
                    else audioPermissionRequest.launch(Manifest.permission.RECORD_AUDIO)
                },
            )
        }
    }

    override fun onStop() { audioAnalyzer.stop(); super.onStop() }
    override fun onDestroy() { audioAnalyzer.close(); cubeFx.close(); super.onDestroy() }
}

private enum class PhonePage { LIVE, PATTERNS, SETUP, FIRMWARE, WATCH, EGGS, RESOURCES }

@Composable
private fun CubeFxPhoneApp(cubeFx: CubeFxBleClient, audioAnalyzer: PhoneAudioSpectrumAnalyzer, requestBluetooth: () -> Unit, requestAudio: () -> Unit) {
    var page by remember { mutableStateOf(PhonePage.LIVE) }
    val state by cubeFx.state.collectAsStateWithLifecycle()
    val notice by cubeFx.notice.collectAsStateWithLifecycle()
    Column(Modifier.fillMaxSize().background(Graphite).padding(14.dp), verticalArrangement = Arrangement.spacedBy(10.dp)) {
        Text("CUBE.FX", color = Lime, fontWeight = FontWeight.Black, fontSize = 15.sp, letterSpacing = 2.sp)
        Text("${state.name}  ·  $notice", color = if (state == PhoneConnection.READY) Cyan else Muted, fontSize = 12.sp)
        Row(Modifier.fillMaxSize(), horizontalArrangement = Arrangement.spacedBy(10.dp)) {
            Column(Modifier.width(76.dp).fillMaxHeight(), verticalArrangement = Arrangement.spacedBy(7.dp)) {
                NavButton("01\nLIVE", page == PhonePage.LIVE) { page = PhonePage.LIVE }
                NavButton("02\nFX", page == PhonePage.PATTERNS) { page = PhonePage.PATTERNS }
                NavButton("03\nESP", page == PhonePage.SETUP) { page = PhonePage.SETUP }
                NavButton("04\nFW", page == PhonePage.FIRMWARE) { page = PhonePage.FIRMWARE }
                NavButton("05\nWATCH", page == PhonePage.WATCH) { page = PhonePage.WATCH }
                NavButton("06\nEGGS", page == PhonePage.EGGS) { page = PhonePage.EGGS }
                NavButton("07\nMORE", page == PhonePage.RESOURCES) { page = PhonePage.RESOURCES }
            }
            Column(Modifier.weight(1f).fillMaxHeight()) {
                when (page) {
                    PhonePage.LIVE -> LivePage(cubeFx, audioAnalyzer, requestBluetooth, requestAudio) { page = it }
                    PhonePage.PATTERNS -> PatternPage(cubeFx)
                    PhonePage.SETUP -> SetupPage(cubeFx)
                    PhonePage.FIRMWARE -> FirmwarePage()
                    PhonePage.WATCH -> WatchPage()
                    PhonePage.EGGS -> EasterEggsPage(cubeFx)
                    PhonePage.RESOURCES -> ResourcesPage()
                }
            }
        }
    }
}

@Composable
private fun LivePage(cubeFx: CubeFxBleClient, audioAnalyzer: PhoneAudioSpectrumAnalyzer, requestBluetooth: () -> Unit, requestAudio: () -> Unit, onNavigate: (PhonePage) -> Unit) {
    var brightness by remember { mutableIntStateOf(100) }
    var speed by remember { mutableIntStateOf(150) }
    var auto by remember { mutableStateOf(true) }
    var banner by remember { mutableStateOf("CUBE 4 3 2 1 0") }
    val audioStreaming by audioAnalyzer.streaming.collectAsStateWithLifecycle()
    val connection by cubeFx.state.collectAsStateWithLifecycle()
    Column(Modifier.verticalScroll(rememberScrollState()), verticalArrangement = Arrangement.spacedBy(12.dp)) {
        Text("CUBE CONTROL", color = TextMain, fontSize = 28.sp, fontWeight = FontWeight.Black)
        Text("The everyday controls are here. Connect once, then use the big buttons—no manual required.", color = Muted, fontSize = 14.sp)
        ActionButton("SCAN FOR CUBEFX", Lime, requestBluetooth)

        NoticeCard("PATTERN NOW", "Choose a specific effect in FX, or move cleanly to the next one from here.")
        ActionButton("NEXT PATTERN  ›", Cyan) { cubeFx.sendNextPattern() }
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            MiniActionButton("FX GALLERY", Panel, TextMain) { onNavigate(PhonePage.PATTERNS) }
            MiniActionButton("EASTER EGGS", Amber, Color.Black) { onNavigate(PhonePage.EGGS) }
        }

        NoticeCard("ENGINE", "Brightness $brightness  ·  Speed $speed  ·  ${if (auto) "AUTO" else "MANUAL"}")
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            StepButton("B−") { brightness = (brightness - 16).coerceAtLeast(1) }
            StepButton("B+") { brightness = (brightness + 16).coerceAtMost(255) }
            StepButton("S−") { speed = (speed - 16).coerceAtLeast(1) }
            StepButton("S+") { speed = (speed + 16).coerceAtMost(255) }
            StepButton(if (auto) "AUTO" else "MANUAL") { auto = !auto }
        }
        ActionButton("APPLY ENGINE SETTINGS", Lime) { cubeFx.sendEngine(brightness, speed, auto) }
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            MiniActionButton("AUTO", if (auto) Lime else Panel, if (auto) Color.Black else TextMain) { auto = true; cubeFx.sendEngine(brightness, speed, true) }
            MiniActionButton("MANUAL", if (!auto) Amber else Panel, if (!auto) Color.Black else TextMain) { auto = false; cubeFx.sendEngine(brightness, speed, false) }
        }

        NoticeCard("AUDIO LINK", if (audioStreaming) "Microphone analysis is active. Eight local spectrum bands are streaming to CubeFX; no raw audio is sent." else "Start Audio Link before selecting Phone VU Meter or Phone Spectrum 3-D.")
        ActionButton(if (audioStreaming) "STOP AUDIO LINK" else "START AUDIO LINK", if (audioStreaming) Warning else Cyan) {
            if (audioStreaming) audioAnalyzer.stop() else if (connection == PhoneConnection.READY) requestAudio()
        }
        if (!audioStreaming && connection != PhoneConnection.READY) Text("Connect to CubeFX before starting Audio Link.", color = Amber, fontSize = 12.sp)

        NoticeCard("BANNER", "Type it normally. The CubeFX firmware now reads the physical perimeter forward.")
        TextField(value = banner, onValueChange = { banner = it.take(60) }, label = { Text("Banner text") }, modifier = Modifier.fillMaxWidth())
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            MiniActionButton("SEND", Cyan, Color.Black) { cubeFx.sendBanner(banner) }
            MiniActionButton("ACTION 1", Amber, Color.Black) { cubeFx.sendAction(true) }
            MiniActionButton("ACTION 2", Amber, Color.Black) { cubeFx.sendAction(false) }
        }
        NoticeCard("CONTEXT ACTIONS", "ACTION 1 and ACTION 2 mirror short physical buttons for the active effect: Pong movement, colour changes, Zarch actions, rain palettes, and more.")

        NoticeCard("QUICK ROUTES", "Firmware, wiring setup, the Galaxy Watch controller, repository, support link, and owner controls are all one tap away.")
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            MiniActionButton("FIRMWARE", Panel, TextMain) { onNavigate(PhonePage.FIRMWARE) }
            MiniActionButton("ESP32 SETUP", Panel, TextMain) { onNavigate(PhonePage.SETUP) }
            MiniActionButton("WATCH", Panel, TextMain) { onNavigate(PhonePage.WATCH) }
        }
    }
}

@Composable
private fun PatternPage(cubeFx: CubeFxBleClient) {
    val context = LocalContext.current
    var requestedPatternId by remember { mutableIntStateOf(-1) }
    Column(Modifier.fillMaxSize(), verticalArrangement = Arrangement.spacedBy(8.dp)) {
        Text("CUBE.FX LIBRARY", color = TextMain, fontWeight = FontWeight.Black, fontSize = 22.sp)
        Text("The first 56 entries below are the modes built into CubeFXWeb and every PLAY button sends its canonical BLE ID. The remaining 17 are honest standalone reference sketches, kept separate so no visible row silently does nothing.", color = Muted, fontSize = 13.sp)
        if (requestedPatternId > 0) Text("REQUESTED: ${requestedPatternId.toString().padStart(2, '0')} — awaiting CubeFX acknowledgement", color = Cyan, fontSize = 12.sp, fontWeight = FontWeight.Bold)
        LazyColumn(modifier = Modifier.weight(1f), verticalArrangement = Arrangement.spacedBy(7.dp)) {
            item { Text("56 EMBEDDED CUBEFXWEB MODES", color = Lime, fontSize = 12.sp, fontWeight = FontWeight.Black, modifier = Modifier.padding(top = 4.dp)) }
            items(CubeFxPatternCatalog.embeddedModes, key = { it.id }) { pattern ->
                Button(onClick = { requestedPatternId = pattern.id; cubeFx.sendPattern(pattern.id) }, modifier = Modifier.fillMaxWidth(), colors = ButtonDefaults.buttonColors(containerColor = Panel, contentColor = TextMain)) {
                    Text("PLAY ${pattern.id.toString().padStart(2, '0')}  ${pattern.title}", fontWeight = FontWeight.Bold)
                }
            }
            item {
                Column(Modifier.fillMaxWidth().background(Color(0xFF1C1610), RoundedCornerShape(12.dp)).padding(12.dp), verticalArrangement = Arrangement.spacedBy(7.dp)) {
                    Text("17 STANDALONE REFERENCE SKETCHES", color = Amber, fontSize = 12.sp, fontWeight = FontWeight.Black)
                    Text("These were deliberately never compiled into CubeFXWeb. They require their own Arduino upload and are listed below as reference, not fake play controls.", color = Muted, fontSize = 12.sp)
                    Button(onClick = { context.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://github.com/Mystereon/5x5x5-skeletoncube-patterns/tree/main/patterns"))) }, modifier = Modifier.fillMaxWidth(), colors = ButtonDefaults.buttonColors(containerColor = Amber, contentColor = Color.Black)) { Text("OPEN STANDALONE SKETCHES", fontWeight = FontWeight.Black) }
                }
            }
            items(CubeFxPatternCatalog.standaloneModes, key = { "standalone-${it.id}" }) { pattern ->
                Row(Modifier.fillMaxWidth().background(Color(0xFF17140E), RoundedCornerShape(12.dp)).padding(12.dp), horizontalArrangement = Arrangement.SpaceBetween, verticalAlignment = Alignment.CenterVertically) {
                    Text("${pattern.id.toString().padStart(2, '0')}  ${pattern.title}", color = Muted, fontWeight = FontWeight.Bold)
                    Text("UPLOAD .INO", color = Amber, fontSize = 10.sp, fontWeight = FontWeight.Black)
                }
            }
        }
    }
}

@Composable
private fun FirmwarePage() {
    val context = LocalContext.current
    Column(Modifier.verticalScroll(rememberScrollState()), verticalArrangement = Arrangement.spacedBy(12.dp)) {
        Text("CUBEFX FIRMWARE", color = TextMain, fontSize = 22.sp, fontWeight = FontWeight.Black)
        NoticeCard("CUBEFXWEB", "Download the current ESP32-S3 Zero Wi-Fi + BLE firmware sketch. It includes 46 embedded CubeFX modes, a 125-voxel cube plus 12-pixel rear ring, live phone-audio visualisers, configurable button pins, and the recommended Huge APP no-OTA profile.")
        ActionButton("OPEN CUBEFXWEB FIRMWARE", Lime) { context.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://github.com/Mystereon/5x5x5-skeletoncube-patterns/tree/main/standalone/CubeFXWeb"))) }
        ActionButton("OPEN CURRENT RELEASE", Cyan) { context.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://github.com/Mystereon/5x5x5-skeletoncube-patterns/releases/latest"))) }
        Text("Flash checklist: 1) download the repository or release source; 2) open CubeFXWeb.ino in Arduino IDE with the whole CubeFXWeb folder intact; 3) install FastLED and select ESP32S3 Dev Module, 4 MB flash, Huge APP, and QSPI PSRAM; 4) use GPIO6 data, GPIO2 primary, and GPIO4 secondary; 5) edit CubeFXConfig.h only when changing physical wiring; 6) upload by USB, then pair this controller with CubeFX-5x5x5.", color = Muted, fontSize = 14.sp)
        Text("Controller modes 57–63 add Ring Bouncer, Hologram, Voxel World Explorer, two Audio Link visualisers, Cloud-Top Rain, and Rotating Gold O. The app can save button-pin choices over BLE, but LED data-pin and cube-dimension changes require editing CubeFXConfig.h and reflashing.", color = Amber, fontSize = 13.sp, fontWeight = FontWeight.Bold)
    }
}

@Composable
private fun SetupPage(cubeFx: CubeFxBleClient) {
    val pins = listOf(0, 1, 2, 3, 4, 5, 6, 7, 10, 11, 20, 21)
    var dataPin by remember { mutableIntStateOf(6) }
    var primaryPin by remember { mutableIntStateOf(2) }
    var secondaryPin by remember { mutableIntStateOf(4) }
    var columns by remember { mutableIntStateOf(5) }
    var rows by remember { mutableIntStateOf(5) }
    var layers by remember { mutableIntStateOf(5) }
    val total = columns * rows * layers
    val validPins = primaryPin != secondaryPin && primaryPin != dataPin && secondaryPin != dataPin
    Column(Modifier.verticalScroll(rememberScrollState()), verticalArrangement = Arrangement.spacedBy(10.dp)) {
        Text("ESP32 SETUP", color = TextMain, fontWeight = FontWeight.Black, fontSize = 22.sp)
        Text("Button pin choices are sent to a connected cube and persisted. LED data pin and dimensions require a reflash.", color = Muted, fontSize = 13.sp)
        PinPicker("LED DATA PIN", dataPin, pins) { dataPin = it }
        PinPicker("PRIMARY BUTTON", primaryPin, pins) { primaryPin = it }
        PinPicker("SECONDARY BUTTON", secondaryPin, pins) { secondaryPin = it }
        if (!validPins) Text("Pins must be different. Do not share a button pin with LED data.", color = Warning, fontWeight = FontWeight.Bold)
        NoticeCard("BOOT WARNING", "GPIO2, GPIO8, and GPIO9 are boot strapping pins. Keep a switch on one of these pins released at reset. Avoid GPIO12–17 (flash) and GPIO18–19 if you need USB-JTAG.")
        DimensionPicker("COLUMNS", columns) { columns = it }
        DimensionPicker("ROWS", rows) { rows = it }
        DimensionPicker("LAYERS", layers) { layers = it }
        NoticeCard("LED TOTAL", "$columns columns × $rows rows × $layers layers = $total LEDs")
        Text("#define CUBEFX_LED_DATA_PIN $dataPin\n#define CUBEFX_PRIMARY_BUTTON_PIN $primaryPin\n#define CUBEFX_SECONDARY_BUTTON_PIN $secondaryPin\n#define CUBEFX_COLUMNS $columns\n#define CUBEFX_ROWS $rows\n#define CUBEFX_LAYERS $layers", color = Cyan, fontSize = 12.sp, modifier = Modifier.background(Color(0xFF071008)).padding(12.dp))
        ActionButton("APPLY BUTTON PINS TO CUBE", Lime) { if (validPins) cubeFx.sendPins(dataPin, primaryPin, secondaryPin) }
    }
}

@Composable
private fun WatchPage() {
    val context = LocalContext.current
    Column(Modifier.verticalScroll(rememberScrollState()), verticalArrangement = Arrangement.spacedBy(12.dp)) {
        Text("GALAXY WATCH", color = TextMain, fontSize = 22.sp, fontWeight = FontWeight.Black)
        NoticeCard("WATCH8 CLASSIC REMOTE", "Turn the physical bezel to browse embedded CubeFX modes. Tap to send a mode, then use touch controls for brightness, speed, Auto/Manual, Banner, and Pong left/right.")
        ActionButton("GET GALAXY WATCH CONTROLLER", Cyan) { context.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://github.com/Mystereon/5x5x5-skeletoncube-patterns/releases/latest"))) }
        Text("Install the Watch8 Classic APK from the release page, then pair directly with CubeFX-5x5x5 over BLE.", color = Muted, fontSize = 14.sp)
    }
}

@Composable
private fun EasterEggsPage(cubeFx: CubeFxBleClient) {
    Column(Modifier.verticalScroll(rememberScrollState()), verticalArrangement = Arrangement.spacedBy(10.dp)) {
        Text("EASTER EGGS", color = TextMain, fontSize = 22.sp, fontWeight = FontWeight.Black)
        NoticeCard("OWNER CONTROL DECK", "No ritual needed here: connect to CubeFX and tap a scene to play it immediately. The original owner routes remain available as a backup.")
        SecretSceneButton("01  RED EYE", "Small red cube apparition with matching ring pulse.", Warning) { cubeFx.sendSecretScene(0) }
        SecretSceneButton("02  WARM PULSE", "Ring-only slow warm pulse; all cube voxels stay dark.", Amber) { cubeFx.sendSecretScene(1) }
        SecretSceneButton("03  POCKET CLOCK", "Ring-only amber and cyan clock face.", Cyan) { cubeFx.sendSecretScene(2) }
        SecretSceneButton("04  COUNTDOWN", "Five-second ring countdown and release flash.", Lime) { cubeFx.sendSecretScene(3) }
        SecretSceneButton("05  EXPLOSION BURST", "Orange impact waves race around the acrylic ring.", Warning) { cubeFx.sendSecretScene(4) }
        NoticeCard("OWNER’S ACTIVATION GUIDE", "1. Red Eye: send Banner text OPEN EYE.  2. Warm Pulse: tap the physical primary button four times within about one second.  3. Pocket Clock: select Vector Cube, Matrix Rain, Vector Cube, Matrix Rain in that order.  4. Countdown: apply engine Brightness 42 and Speed 42.  5. Explosion Burst: leave manual Cloud Volume running for four minutes. These routes are backups; ordinary players should use the trigger buttons above.")
    }
}

@Composable
private fun ResourcesPage() {
    val context = LocalContext.current
    Column(Modifier.verticalScroll(rememberScrollState()), verticalArrangement = Arrangement.spacedBy(12.dp)) {
        Text("USEFUL LINKS", color = TextMain, fontSize = 22.sp, fontWeight = FontWeight.Black)
        NoticeCard("CUBEFX PROJECT", "Firmware, direct-upload patterns, Android and Watch sources, wiring notes, release APKs, and the public build history.")
        ActionButton("OPEN GITHUB REPOSITORY", Lime) { context.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://github.com/Mystereon/5x5x5-skeletoncube-patterns"))) }
        ActionButton("OPEN LATEST RELEASE", Cyan) { context.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://github.com/Mystereon/5x5x5-skeletoncube-patterns/releases/latest"))) }
        ActionButton("OPEN WIRING GUIDE", Amber) { context.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://github.com/Mystereon/5x5x5-skeletoncube-patterns/blob/main/standalone/CubeFXWeb/ESP32_S3_ZERO_WIRING.md"))) }
        NoticeCard("SUPPORT THE PROJECT", "FEED ME, I’M POOR AND I MADE THIS FOR FREE. Thank you for supporting open CubeFX development.")
        ActionButton("OPEN SUPPORT LINK", Warning) { context.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://paypal.me/Mystereon"))) }
    }
}

@Composable
private fun PinPicker(label: String, value: Int, choices: List<Int>, onValue: (Int) -> Unit) {
    val current = choices.indexOf(value).coerceAtLeast(0)
    Row(Modifier.fillMaxWidth().background(Panel, RoundedCornerShape(12.dp)).padding(12.dp), verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.SpaceBetween) {
        Text(label, color = TextMain, fontWeight = FontWeight.Bold)
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp), verticalAlignment = Alignment.CenterVertically) {
            StepButton("−") { onValue(choices[Math.floorMod(current - 1, choices.size)]) }
            Text("GPIO$value", color = Lime, fontWeight = FontWeight.Black, modifier = Modifier.width(58.dp))
            StepButton("+") { onValue(choices[(current + 1) % choices.size]) }
        }
    }
}

@Composable
private fun DimensionPicker(label: String, value: Int, onValue: (Int) -> Unit) {
    Row(Modifier.fillMaxWidth().background(Panel, RoundedCornerShape(12.dp)).padding(12.dp), verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.SpaceBetween) {
        Text(label, color = TextMain, fontWeight = FontWeight.Bold)
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp), verticalAlignment = Alignment.CenterVertically) {
            StepButton("−") { onValue((value - 1).coerceAtLeast(1)) }
            Text(value.toString(), color = Lime, fontWeight = FontWeight.Black, modifier = Modifier.width(30.dp))
            StepButton("+") { onValue((value + 1).coerceAtMost(64)) }
        }
    }
}

@Composable
private fun NavButton(label: String, active: Boolean, onClick: () -> Unit) = Button(onClick = onClick, modifier = Modifier.fillMaxWidth(), colors = ButtonDefaults.buttonColors(containerColor = if (active) Lime else Panel, contentColor = if (active) Color.Black else TextMain)) { Text(label, fontSize = 10.sp, fontWeight = FontWeight.Black) }

@Composable
private fun StepButton(label: String, onClick: () -> Unit) = Text(label, color = Color.Black, fontSize = 18.sp, fontWeight = FontWeight.Black, modifier = Modifier.background(Lime, RoundedCornerShape(8.dp)).clickable(onClick = onClick).padding(horizontal = 11.dp, vertical = 3.dp))

@Composable
private fun ActionButton(label: String, colour: Color, onClick: () -> Unit) = Button(onClick = onClick, modifier = Modifier.fillMaxWidth(), colors = ButtonDefaults.buttonColors(containerColor = colour, contentColor = Color.Black)) { Text(label, fontWeight = FontWeight.Black) }

@Composable
private fun RowScope.MiniActionButton(label: String, colour: Color, textColour: Color, onClick: () -> Unit) = Button(onClick = onClick, modifier = Modifier.weight(1f), colors = ButtonDefaults.buttonColors(containerColor = colour, contentColor = textColour)) { Text(label, fontSize = 10.sp, fontWeight = FontWeight.Black) }

@Composable
private fun SecretSceneButton(title: String, body: String, colour: Color, onClick: () -> Unit) = Column(Modifier.fillMaxWidth().background(Panel, RoundedCornerShape(14.dp)).padding(12.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
    Text(title, color = colour, fontSize = 14.sp, fontWeight = FontWeight.Black)
    Text(body, color = TextMain, fontSize = 13.sp)
    Button(onClick = onClick, modifier = Modifier.fillMaxWidth(), colors = ButtonDefaults.buttonColors(containerColor = colour, contentColor = Color.Black)) { Text("TRIGGER NOW", fontWeight = FontWeight.Black) }
}

@Composable
private fun NoticeCard(title: String, body: String) = Column(Modifier.fillMaxWidth().background(Panel, RoundedCornerShape(14.dp)).padding(14.dp), verticalArrangement = Arrangement.spacedBy(4.dp)) { Text(title, color = Amber, fontSize = 12.sp, fontWeight = FontWeight.Black); Text(body, color = TextMain, fontSize = 14.sp) }
