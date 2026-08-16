package com.mystereon.cubefxwatch

import android.Manifest
import android.os.Bundle
import android.view.HapticFeedbackConstants
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.focusable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.wear.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.focus.FocusRequester
import androidx.compose.ui.focus.focusRequester
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.rotary.onRotaryScrollEvent
import androidx.compose.ui.platform.LocalView
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.compose.collectAsStateWithLifecycle

private val Graphite = Color(0xFF0B0D0B)
private val Panel = Color(0xFF121612)
private val Lime = Color(0xFFC8FF20)
private val Cyan = Color(0xFF32C8FF)
private val Amber = Color(0xFFFF7A16)
private val Muted = Color(0xFF92A08B)
private val Line = Color(0xFF51634C)

class MainActivity : ComponentActivity() {
    private lateinit var cubeFx: CubeFxBleClient
    private val requestBluetooth = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { granted ->
        if (granted.values.all { it }) cubeFx.startScan()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        cubeFx = CubeFxBleClient(applicationContext)
        setContent {
            CubeFxWatchApp(cubeFx) {
                requestBluetooth.launch(arrayOf(Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT))
            }
        }
    }

    override fun onDestroy() {
        cubeFx.close()
        super.onDestroy()
    }
}

private enum class WatchPage { PATTERNS, ENGINE, ACTIONS }
private enum class EngineTarget { BRIGHTNESS, SPEED }

@Composable
private fun CubeFxWatchApp(cubeFx: CubeFxBleClient, requestBluetooth: () -> Unit) {
    val connection by cubeFx.connection.collectAsStateWithLifecycle()
    val status by cubeFx.message.collectAsStateWithLifecycle()
    var page by remember { mutableStateOf(WatchPage.PATTERNS) }
    var patternIndex by remember { mutableIntStateOf(0) }
    var brightness by remember { mutableIntStateOf(100) }
    var speed by remember { mutableIntStateOf(150) }
    var auto by remember { mutableStateOf(true) }
    var engineTarget by remember { mutableStateOf(EngineTarget.BRIGHTNESS) }
    val focusRequester = remember { FocusRequester() }
    val view = LocalView.current

    fun hapticTick() = view.performHapticFeedback(HapticFeedbackConstants.CLOCK_TICK)
    fun applyEngine() = cubeFx.send(CubeFxProtocol.engineCommand(brightness, speed, auto))

    DisposableEffect(Unit) {
        focusRequester.requestFocus()
        onDispose { }
    }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Graphite)
            .focusRequester(focusRequester)
            .focusable()
            .onRotaryScrollEvent { event ->
                val direction = if (event.verticalScrollPixels > 0f) 1 else -1
                when (page) {
                    WatchPage.PATTERNS -> {
                        patternIndex = Math.floorMod(patternIndex + direction, CubeFxProtocol.embeddedPatterns.size)
                        hapticTick()
                    }
                    WatchPage.ENGINE -> {
                        if (engineTarget == EngineTarget.BRIGHTNESS) {
                            brightness = (brightness + direction * 5).coerceIn(5, 255)
                        } else {
                            speed = (speed + direction * 5).coerceIn(20, 255)
                        }
                        hapticTick()
                    }
                    WatchPage.ACTIONS -> hapticTick()
                }
                true
            },
        contentAlignment = Alignment.Center
    ) {
        Column(
            modifier = Modifier
                .size(196.dp)
                .clip(CircleShape)
                .border(1.dp, Line, CircleShape)
                .background(Panel)
                .padding(horizontal = 16.dp, vertical = 15.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.SpaceBetween
        ) {
            Header(connection, status)
            when (page) {
                WatchPage.PATTERNS -> PatternPicker(
                    pattern = CubeFxProtocol.embeddedPatterns[patternIndex],
                    onSelect = { cubeFx.send(CubeFxProtocol.patternCommand(CubeFxProtocol.embeddedPatterns[patternIndex])) },
                    onPrevious = { page = WatchPage.ACTIONS },
                    onNext = { page = WatchPage.ENGINE }
                )
                WatchPage.ENGINE -> EnginePanel(
                    brightness = brightness,
                    speed = speed,
                    auto = auto,
                    target = engineTarget,
                    onSelectBrightness = { engineTarget = EngineTarget.BRIGHTNESS },
                    onSelectSpeed = { engineTarget = EngineTarget.SPEED },
                    onBrightnessDelta = { brightness = (brightness + it).coerceIn(5, 255) },
                    onSpeedDelta = { speed = (speed + it).coerceIn(20, 255) },
                    onApply = { applyEngine() },
                    onToggleAuto = { auto = !auto; applyEngine() },
                    onPrevious = { page = WatchPage.PATTERNS },
                    onNext = { page = WatchPage.ACTIONS }
                )
                WatchPage.ACTIONS -> QuickActions(
                    onBanner = { cubeFx.send(CubeFxProtocol.patternCommand(CubeFxProtocol.embeddedPatterns.first { it.canonicalId == 29 })) },
                    onNext = { cubeFx.send(CubeFxProtocol.nextCommand()) },
                    onReseed = { cubeFx.send(CubeFxProtocol.reseedCommand()) },
                    onPongLeft = { cubeFx.send(CubeFxProtocol.actionCommand(true)) },
                    onPongRight = { cubeFx.send(CubeFxProtocol.actionCommand(false)) },
                    onPrevious = { page = WatchPage.ENGINE },
                    onNextPage = { page = WatchPage.PATTERNS }
                )
            }
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp), modifier = Modifier.fillMaxWidth()) {
                TinyControl("SCAN", Modifier.weight(1f)) { requestBluetooth() }
                TinyControl("${page.name}", Modifier.weight(1f)) { page = WatchPage.entries[(page.ordinal + 1) % WatchPage.entries.size] }
            }
        }
    }
}

@Composable
private fun Header(connection: CubeConnection, status: String) {
    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Text("CUBE.FX / WATCH", color = Lime, fontSize = 9.sp, fontWeight = FontWeight.Black, letterSpacing = 1.sp)
        Text(connection.name, color = if (connection == CubeConnection.READY) Cyan else Muted, fontSize = 9.sp, fontWeight = FontWeight.Bold)
        Text(status, color = Muted, fontSize = 8.sp, maxLines = 1, textAlign = TextAlign.Center)
    }
}

@Composable
private fun PatternPicker(pattern: CubeFxProtocol.Pattern, onSelect: () -> Unit, onPrevious: () -> Unit, onNext: () -> Unit) {
    Column(horizontalAlignment = Alignment.CenterHorizontally, verticalArrangement = Arrangement.spacedBy(4.dp)) {
        Text("BEZEL / BROWSE", color = Amber, fontSize = 9.sp, fontWeight = FontWeight.Bold)
        Text("${pattern.canonicalId.toString().padStart(2, '0')}", color = Lime, fontSize = 28.sp, fontWeight = FontWeight.Black)
        Text(pattern.title.uppercase(), color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Bold, textAlign = TextAlign.Center, lineHeight = 14.sp)
        TouchPill("TAP TO SEND", Lime, Color.Black, onSelect)
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            TinyControl("‹", Modifier.size(30.dp)) { onPrevious() }
            TinyControl("›", Modifier.size(30.dp)) { onNext() }
        }
    }
}

@Composable
private fun EnginePanel(
    brightness: Int,
    speed: Int,
    auto: Boolean,
    target: EngineTarget,
    onSelectBrightness: () -> Unit,
    onSelectSpeed: () -> Unit,
    onBrightnessDelta: (Int) -> Unit,
    onSpeedDelta: (Int) -> Unit,
    onApply: () -> Unit,
    onToggleAuto: () -> Unit,
    onPrevious: () -> Unit,
    onNext: () -> Unit
) {
    Column(horizontalAlignment = Alignment.CenterHorizontally, verticalArrangement = Arrangement.spacedBy(4.dp)) {
        Text("ENGINE / ${if (target == EngineTarget.BRIGHTNESS) "BRIGHT" else "SPEED"}", color = Amber, fontSize = 9.sp, fontWeight = FontWeight.Bold)
        Row(horizontalArrangement = Arrangement.spacedBy(4.dp)) {
            TouchPill("BRI $brightness", if (target == EngineTarget.BRIGHTNESS) Lime else Color.White, Color.Black, onSelectBrightness, Modifier.weight(1f))
            TouchPill("SPD $speed", if (target == EngineTarget.SPEED) Lime else Color.White, Color.Black, onSelectSpeed, Modifier.weight(1f))
        }
        Row(horizontalArrangement = Arrangement.spacedBy(4.dp)) {
            TouchPill("BRI −", Color.White, Color.Black, { onBrightnessDelta(-5) }, Modifier.weight(1f))
            TouchPill("BRI +", Color.White, Color.Black, { onBrightnessDelta(5) }, Modifier.weight(1f))
        }
        Row(horizontalArrangement = Arrangement.spacedBy(4.dp)) {
            TouchPill("SPD −", Color.White, Color.Black, { onSpeedDelta(-5) }, Modifier.weight(1f))
            TouchPill("SPD +", Color.White, Color.Black, { onSpeedDelta(5) }, Modifier.weight(1f))
        }
        TouchPill(if (auto) "AUTO ON" else "MANUAL", Cyan, Color.Black, onToggleAuto)
        TouchPill("APPLY", Lime, Color.Black, onApply)
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            TinyControl("‹", Modifier.size(30.dp)) { onPrevious() }
            TinyControl("›", Modifier.size(30.dp)) { onNext() }
        }
    }
}

@Composable
private fun QuickActions(onBanner: () -> Unit, onNext: () -> Unit, onReseed: () -> Unit, onPongLeft: () -> Unit, onPongRight: () -> Unit, onPrevious: () -> Unit, onNextPage: () -> Unit) {
    Column(horizontalAlignment = Alignment.CenterHorizontally, verticalArrangement = Arrangement.spacedBy(4.dp)) {
        Text("QUICK ACTIONS", color = Amber, fontSize = 9.sp, fontWeight = FontWeight.Bold)
        Row(horizontalArrangement = Arrangement.spacedBy(4.dp)) {
            TouchPill("BANNER", Lime, Color.Black, onBanner, Modifier.weight(1f))
            TouchPill("NEXT", Cyan, Color.Black, onNext, Modifier.weight(1f))
        }
        Row(horizontalArrangement = Arrangement.spacedBy(4.dp)) {
            TouchPill("PONG ◀", Color(0xFFFF7A16), Color.Black, onPongLeft, Modifier.weight(1f))
            TouchPill("PONG ▶", Color(0xFFFF7A16), Color.Black, onPongRight, Modifier.weight(1f))
        }
        TouchPill("LIFE RESEED", Color.White, Color.Black, onReseed)
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            TinyControl("‹", Modifier.size(30.dp)) { onPrevious() }
            TinyControl("›", Modifier.size(30.dp)) { onNextPage() }
        }
    }
}

@Composable
private fun TouchPill(label: String, background: Color, foreground: Color, onClick: () -> Unit, modifier: Modifier = Modifier) {
    Box(
        modifier = modifier
            .clip(CircleShape)
            .background(background)
            .clickable(onClick = onClick)
            .padding(horizontal = 9.dp, vertical = 7.dp),
        contentAlignment = Alignment.Center
    ) { Text(label, color = foreground, fontSize = 9.sp, fontWeight = FontWeight.Black, maxLines = 1) }
}

@Composable
private fun TinyControl(label: String, modifier: Modifier = Modifier, onClick: () -> Unit) {
    Box(
        modifier = modifier
            .clip(CircleShape)
            .border(1.dp, Line, CircleShape)
            .clickable(onClick = onClick)
            .padding(vertical = 5.dp),
        contentAlignment = Alignment.Center
    ) { Text(label, color = Muted, fontSize = 8.sp, fontWeight = FontWeight.Bold, maxLines = 1) }
}
