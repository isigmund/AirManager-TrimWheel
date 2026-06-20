# Project: [Device Name] ESP32 Firmware

## Overview
Firmware for a single ESP32 board running [brief purpose — e.g. "a capacitive-touch night light controller"].
Built with PlatformIO using the Arduino framework.

## Hardware
- Board: [e.g. ESP32-C6-DevKitC-1 / generic ESP32-WROOM-32]
- Key peripherals: [e.g. CAP1188 touch controller via I2C, WS2812 LED strip, OLED display]
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
