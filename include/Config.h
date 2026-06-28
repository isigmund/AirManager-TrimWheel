#pragma once

// =============================================================================
//  Config.h — central hardware map and tuning parameters for the TrimWheel.
//
//  All pin assignments come from the project hardware description for the
//  ESP32-S3 based PD-Stepper board. Anything marked "VERIFY" depends on the
//  exact silicon / wiring and should be checked against the board schematic
//  before trusting it in the field.
// =============================================================================

#include <Arduino.h>

// -----------------------------------------------------------------------------
//  Power Delivery (CH224K USB-PD sink controller)
// -----------------------------------------------------------------------------
constexpr uint8_t PIN_PD_PG   = 15;  // Power Good (open-drain, active low)
constexpr uint8_t PIN_PD_CFG1 = 38;  // Voltage select bit 1
constexpr uint8_t PIN_PD_CFG2 = 48;  // Voltage select bit 2
constexpr uint8_t PIN_PD_CFG3 = 47;  // Voltage select bit 3

// CH224K voltage-select truth table (from datasheet). We request 5V.
//   5V : CFG1=1, CFG2=x, CFG3=x
//   9V : CFG1=0, CFG2=0, CFG3=0
//  12V : CFG1=0, CFG2=0, CFG3=1
//  15V : CFG1=0, CFG2=1, CFG3=1
//  20V : CFG1=0, CFG2=1, CFG3=0
constexpr uint8_t PD_CFG1_FOR_5V = HIGH;
constexpr uint8_t PD_CFG2_FOR_5V = LOW;
constexpr uint8_t PD_CFG3_FOR_5V = LOW;

// PG asserts LOW when the negotiated voltage is established. VERIFY polarity.
constexpr bool PD_POWER_GOOD_ACTIVE_LOW = true;
constexpr uint32_t PD_POWER_GOOD_TIMEOUT_MS = 5000;  // 0 = wait forever

// -----------------------------------------------------------------------------
//  TMC2209 stepper driver
// -----------------------------------------------------------------------------
constexpr uint8_t PIN_TMC_EN     = 21;  // Enable, active LOW
constexpr uint8_t PIN_TMC_STEP   = 5;
constexpr uint8_t PIN_TMC_DIR    = 6;
constexpr uint8_t PIN_TMC_MS1    = 1;   // UART slave-address bit 0
constexpr uint8_t PIN_TMC_MS2    = 2;   // UART slave-address bit 1
constexpr uint8_t PIN_TMC_SPREAD = 7;   // SpreadCycle / StealthChop select pin
constexpr uint8_t PIN_TMC_TX     = 17;  // ESP TX -> TMC PDN_UART (via 1k)
constexpr uint8_t PIN_TMC_RX     = 18;  // ESP RX <- TMC PDN_UART
constexpr uint8_t PIN_TMC_DIAG   = 16;
constexpr uint8_t PIN_TMC_INDEX  = 11;

// MS1/MS2 both LOW -> UART slave address 0.
constexpr uint8_t TMC_UART_ADDRESS = 0b00;

// Sense-resistor value on the PD-Stepper (100 mOhm on each coil, confirmed).
constexpr float TMC_R_SENSE = 0.10f;

constexpr uint16_t TMC_RMS_CURRENT_MA = 800;  // motion current
constexpr uint16_t TMC_MICROSTEPS     = 32;

// DIR pin level that produces a CW rotation (looking onto the motor axis).
// CW == "Nose Down". VERIFY against the physical wiring; flip if reversed.
constexpr uint8_t TMC_DIR_CW_LEVEL = HIGH;

// -----------------------------------------------------------------------------
//  AS5600 rotary hall sensor (I2C)
// -----------------------------------------------------------------------------
constexpr uint8_t PIN_AS5600_SDA = 8;
constexpr uint8_t PIN_AS5600_SCL = 9;
constexpr uint32_t AS5600_I2C_HZ = 400000;
constexpr int32_t  AS5600_COUNTS_PER_REV = 4096;  // 12-bit absolute angle

// -----------------------------------------------------------------------------
//  Endstops / switches (mechanical, wired to ground, internal pull-up)
// -----------------------------------------------------------------------------
constexpr uint8_t PIN_ENDSTOP_S1 = 35;  // CW limit  (max Nose Down)
constexpr uint8_t PIN_ENDSTOP_S2 = 36;  // CCW limit (max Nose Up)
constexpr uint8_t PIN_SWITCH_S3  = 37;  // momentary "center" button
constexpr bool    INPUT_ACTIVE_LOW = true;
constexpr uint32_t SWITCH_DEBOUNCE_MS = 30;
constexpr uint32_t SWITCH_LONG_PRESS_MS = 3000;  // S3 hold -> re-run calibration

// -----------------------------------------------------------------------------
//  Status LEDs
// -----------------------------------------------------------------------------
constexpr uint8_t PIN_LED1 = 12;  // status: blinks on fault / during calibration
constexpr uint8_t PIN_LED2 = 10;  // ready : steady 50% in normal operation
constexpr bool    LED_ACTIVE_HIGH = true;

constexpr uint32_t LED_BLINK_FAST_MS = 80;   // power-not-good (rapid)
constexpr uint32_t LED_BLINK_SLOW_MS = 400;  // calibrating (slow)

// LED2 PWM (LEDC) — 8-bit, 50% duty in normal operation.
constexpr uint8_t  LED2_PWM_CHANNEL    = 0;
constexpr uint32_t LED2_PWM_FREQ_HZ    = 5000;
constexpr uint8_t  LED2_PWM_RESOLUTION = 8;
constexpr uint32_t LED2_RUN_DUTY       = 128;  // ~50% of 255

// -----------------------------------------------------------------------------
//  Air Manager link (SiMessagePort over Serial2)
// -----------------------------------------------------------------------------
constexpr uint8_t PIN_AM_TX = 13;  // ESP TX -> Air Manager RX
constexpr uint8_t PIN_AM_RX = 14;  // ESP RX <- Air Manager TX
constexpr uint32_t AM_BAUD  = 115200;

// Message IDs shared with the Air Manager script. These MUST match the
// `hw_message_port_*` IDs used on the Air Manager side.
constexpr uint16_t MSG_TRIM_FROM_SIM = 1;  // in : current elevator trim position
constexpr uint16_t MSG_TRIM_TO_SIM   = 2;  // out: trim position set by the wheel

// -----------------------------------------------------------------------------
//  Cessna 172 elevator-trim sim-value calibration points
//  (provided by the project description)
// -----------------------------------------------------------------------------
constexpr float SIM_NOSE_DOWN = -0.86363636f;  // S1 / CW extreme
constexpr float SIM_CENTER    =  0.13636364f;  // mechanical centre of travel
constexpr float SIM_NOSE_UP   =  1.00000000f;  // S2 / CCW extreme

// -----------------------------------------------------------------------------
//  Motion tuning
// -----------------------------------------------------------------------------
// Speeds are in steps/s and scaled for 32 microsteps (2x the 16-microstep base)
// so physical RPM matches the original tuning.
constexpr float CALIB_SPEED_SPS   = 1200.0f;  // steps/s while seeking endstops
constexpr float DRIVE_MAX_SPS     = 3600.0f;  // steps/s cap while following sim
constexpr float DRIVE_MIN_SPS     = 300.0f;   // steps/s floor near the target
constexpr float DRIVE_KP_SPS      = 12.0f;    // steps/s per sensor count of error

// Closed-loop tolerances, expressed in AS5600 counts (4096 == one motor turn).
constexpr int32_t POS_DEADBAND_COUNTS    = 12;  // "on target" window
constexpr int32_t MANUAL_MOVE_COUNTS     = 40;  // freewheel move that counts as
                                                // a deliberate manual turn

// Manual over-travel handling: when a hand-turn runs the wheel onto a limit
// switch, drive it this far back toward centre before holding it with a brake,
// so the wheel is never pinned on the switch (freeing it from there needs a
// hard shove that mis-wraps the multi-turn count). Tunable; start small. If the
// back-off doesn't release the switch the limit handler simply steps off again.
constexpr float   ENDSTOP_BACKOFF_DEG    = 2.0f;
constexpr int32_t ENDSTOP_BACKOFF_COUNTS =
    (int32_t)(ENDSTOP_BACKOFF_DEG * AS5600_COUNTS_PER_REV / 360.0f + 0.5f);
// Sim-value change (absolute) large enough to command a follow move.
constexpr float SIM_FOLLOW_DEADBAND = 0.01f;

constexpr uint32_t CALIB_PHASE_TIMEOUT_MS = 20000;  // abort if an endstop never hits
