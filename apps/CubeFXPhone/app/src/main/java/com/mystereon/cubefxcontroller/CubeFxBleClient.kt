package com.mystereon.cubefxcontroller

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.os.Build
import android.os.ParcelUuid
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import java.nio.charset.StandardCharsets
import java.util.UUID

enum class PhoneConnection { OFFLINE, SCANNING, CONNECTING, READY, ERROR }

class CubeFxBleClient(private val context: Context) {
    private val manager = context.getSystemService(BluetoothManager::class.java)
    private val adapter: BluetoothAdapter? get() = manager?.adapter
    private var gatt: BluetoothGatt? = null
    private var command: BluetoothGattCharacteristic? = null
    private var status: BluetoothGattCharacteristic? = null
    private val _state = MutableStateFlow(PhoneConnection.OFFLINE)
    val state: StateFlow<PhoneConnection> = _state
    private val _notice = MutableStateFlow("Connect a CubeFX ESP32-S3")
    val notice: StateFlow<String> = _notice

    companion object {
        val serviceUuid: UUID = UUID.fromString("6c75a300-7b1d-4f29-a221-000000000001")
        val commandUuid: UUID = UUID.fromString("6c75a300-7b1d-4f29-a221-000000000002")
        val statusUuid: UUID = UUID.fromString("6c75a300-7b1d-4f29-a221-000000000003")
        val clientConfigUuid: UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")
    }

    @SuppressLint("MissingPermission")
    fun scan() {
        val scanner = adapter?.bluetoothLeScanner
        if (scanner == null || adapter?.isEnabled != true) {
            _state.value = PhoneConnection.ERROR
            _notice.value = "Bluetooth is unavailable"
            return
        }
        _state.value = PhoneConnection.SCANNING
        _notice.value = "Looking for CubeFX-5x5x5"
        val filter = ScanFilter.Builder().setServiceUuid(ParcelUuid(serviceUuid)).build()
        scanner.startScan(listOf(filter), ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build(), scanCallback)
    }

    @SuppressLint("MissingPermission")
    fun sendPins(dataPin: Int, primaryPin: Int, secondaryPin: Int) {
        send("{\"op\":\"pins\",\"dataPin\":$dataPin,\"primaryPin\":$primaryPin,\"secondaryPin\":$secondaryPin}")
    }

    @SuppressLint("MissingPermission")
    fun sendPattern(canonicalId: Int) = send("{\"op\":\"pattern\",\"id\":$canonicalId}")

    @SuppressLint("MissingPermission")
    fun sendEngine(brightness: Int, speed: Int, auto: Boolean) =
        send("{\"op\":\"engine\",\"brightness\":$brightness,\"speed\":$speed,\"cycle\":30,\"auto\":$auto}")

    @SuppressLint("MissingPermission")
    fun sendBanner(text: String, font: Int = 3, hue: Int = 96, speed: Int = 150) =
        send("{\"op\":\"banner\",\"text\":\"${text.replace("\"", " ")}\",\"font\":$font,\"hue\":$hue,\"speed\":$speed}")

    @SuppressLint("MissingPermission")
    fun sendAction(primary: Boolean) = send("{\"op\":\"action\",\"primary\":$primary}")

    /** Direct, discoverable next-pattern control for ordinary users. */
    @SuppressLint("MissingPermission")
    fun sendNextPattern() = send("{\"op\":\"next\"}")

    /** Deliberately triggers a short CubeFX hidden scene; no physical ritual is required. */
    @SuppressLint("MissingPermission")
    fun sendSecretScene(scene: Int) = send("{\"op\":\"secret\",\"scene\":${scene.coerceIn(0, 4)}}")

    /** Sends only the 13-byte analyser envelope; raw microphone PCM never leaves the phone. */
    @SuppressLint("MissingPermission")
    fun sendAudioSpectrum(packet: ByteArray) {
        if (packet.size != 13) return
        val active = gatt
        val target = command
        if (active == null || target == null || _state.value != PhoneConnection.READY) return
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            active.writeCharacteristic(target, packet, BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE)
        } else {
            target.writeType = BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
            target.value = packet
            @Suppress("DEPRECATION") active.writeCharacteristic(target)
        }
    }

    @SuppressLint("MissingPermission")
    fun send(json: String) {
        val active = gatt
        val target = command
        if (active == null || target == null || _state.value != PhoneConnection.READY) {
            _notice.value = "Connect to CubeFX before applying"
            return
        }
        val value = json.toByteArray(StandardCharsets.UTF_8)
        val ok = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            active.writeCharacteristic(target, value, BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE) == BluetoothGatt.GATT_SUCCESS
        } else {
            target.writeType = BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
            target.value = value
            @Suppress("DEPRECATION") active.writeCharacteristic(target)
        }
        _notice.value = if (ok) "Command sent — waiting for CubeFX acknowledgement" else "CubeFX write failed"
    }

    @SuppressLint("MissingPermission")
    fun close() { adapter?.bluetoothLeScanner?.stopScan(scanCallback); gatt?.close() }

    private val scanCallback = object : ScanCallback() {
        override fun onScanResult(type: Int, result: ScanResult) {
            val name = result.device.name ?: result.scanRecord?.deviceName.orEmpty()
            if (name.startsWith("CubeFX")) connect(result.device)
        }
        override fun onScanFailed(errorCode: Int) { _state.value = PhoneConnection.ERROR; _notice.value = "BLE scan error $errorCode" }
    }

    @SuppressLint("MissingPermission")
    private fun connect(device: BluetoothDevice) {
        adapter?.bluetoothLeScanner?.stopScan(scanCallback)
        _state.value = PhoneConnection.CONNECTING
        gatt = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) device.connectGatt(context, false, callback, BluetoothDevice.TRANSPORT_LE) else device.connectGatt(context, false, callback)
    }

    private val callback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            if (status == BluetoothGatt.GATT_SUCCESS && newState == BluetoothProfile.STATE_CONNECTED) {
                @SuppressLint("MissingPermission") gatt.discoverServices()
            } else { _state.value = PhoneConnection.OFFLINE; _notice.value = "CubeFX disconnected" }
        }
        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            val service = gatt.getService(serviceUuid)
            command = service?.getCharacteristic(commandUuid)
            this@CubeFxBleClient.status = service?.getCharacteristic(statusUuid)
            if (command == null) {
                _state.value = PhoneConnection.ERROR
                _notice.value = "CubeFX command service unavailable"
                return
            }
            command?.writeType = BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
            val statusCharacteristic = this@CubeFxBleClient.status
            if (statusCharacteristic != null && enableStatusNotifications(gatt, statusCharacteristic)) {
                _state.value = PhoneConnection.CONNECTING
                _notice.value = "Enabling CubeFX acknowledgements"
            } else {
                _state.value = PhoneConnection.READY
                _notice.value = "CubeFX ready (no acknowledgement channel)"
            }
        }

        override fun onDescriptorWrite(gatt: BluetoothGatt, descriptor: BluetoothGattDescriptor, status: Int) {
            if (descriptor.uuid != clientConfigUuid) return
            _state.value = PhoneConnection.READY
            _notice.value = if (status == BluetoothGatt.GATT_SUCCESS) "CubeFX ready — acknowledgements enabled" else "CubeFX ready (acknowledgements unavailable)"
        }

        override fun onCharacteristicWrite(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic, status: Int) {
            if (characteristic.uuid == commandUuid && status != BluetoothGatt.GATT_SUCCESS) _notice.value = "CubeFX write failed ($status)"
        }

        override fun onCharacteristicChanged(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic, value: ByteArray) {
            handleStatus(characteristic, value)
        }

        @Suppress("DEPRECATION")
        override fun onCharacteristicChanged(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic) {
            handleStatus(characteristic, characteristic.value ?: byteArrayOf())
        }
    }

    @SuppressLint("MissingPermission")
    private fun enableStatusNotifications(activeGatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic): Boolean {
        if (!activeGatt.setCharacteristicNotification(characteristic, true)) return false
        val descriptor = characteristic.getDescriptor(clientConfigUuid) ?: return false
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            activeGatt.writeDescriptor(descriptor, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE) == BluetoothGatt.GATT_SUCCESS
        } else {
            descriptor.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
            @Suppress("DEPRECATION") activeGatt.writeDescriptor(descriptor)
        }
    }

    private fun handleStatus(characteristic: BluetoothGattCharacteristic, value: ByteArray) {
        if (characteristic.uuid != statusUuid) return
        val payload = String(value, StandardCharsets.UTF_8)
        val message = Regex("\\\"message\\\":\\\"([^\\\"]*)").find(payload)?.groupValues?.getOrNull(1) ?: "status received"
        _notice.value = if (payload.contains("\"ok\":true")) "CubeFX ✓ $message" else "CubeFX rejected: $message"
    }
}
