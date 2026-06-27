# Project: [Device Name] ESP32 Firmware

## Overview
Firmware for a single ESP32 board running the required logic for a flight simulator trim wheel wich is communicatin with Air Manager from SimInnovations. Built with PlatformIO using the Arduino framework.

To calibrate the trimwheel during setup:
Run the motor cw until endstop s1 is triggered, the run it ccw until endstop s2 ist triggered. During this second ccw phase use the rotary hall sensor to record the rotation needed between the 2 endstops. The motor axis will has sevaral full turn rotations top sweep the whole range, so the values stored for the rotation needs to account for that. In a third phase turn the motor cw again until the half of the previously measured rotation value in order to position the trimwheel in the center as a starting point.

A CW turn from the motor (as viewed when looking onto the axis) represents the "Nose Down" trim direction, consequently a CCW turn of the motor represents the "Node Up" trim.

The values from the Simulator range from 1 representing the full "Nose Up" position -0.86363636363636 representing the maximum "Nose Down" position, these values are specific for a Cessna 172. The sim value at middle postion of the trim wheel is 0.13636363636364

After the calibration is done, the firmware shall contnously monitor the values from the sim, then transpose that into the motor rotation values by using the min and max rotation values determined during calibration. That way the trim wheel follows the values from the simulator. After the trimwheel has reached the position as decribed above the motor shall be in freewheel mode allowing manual turning of the wheel.

The trim wheel can also be turned manually, if that happens the rotation value measured by the rotary hall sensor shall be transposed into the corresponding position value understood by the sim and send to the sim via the corresponding msfs event.

As an additional function the switch S3 is used to center the trim wheel. If presed the wheel returns to the center position ad like with a manual turn of the trim wheel this is relayed to the sim via the event.



## Hardware
- Board:ESP32-S3 base PD-Stepper board (see https://github.com/joshr120/PD-Stepper)
- Key peripherals:
  - Endstop S1 connected to gpio35
  - Endstop S2 connected to gpio36
  - Center Switch S3 connected to gpio37
  - Rotary HallSensor AS5600 conneced via i2c with the following pins:
    - SDA: gpio
    - SCL: gpio
  - TMC2209 Stepper driver connected to the following pins:
    - STEP: gpio5
    - DIR: gpio6
    - TMC_EN: gpio2
  - Second Serial port connected to gpio13 for TX and gpio14 for RX. This port is to be used to communicate via the simessageport library with AirManager



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

## Useful Context
- PD Stepper repo[Links to datasheets, related repos, or prior design notes]
- Sim Innovationm wiki page [Known quirks/bugs to watch for]

