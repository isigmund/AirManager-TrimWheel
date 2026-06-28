#include "Inputs.h"

#include "Config.h"

void Inputs::begin() {
    const uint8_t mode = INPUT_ACTIVE_LOW ? INPUT_PULLUP : INPUT;
    pinMode(PIN_ENDSTOP_S1, mode);
    pinMode(PIN_ENDSTOP_S2, mode);
    pinMode(PIN_SWITCH_S3, mode);
}

bool Inputs::readActive(uint8_t pin) const {
    const int level = digitalRead(pin);
    return INPUT_ACTIVE_LOW ? (level == LOW) : (level == HIGH);
}

bool Inputs::s1Triggered() const { return readActive(PIN_ENDSTOP_S1); }
bool Inputs::s2Triggered() const { return readActive(PIN_ENDSTOP_S2); }

void Inputs::update() {
    const bool raw = readActive(PIN_SWITCH_S3);
    const uint32_t now = millis();

    // Debounce: accept a new stable level only after it has held steady.
    if (raw != _s3LastRaw) {
        _s3LastRaw = raw;
        _s3LastChangeMs = now;
    } else if ((now - _s3LastChangeMs) >= SWITCH_DEBOUNCE_MS && raw != _s3Stable) {
        _s3Stable = raw;
        if (_s3Stable) {
            // Press started; defer the gesture decision until release/hold.
            _s3PressStartMs = now;
            _longFired = false;
        } else if (!_longFired) {
            // Released before the long-press threshold -> short press.
            _shortEdge = true;
        }
    }

    // Fire the long press as soon as the hold time is reached (while still held)
    // so the action is immediate and a later release won't also centre.
    if (_s3Stable && !_longFired &&
        (now - _s3PressStartMs) >= SWITCH_LONG_PRESS_MS) {
        _longFired = true;
        _longEdge = true;
    }
}

bool Inputs::centerPressed() {
    const bool edge = _shortEdge;
    _shortEdge = false;
    return edge;
}

bool Inputs::recalibrateRequested() {
    const bool edge = _longEdge;
    _longEdge = false;
    return edge;
}
