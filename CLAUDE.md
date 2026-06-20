# Project: [Device Name] ESP32 Firmware

## Overview
Firmware for a single ESP32 board running the required logic for a flight simulator trim wheel wich is communicatin with Air Manager from SimInnovations. Built with PlatformIO using the Arduino framework.



## Hardware
- Board:ESP32-S3 base PD-Stepper board (see https://github.com/joshr120/PD-Stepper)
- Key peripherals:
  - Endstop S1 connected to gpio35
  - Endstop S2 connected to gpio36
  - Rotary HallSensor AS5600 conneced via i2c
  - TMC2209 Stepper driver connected to gpio5 for STEP, gpio6 for DIR and gpio2q for TMC_EN
  - Second Serial port connected to gpio13 for TX and gpio14 for RX. This port is to be used to communicate via the simessageport library with AirManager
- Pinout: see `docs/pinout.md` (create if it doesn't exist yet)

## Build / Flash / Monitor
```bash
pio run                     # build
pio run -t upload           # flash
pio device monitor          # serial monitor (115200 baud unless noted)
pio run -t upload -t monitor  # flash + monitor in one go
pio test                    # run unit tests (if present)
```

## Project Structure
- `src/` — application code (`main.cpp` + modules)
- `include/` — shared headers
- `lib/` — local libraries (project-specific, not from registry)
- `platformio.ini` — board config, framework, library dependencies
- `data/` — files for SPIFFS/LittleFS upload, if used

## Conventions
- [e.g. One class per peripheral driver, header + cpp split]
- [e.g. Use `Serial.printf` for debug logging, prefixed with module tag]
- [e.g. Non-blocking code only — no `delay()` in loop(), use millis()-based timing]
- [e.g. Config/secrets (WiFi creds, API keys) live in `include/secrets.h`, gitignored]

## Constraints
- Flash size: [e.g. 4MB, partition scheme: default / custom]
- RAM budget: [note if tight — relevant for buffers, String usage, etc.]
- [Any timing-critical sections, interrupt safety notes, power constraints]

## Do Not
- Commit `include/secrets.h` or any credentials
- [Anything else off-limits — e.g. "don't change the partition table without checking OTA impact"]

## Useful Context
- [Links to datasheets, related repos, or prior design notes]
- [Known quirks/bugs to watch for]
