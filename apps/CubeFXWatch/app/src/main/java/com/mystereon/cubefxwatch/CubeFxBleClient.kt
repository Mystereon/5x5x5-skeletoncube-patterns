package com.mystereon.cubefxwatch

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
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

enum class CubeConnection { DISCONNECTED, SCANNING, CONNECTING, READY, ERROR }

class CubeFxBleClient(private val context: Context) {
    private val bluetoothManager = context.getSystemService(BluetoothManager::class.java)
    private val adapter: BluetoothAdapter? get() = bluetoothManager?.adapter
    private var gatt: BluetoothGatt? = null
    private var commandCharacteristic: BluetoothGattCharacteristic? = null

    private val _connection = MutableStateFlow(CubeConnection.DISCONNECTED)
    val connection: StateFlow<CubeConnection> = _connection
    private val _message = MutableStateFlow("Turn bezel to browse once connected")
    val message: StateFlow<String> = _message

    @SuppressLint("MissingPermission")
    fun startScan() {
        val scanner = adapter?.bluetoothLeScanner
        if (scanner == null || adapter?.isEnabled != true) {
            _connection.value = CubeConnection.ERROR
            _message.value = "Bluetooth is unavailable"
            return
        }
        _connection.value = CubeConnection.SCANNING
        _message.value = "Looking for CubeFX-5x5x5"
        val filters = listOf(ScanFilter.Builder().setServiceUuid(ParcelUuid(CubeFxProtocol.serviceUuid)).build())
        val settings = ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build()
        scanner.startScan(filters, settings, scanCallback)
    }

    @SuppressLint("MissingPermission")
    fun stopScan() {
        adapter?.bluetoothLeScanner?.stopScan(scanCallback)
        if (_connection.value == CubeConnection.SCANNING) _connection.value = CubeConnection.DISCONNECTED
    }

    @SuppressLint("MissingPermission")
    private fun connect(device: BluetoothDevice) {
        stopScan()
        _connection.value = CubeConnection.CONNECTING
        _message.value = "Connecting to ${device.name ?: "CubeFX"}"
        gatt?.close()
        gatt = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            device.connectGatt(context, false, gattCallback, BluetoothDevice.TRANSPORT_LE)
        } else {
            device.connectGatt(context, false, gattCallback)
        }
    }

    @SuppressLint("MissingPermission")
    fun send(json: String) {
        val activeGatt = gatt
        val characteristic = commandCharacteristic
        if (activeGatt == null || characteristic == null || _connection.value != CubeConnection.READY) {
            _message.value = "Connect to CubeFX first"
            return
        }
        val bytes = json.toByteArray(StandardCharsets.UTF_8)
        val accepted = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            activeGatt.writeCharacteristic(characteristic, bytes, BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE) == BluetoothGatt.GATT_SUCCESS
        } else {
            characteristic.writeType = BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
            characteristic.value = bytes
            @Suppress("DEPRECATION") activeGatt.writeCharacteristic(characteristic)
        }
        _message.value = if (accepted) "Sent to CubeFX" else "CubeFX write failed"
    }

    @SuppressLint("MissingPermission")
    fun close() {
        stopScan()
        gatt?.close()
        gatt = null
        commandCharacteristic = null
        _connection.value = CubeConnection.DISCONNECTED
    }

    private val scanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            val name = result.device.name ?: result.scanRecord?.deviceName.orEmpty()
            if (name.startsWith(CubeFxProtocol.devicePrefix)) connect(result.device)
        }

        override fun onScanFailed(errorCode: Int) {
            _connection.value = CubeConnection.ERROR
            _message.value = "BLE scan error $errorCode"
        }
    }

    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            if (status == BluetoothGatt.GATT_SUCCESS && newState == BluetoothProfile.STATE_CONNECTED) {
                _message.value = "Discovering CubeFX service"
                @SuppressLint("MissingPermission") gatt.discoverServices()
            } else {
                commandCharacteristic = null
                _connection.value = CubeConnection.DISCONNECTED
                _message.value = "CubeFX disconnected"
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            commandCharacteristic = gatt.getService(CubeFxProtocol.serviceUuid)
                ?.getCharacteristic(CubeFxProtocol.commandUuid)
            if (commandCharacteristic == null) {
                _connection.value = CubeConnection.ERROR
                _message.value = "CubeFX service not found"
            } else {
                _connection.value = CubeConnection.READY
                _message.value = "CubeFX ready"
            }
        }
    }
}
