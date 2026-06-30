#include "StepperDriver.h"

#include "Config.h"
#include "DebugLog.h"

// The TMC2209 talks UART on Serial1; the port is opened in begin().
StepperDriver::StepperDriver()
    : _driver(&Serial1, TMC_R_SENSE, TMC_UART_ADDRESS) {}

bool StepperDriver::begin() {
    pinMode(PIN_TMC_EN, OUTPUT);
    digitalWrite(PIN_TMC_EN, HIGH);  // start disabled (freewheel)
    _enabled = false;

    pinMode(PIN_TMC_STEP, OUTPUT);
    pinMode(PIN_TMC_DIR, OUTPUT);
    digitalWrite(PIN_TMC_STEP, LOW);
    digitalWrite(PIN_TMC_DIR, TMC_DIR_CW_LEVEL);

    // MS1/MS2 select UART slave address 0 when both are low.
    pinMode(PIN_TMC_MS1, OUTPUT);
    pinMode(PIN_TMC_MS2, OUTPUT);
    digitalWrite(PIN_TMC_MS1, (TMC_UART_ADDRESS & 0x01) ? HIGH : LOW);
    digitalWrite(PIN_TMC_MS2, (TMC_UART_ADDRESS & 0x02) ? HIGH : LOW);

    pinMode(PIN_TMC_SPREAD, OUTPUT);
    digitalWrite(PIN_TMC_SPREAD, LOW);  // let UART decide the chopper mode

    pinMode(PIN_TMC_DIAG, INPUT);
    pinMode(PIN_TMC_INDEX, INPUT);

    Serial1.begin(115200, SERIAL_8N1, PIN_TMC_RX, PIN_TMC_TX);

    _driver.begin();
    _driver.toff(5);                       // enable the chopper
    _driver.rms_current(TMC_RMS_CURRENT_MA);
    _driver.microsteps(TMC_MICROSTEPS);
    _driver.pwm_autoscale(true);
    _driver.en_spreadCycle(false);         // StealthChop: quiet motion
    _driver.blank_time(24);

    const bool ok = driverOk();
    LOGF("[TMC2209] begin, UART %s\n", ok ? "OK" : "FAILED");
    return ok;
}

bool StepperDriver::driverOk() {
    // test_connection() returns 0 when the driver answers correctly.
    return _driver.test_connection() == 0;
}

void StepperDriver::enable() {
    digitalWrite(PIN_TMC_EN, LOW);
    _enabled = true;
}

void StepperDriver::disable() {
    stop();
    digitalWrite(PIN_TMC_EN, HIGH);  // coils released -> wheel turns freely
    _enabled = false;
}

void StepperDriver::setSpeed(bool cw, float stepsPerSec) {
    digitalWrite(PIN_TMC_DIR, cw ? TMC_DIR_CW_LEVEL : !TMC_DIR_CW_LEVEL);

    if (stepsPerSec <= 0.0f) {
        _running = false;
        return;
    }
    _stepIntervalUs = (uint32_t)(1000000.0f / stepsPerSec);
    _running = true;
}

void StepperDriver::runCW(float stepsPerSec)  { setSpeed(true, stepsPerSec); }
void StepperDriver::runCCW(float stepsPerSec) { setSpeed(false, stepsPerSec); }

void StepperDriver::stop() {
    _running = false;
}

void StepperDriver::service() {
    if (!_running || _stepIntervalUs == 0) return;

    const uint32_t now = micros();
    if ((uint32_t)(now - _lastStepUs) >= _stepIntervalUs) {
        _lastStepUs = now;
        // TMC2209 needs >=100ns; a few microseconds is comfortably safe and
        // keeps the loop non-blocking.
        digitalWrite(PIN_TMC_STEP, HIGH);
        delayMicroseconds(3);
        digitalWrite(PIN_TMC_STEP, LOW);
    }
}
