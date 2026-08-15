package com.mystereon.cubefxcontroller

import android.Manifest
import android.content.Intent
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
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.MaterialTheme
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
    private val permissionRequest = registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) { result ->
        if (result.values.all { it }) cubeFx.scan()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        cubeFx = CubeFxBleClient(applicationContext)
        setContent { CubeFxPhoneApp(cubeFx) { permissionRequest.launch(arrayOf(Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT)) } }
    }

    override fun onDestroy() { cubeFx.close(); super.onDestroy() }
}

private enum class PhonePage { LIVE, PATTERNS, SETUP, FIRMWARE, WATCH }

@Composable
private fun CubeFxPhoneApp(cubeFx: CubeFxBleClient, requestBluetooth: () -> Unit) {
    var page by remember { mutableStateOf(PhonePage.LIVE) }
    val state by cubeFx.state.collectAsStateWithLifecycle()
    val notice by cubeFx.notice.collectAsStateWithLifecycle()
    Column(Modifier.fillMaxSize().background(Graphite).padding(18.dp), verticalArrangement = Arrangement.spacedBy(14.dp)) {
        Text("CUBE.FX", color = Lime, fontWeight = FontWeight.Black, fontSize = 15.sp, letterSpacing = 2.sp)
        Text("${state.name}  ·  $notice", color = if (state == PhoneConnection.READY) Cyan else Muted, fontSize = 12.sp)
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            NavButton("LIVE", page == PhonePage.LIVE) { page = PhonePage.LIVE }
            NavButton("PATTERNS", page == PhonePage.PATTERNS) { page = PhonePage.PATTERNS }
            NavButton("ESP32", page == PhonePage.SETUP) { page = PhonePage.SETUP }
            NavButton("FW", page == PhonePage.FIRMWARE) { page = PhonePage.FIRMWARE }
            NavButton("WATCH", page == PhonePage.WATCH) { page = PhonePage.WATCH }
        }
        when (page) {
            PhonePage.LIVE -> LivePage(cubeFx, requestBluetooth)
            PhonePage.PATTERNS -> PatternPage(cubeFx)
            PhonePage.SETUP -> SetupPage(cubeFx)
            PhonePage.FIRMWARE -> FirmwarePage()
            PhonePage.WATCH -> WatchPage()
        }
    }
}

@Composable
private fun LivePage(cubeFx: CubeFxBleClient, requestBluetooth: () -> Unit) {
    var brightness by remember { mutableIntStateOf(100) }
    var speed by remember { mutableIntStateOf(150) }
    var auto by remember { mutableStateOf(true) }
    var banner by remember { mutableStateOf("CUBE 4 3 2 1 0") }
    Column(Modifier.verticalScroll(rememberScrollState()), verticalArrangement = Arrangement.spacedBy(12.dp)) {
        Text("Your cube, on hand.", color = TextMain, fontSize = 30.sp, fontWeight = FontWeight.Black)
        Text("Connect to CubeFX-5x5x5 to apply patterns, Banner controls, engine settings, and ESP32 button-pin changes.", color = Muted, fontSize = 15.sp)
        ActionButton("SCAN FOR CUBEFX", Lime, requestBluetooth)
        NoticeCard("ENGINE", "Brightness $brightness  ·  Speed $speed  ·  ${if (auto) "AUTO" else "MANUAL"}")
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            StepButton("B−") { brightness = (brightness - 16).coerceAtLeast(1) }
            StepButton("B+") { brightness = (brightness + 16).coerceAtMost(255) }
            StepButton("S−") { speed = (speed - 16).coerceAtLeast(1) }
            StepButton("S+") { speed = (speed + 16).coerceAtMost(255) }
            StepButton(if (auto) "AUTO" else "MANUAL") { auto = !auto }
        }
        ActionButton("APPLY ENGINE", Lime) { cubeFx.sendEngine(brightness, speed, auto) }
        TextField(value = banner, onValueChange = { banner = it.take(60) }, label = { Text("Banner text") }, modifier = Modifier.fillMaxWidth())
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            ActionButton("SEND BANNER", Cyan) { cubeFx.sendBanner(banner) }
            ActionButton("PRIMARY", Amber) { cubeFx.sendAction(true) }
            ActionButton("SECONDARY", Amber) { cubeFx.sendAction(false) }
        }
        NoticeCard("GALAXY WATCH", "The Watch8 Classic controller uses its physical bezel to browse CubeFX modes. Open WATCH when its APK release is available.")
    }
}

@Composable
private fun PatternPage(cubeFx: CubeFxBleClient) {
    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
        Text("54 PATTERNS", color = TextMain, fontWeight = FontWeight.Black, fontSize = 22.sp)
        Text("Tap an EMBEDDED mode to send it to CubeFXWeb. UPLOAD DEMO effects remain direct-upload sketches in the public library.", color = Muted, fontSize = 13.sp)
        LazyColumn(verticalArrangement = Arrangement.spacedBy(7.dp)) {
            items(CubeFxPatternCatalog.all, key = { it.id }) { pattern ->
                Row(
                    modifier = Modifier.fillMaxWidth().background(Panel, RoundedCornerShape(12.dp)).clickable { if (pattern.embedded) cubeFx.sendPattern(pattern.id) }.padding(12.dp),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text("${pattern.id.toString().padStart(2, '0')}  ${pattern.title}", color = if (pattern.embedded) TextMain else Muted, fontWeight = FontWeight.Bold)
                    Text(if (pattern.embedded) "EMBEDDED" else "UPLOAD DEMO", color = if (pattern.embedded) Lime else Amber, fontSize = 10.sp, fontWeight = FontWeight.Black)
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
        NoticeCard("CUBEFXWEB", "Download the current ESP32-C3 Wi-Fi + BLE firmware sketch. It includes the 37 embedded CubeFX modes, configurable data/button pins, and the recommended maximum-app partition profile.")
        ActionButton("OPEN CUBEFXWEB FIRMWARE", Lime) {
            context.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://github.com/Mystereon/5x5x5-skeletoncube-patterns/tree/main/standalone/CubeFXWeb")))
        }
        ActionButton("OPEN CURRENT RELEASE", Cyan) {
            context.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://github.com/Mystereon/5x5x5-skeletoncube-patterns/releases/latest")))
        }
        Text("Flash checklist: 1) download the repository or release source; 2) open CubeFXWeb.ino in Arduino IDE with the whole CubeFXWeb folder intact; 3) install FastLED and select ESP32C3 Dev Module; 4) retain the supplied partitions.csv for the 3.94 MiB maximum-app profile; 5) edit CubeFXConfig.h only when changing physical wiring; 6) upload by USB, then pair this controller with CubeFX-5x5x5.", color = Muted, fontSize = 14.sp)
        Text("The app can save button-pin choices over BLE. LED data-pin and cube-dimension changes are firmware changes, so they require editing CubeFXConfig.h and reflashing.", color = Amber, fontSize = 13.sp, fontWeight = FontWeight.Bold)
    }
}

@Composable
private fun SetupPage(cubeFx: CubeFxBleClient) {
    val pins = listOf(0, 1, 2, 3, 4, 5, 6, 7, 10, 11, 20, 21)
    var dataPin by remember { mutableIntStateOf(2) }
    var primaryPin by remember { mutableIntStateOf(4) }
    var secondaryPin by remember { mutableIntStateOf(8) }
    var columns by remember { mutableIntStateOf(5) }
    var rows by remember { mutableIntStateOf(5) }
    var layers by remember { mutableIntStateOf(5) }
    val total = columns * rows * layers
    val validPins = primaryPin != secondaryPin && primaryPin != dataPin && secondaryPin != dataPin
    val scroll = rememberScrollState()
    Column(Modifier.verticalScroll(scroll), verticalArrangement = Arrangement.spacedBy(10.dp)) {
        Text("ESP32 SETUP", color = TextMain, fontWeight = FontWeight.Black, fontSize = 22.sp)
        Text("Choose the wiring profile. Button pin choices are sent to a connected cube and persisted. LED data pin and dimensions generate a header profile and require a reflash.", color = Muted, fontSize = 13.sp)
        PinPicker("LED DATA PIN", dataPin, pins) { dataPin = it }
        PinPicker("PRIMARY BUTTON", primaryPin, pins) { primaryPin = it }
        PinPicker("SECONDARY BUTTON", secondaryPin, pins) { secondaryPin = it }
        if (!validPins) Text("Pins must be different. Do not share a button pin with LED data.", color = Warning, fontWeight = FontWeight.Bold)
        NoticeCard("BOOT WARNING", "GPIO2, GPIO8, and GPIO9 are boot strapping pins. Keep a switch on one of these pins released at reset. Avoid GPIO12–17 (flash) and GPIO18–19 if you need USB-JTAG.")
        DimensionPicker("COLUMNS", columns) { columns = it }
        DimensionPicker("ROWS", rows) { rows = it }
        DimensionPicker("LAYERS", layers) { layers = it }
        NoticeCard("LED TOTAL", "$columns columns × $rows rows × $layers layers = $total LEDs")
        if (columns != 5 || rows != 5 || layers != 5) NoticeCard("RENDERER LIMIT", "CubeFXWeb v0.5 patterns require 5×5×5. This total is correctly calculated, but a generic renderer is needed before compiling non-5×5×5 dimensions.")
        Text("#define CUBEFX_LED_DATA_PIN $dataPin\n#define CUBEFX_PRIMARY_BUTTON_PIN $primaryPin\n#define CUBEFX_SECONDARY_BUTTON_PIN $secondaryPin\n#define CUBEFX_COLUMNS $columns\n#define CUBEFX_ROWS $rows\n#define CUBEFX_LAYERS $layers\n#define CUBEFX_TOTAL_LEDS (CUBEFX_COLUMNS * CUBEFX_ROWS * CUBEFX_LAYERS)", color = Cyan, fontSize = 12.sp, modifier = Modifier.background(Color(0xFF071008)).padding(12.dp))
        ActionButton("APPLY BUTTON PINS TO CUBE", Lime) { if (validPins) cubeFx.sendPins(dataPin, primaryPin, secondaryPin) }
        Text("Changing the LED data pin or dimensions: copy the generated values into CubeFXConfig.h, then compile and upload CubeFXWeb again.", color = Amber, fontSize = 13.sp, fontWeight = FontWeight.Bold)
    }
}

@Composable
private fun WatchPage() {
    val context = LocalContext.current
    Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
        Text("GALAXY WATCH", color = TextMain, fontSize = 22.sp, fontWeight = FontWeight.Black)
        NoticeCard("WATCH8 CLASSIC REMOTE", "Turn the physical bezel to browse embedded CubeFX modes. Tap to send a mode, then use large touch controls for brightness, speed, Auto/Manual, Banner, and Pong left/right.")
        ActionButton("GET GALAXY WATCH CONTROLLER", Cyan) {
            context.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://github.com/Mystereon/5x5x5-skeletoncube-patterns/releases/latest")))
        }
        Text("The release page will host the Wear OS APK once the native watch build is signed. Install it on the Watch8 Classic from your phone or using Android Debug Bridge, then pair directly with CubeFX-5x5x5 over BLE.", color = Muted, fontSize = 14.sp)
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
private fun NavButton(label: String, active: Boolean, onClick: () -> Unit) = Button(onClick = onClick, colors = ButtonDefaults.buttonColors(containerColor = if (active) Lime else Panel, contentColor = if (active) Color.Black else TextMain)) { Text(label, fontWeight = FontWeight.Black) }

@Composable
private fun StepButton(label: String, onClick: () -> Unit) = Text(label, color = Color.Black, fontSize = 18.sp, fontWeight = FontWeight.Black, modifier = Modifier.background(Lime, RoundedCornerShape(8.dp)).clickable(onClick = onClick).padding(horizontal = 11.dp, vertical = 3.dp))

@Composable
private fun ActionButton(label: String, colour: Color, onClick: () -> Unit) = Button(onClick = onClick, modifier = Modifier.fillMaxWidth(), colors = ButtonDefaults.buttonColors(containerColor = colour, contentColor = Color.Black)) { Text(label, fontWeight = FontWeight.Black) }

@Composable
private fun NoticeCard(title: String, body: String) = Column(Modifier.fillMaxWidth().background(Panel, RoundedCornerShape(14.dp)).padding(14.dp), verticalArrangement = Arrangement.spacedBy(4.dp)) { Text(title, color = Amber, fontSize = 12.sp, fontWeight = FontWeight.Black); Text(body, color = TextMain, fontSize = 14.sp) }
