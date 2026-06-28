# Project: TrimWheel ESP32 Firmware

> Implementation detail and diagrams: see [docs/ARCHITECTURE.md](../docs/ARCHITECTURE.md).

## Overview
Firmware for a single ESP32 board running the required logic for a flight simulator trim wheel wich is communicatin with Air Manager from SimInnovations. Built with PlatformIO using the Arduino framework.

During seuip the pd-stepper shall be configuered to use a voltage of 5V for the motor driver via the PD configuration pins. the motor driver should not be enebled until power is good at 5V

To calibrate the trimwheel during setup:
Run the motor cw until endstop s1 is triggered, the run it ccw until endstop s2 ist triggered. During this second ccw phase use the rotary hall sensor to record the rotation needed between the 2 endstops. The motor axis will has sevaral full turn rotations top sweep the whole range, so the values stored for the rotation needs to account for that. In a third phase turn the motor cw again until the half of the previously measured rotation value in order to position the trimwheel in the center as a starting point.

A CW turn from the motor (as viewed when looking onto the axis) represents the "Nose Down" trim direction, consequently a CCW turn of the motor represents the "Node Up" trim.

The values from the Simulator range from 1 representing the full "Nose Up" position -0.86363636363636 representing the maximum "Nose Down" position, these values are specific for a Cessna 172. The sim value at middle postion of the trim wheel is 0.13636363636364

After the calibration is done, the firmware shall contnously monitor the values from the sim, then transpose that into the motor rotation values by using the min and max rotation values determined during calibration. That way the trim wheel follows the values from the simulator. After the trimwheel has reached the position as decribed above the motor shall be in freewheel mode allowing manual turning of the wheel.

The trim wheel can also be turned manually, if that happens the rotation value measured by the rotary hall sensor shall be transposed into the corresponding position value understood by the sim and send to the sim via the corresponding msfs event.

As an additional function the switch S3 is used to center the trim wheel. If presed the wheel returns to the center position ad like with a manual turn of the trim wheel this is relayed to the sim via the event.

## Implementation status
The firmware is implemented as a state machine (`WaitPower -> CalibrateS1 -> CalibrateS2 -> CalibrateCenter -> FollowSim`, plus `Fault`) with one driver class per peripheral. Highlights:
- **Sim <-> position mapping** is piecewise-linear across the three calibration anchors (Nose Down / centre / Nose Up). The centre sim value (0.13636) is *not* the numeric midpoint of the extremes, so a single linear map would be wrong.
- **Positioning is closed-loop on the AS5600** cumulative (multi-turn) reading, not step counting — this handles the multi-turn motor↔wheel gearing without knowing the ratio.
- **Freewheel** = TMC2209 disabled (coils open) once a target is reached; manual turns past `MANUAL_MOVE_COUNTS` are detected and relayed to the sim. `_referenceSim` suppresses the sim's echo so the wheel doesn't chase itself.
- **S3 gestures**: short press centres; long press (>= `SWITCH_LONG_PRESS_MS`, 3 s) re-runs calibration (fires while held).
- **Status LEDs**: LED1 (gpio12) blinks fast on power-not-good, slow during calibration; LED2 (gpio10) holds ~50% (LEDC PWM) in normal operation. Polarity-checked first on PG, so a power loss reverts to the fault pattern.
- **Air Manager message IDs**: `MSG_TRIM_FROM_SIM = 1` (in), `MSG_TRIM_TO_SIM = 2` (out) — must match the Air Manager script.
- **Heartbeat**: a 1 Hz debug line (`state / PG / position / referenceSim / driving`) so the board is observable over the serial bridge while idle.

### Confirmed / still to verify
- Confirmed against hardware: `PD_POWER_GOOD_ACTIVE_LOW = true` (PG low = good); `TMC_R_SENSE = 0.10` Ω per coil.
- Still verify: `TMC_DIR_CW_LEVEL` (CW = Nose Down), `LED_ACTIVE_HIGH`, `INPUT_ACTIVE_LOW`, and the message IDs vs the AM script.
- Microstepping is `TMC_MICROSTEPS = 32`; speed constants are in steps/s and scaled to the microstep setting so physical RPM is independent of it.



## Hardware
- Board:ESP32-S3 base PD-Stepper board (see https://github.com/joshr120/PD-Stepper)
- Key peripherals:
  - Endstop S1 connected to gpio35
  - Endstop S2 connected to gpio36
  - Center Switch S3 connected to gpio37
  - Rotary HallSensor AS5600 conneced via i2c with the following pins:
    - SDA: gpio8
    - SCL: gpio9
  - Power Delivery configuration connected to the following pins
    - PG (power good) gpio15
    - CFG1 gpio38
    - CFG2 gpio48
    - CFG3 gpio47
  - TMC2209 Stepper driver connected to the following pins:
    - TMC_EN  gpio21
    - STEP    gpio5
    - DIR     gpio6
    - MS1     gpio1
    - MS2     gpio2
    - SPREAD  gpio7
    - TMC_TX  gpio17
    - TMC_RX  gpio18
    - DIAG    gpio16
    - INDEX   gpio11
  - Second Serial port connected to gpio13 for TX and gpio14 for RX. This port is to be used to communicate via the simessageport library with AirManager



## Build / Flash / Monitor
```bash
pio run                     # build
pio run -t upload           # flash
pio device monitor          # serial monitor (115200 baud)
pio run -t upload -t monitor  # flash + monitor in one go
pio test                    # run unit tests (if present)
```
- PlatformIO env is `[env:pd-stepper]` (board `esp32-s3-devkitc-1`).
- `upload_port` / `monitor_port` point to a remote **RFC2217** serial bridge (`rfc2217://192.168.1.228:4001`).
- Note: the workbench bridge's auto-reset is flash-only — after `upload` it lands in the ROM bootloader; the board needs a manual EN/RST tap to run the app. `pio device monitor` needs a TTY; for headless capture use a pyserial RFC2217 reader.

## Project Structure
- `src/` — application code:
  - `main.cpp` — `setup()` → `TrimWheel::begin()`, `loop()` → `TrimWheel::update()`
  - `TrimWheel.{h,cpp}` — top-level state machine / coordinator
  - `PowerDelivery.{h,cpp}` — CH224K 5 V request + power-good
  - `StepperDriver.{h,cpp}` — TMC2209 (UART) + non-blocking step generation + freewheel
  - `AngleSensor.{h,cpp}` — AS5600 multi-turn cumulative position
  - `Inputs.{h,cpp}` — S1/S2 limits + debounced S3 short/long press
  - `SimLink.{h,cpp}` — SiMessagePort wrapper (Air Manager)
  - `StatusLeds.{h,cpp}` — two-LED status indication
- `include/Config.h` — all pins, sim anchors, and tuning constants in one place
- `lib/SiMessagePort/` — vendored Air Manager message-port library
- `docs/ARCHITECTURE.md` — full design write-up + mermaid diagrams
- `platformio.ini` — board config, framework, library dependencies

## Conventions
- One class per peripheral driver, header + cpp split; `TrimWheel` is the only coordinator and owns the drivers (composition). Drivers don't know about each other.
- Every pin and tunable lives in `include/Config.h` — don't hard-code GPIOs or magic numbers elsewhere.
- `Serial.printf`/`println` for debug, prefixed with a module tag (e.g. `[TMC2209]`, `[TrimWheel]`, `[hb]`).
- Non-blocking only — no `delay()` in `loop()`; use `millis()`/`micros()` timing. The one allowed blocking call is the ~3 µs STEP pulse width in `StepperDriver::service()`.
- Drivers expose `begin()` + a per-loop service method; `TrimWheel::update()` calls them in a fixed order each loop.

## Constraints
- Board: ESP32-S3 (8 MB flash). Current usage ~10% flash, ~6% RAM — plenty of headroom.
- `Serial` = UART0 (debug), `Serial1` = TMC2209, `Serial2` = Air Manager. Keep these assignments.
- Top step-rate is bounded by loop frequency (one STEP pulse per `loop()`); the AS5600 I²C read is the dominant per-loop cost. Speed constants are set with margin below that bound.
- Motor must stay disabled until power is good at 5 V (safety requirement).

## Do Not
- Commit `include/secrets.h` or any credentials

## Useful Context
- PD Stepper repo: https://github.com/joshr120/PD-Stepper
- Sim Innovations message-port wiki: https://siminnovations.com/wiki/index.php?title=Hw_message_port_add
- Libraries: `teemuatlut/TMCStepper`, `robtillaart/AS5600` (see `platformio.ini`).
- Full architecture + diagrams: `docs/ARCHITECTURE.md`.

