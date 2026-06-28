#pragma once

#include <Arduino.h>

// Inputs — the two limit switches (S1/S2) and the momentary centre button (S3).
// Limit switches are read directly (level matters during seeking); S3 is
// debounced and distinguishes a short press (centre) from a long press
// (re-run calibration). Each gesture is exposed as a one-shot edge.
class Inputs {
public:
    void begin();
    void update();

    bool s1Triggered() const;  // CW limit  (max Nose Down)
    bool s2Triggered() const;  // CCW limit (max Nose Up)

    // True once per short press of S3 (released before the long-press time).
    bool centerPressed();
    // True once when S3 has been held long enough to request recalibration
    // (fires while still held, so a release afterwards does NOT also centre).
    bool recalibrateRequested();

private:
    bool readActive(uint8_t pin) const;

    bool     _s3Stable = false;
    bool     _s3LastRaw = false;
    uint32_t _s3LastChangeMs = 0;
    uint32_t _s3PressStartMs = 0;
    bool     _longFired = false;   // long-press already emitted for this hold
    bool     _shortEdge = false;
    bool     _longEdge = false;
};
