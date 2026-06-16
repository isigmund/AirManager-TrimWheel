# SiMessagePort — local fork (configurable serial port)

This is a modified version of the [Sim Innovations SiMessagePort library](https://wiki.siminnovations.com/index.php?title=Arduino), version 4.2.0.

## What changed and why

The original library hardcodes `Serial` (UART0) for all communication with Air Manager / Air Player and calls `Serial.begin(115200)` internally. On an ESP32 this is the USB-connected port, which is inconvenient when you also want to use it for debug output via the Serial Monitor.

This fork makes the serial port **injectable**: you choose which `HardwareSerial` instance the library uses by passing a pointer to it in the constructor. The library no longer calls `begin()` — that is the caller's responsibility.

## Constructor signature change

Original:
```cpp
SiMessagePort(SiMessagePortDevice device, SiMessagePortChannel channel,
              void (*callback)(uint16_t, SiMessagePortPayload*))
```

Modified:
```cpp
SiMessagePort(SiMessagePortDevice device, SiMessagePortChannel channel,
              void (*callback)(uint16_t, SiMessagePortPayload*),
              HardwareSerial* serial_port)
```

## Migration guide

**Before:**
```cpp
void setup() {
    messagePort = new SiMessagePort(
        SI_MESSAGE_PORT_DEVICE_ESP32,
        SI_MESSAGE_PORT_CHANNEL_A,
        new_message_callback
    );
}
```

**After:**
```cpp
void setup() {
    Serial2.begin(115200);   // you call begin(), not the library
    messagePort = new SiMessagePort(
        SI_MESSAGE_PORT_DEVICE_ESP32,
        SI_MESSAGE_PORT_CHANNEL_A,
        new_message_callback,
        &Serial2             // pass any HardwareSerial instance
    );
}
```

## ESP32 serial port options

| Instance | Default RX | Default TX | Notes |
|----------|-----------|-----------|-------|
| `Serial`  | GPIO3  | GPIO1  | USB / UART0 — keep free for debug |
| `Serial1` | GPIO9  | GPIO10 | shares pins with flash — avoid |
| `Serial2` | GPIO16 | GPIO17 | recommended for Air Manager |

Pins can be remapped with `Serial2.begin(115200, SERIAL_8N1, rx_pin, tx_pin)`.

## Files changed

| File | Change |
|------|--------|
| `src/si_message_port_driver.h` | Added `void* serial_port` parameter to `si_message_port_driver_init()` |
| `src/si_message_port_driver.cpp` | Stores the injected `HardwareSerial*`; removed `Serial.begin()` call |
| `src/si_message_port.h` | Added `void* serial_port` parameter to `si_message_port_init()` |
| `src/si_message_port.c` | Forwards `serial_port` to the driver |
| `src/si_message_port.hpp` | Added `HardwareSerial* serial_port` parameter to `SiMessagePort` constructor |
| `src/si_message_port.cpp` | Forwards `serial_port` to `si_message_port_init()` |
| `examples/BasicExample/BasicExample.ino` | Updated to call `Serial2.begin()` and pass `&Serial2` |

All other files (`si_network`, `si_input_buffer`, `si_output_buffer`, `sim_extern_*`, etc.) are **unchanged**.
