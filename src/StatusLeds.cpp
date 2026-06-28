#include "StatusLeds.h"

#include "Config.h"

void StatusLeds::begin() {
    pinMode(PIN_LED1, OUTPUT);
    writeLed1(false);

    // LED2 is brightness-controlled via the LEDC peripheral.
    ledcSetup(LED2_PWM_CHANNEL, LED2_PWM_FREQ_HZ, LED2_PWM_RESOLUTION);
    ledcAttachPin(PIN_LED2, LED2_PWM_CHANNEL);
    writeLed2Duty(0);
}

void StatusLeds::set(Mode mode) {
    if (mode == _mode) return;
    _mode = mode;
    _lastToggleMs = millis();
    _led1On = false;

    // Apply the steady parts of each mode immediately.
    switch (_mode) {
        case Mode::PowerFault:
        case Mode::Calibrating:
            writeLed2Duty(0);
            break;
        case Mode::Running:
            writeLed1(false);
            writeLed2Duty(LED2_RUN_DUTY);
            break;
    }
}

void StatusLeds::service() {
    if (_mode == Mode::Running) return;  // nothing to animate

    const uint32_t interval =
        (_mode == Mode::PowerFault) ? LED_BLINK_FAST_MS : LED_BLINK_SLOW_MS;

    const uint32_t now = millis();
    if ((now - _lastToggleMs) >= interval) {
        _lastToggleMs = now;
        _led1On = !_led1On;
        writeLed1(_led1On);
    }
}

void StatusLeds::writeLed1(bool on) {
    digitalWrite(PIN_LED1, (on == LED_ACTIVE_HIGH) ? HIGH : LOW);
}

void StatusLeds::writeLed2Duty(uint32_t duty) {
    // Invert for active-low wiring so 0 duty == LED off in both cases.
    const uint32_t maxDuty = (1u << LED2_PWM_RESOLUTION) - 1u;
    ledcWrite(LED2_PWM_CHANNEL, LED_ACTIVE_HIGH ? duty : (maxDuty - duty));
}
