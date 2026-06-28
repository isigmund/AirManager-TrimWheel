#pragma once

#include <Arduino.h>
#include <AS5600.h>

// AngleSensor — thin wrapper over the AS5600 that tracks a multi-turn,
// cumulative position. The motor sweeps several full revolutions across the
// trim range, so a single 0..4095 reading is not enough; getCumulativePosition()
// accumulates wrap-arounds into a signed count (4096 counts per revolution).
//
// update() must be called frequently enough that the wheel never moves more
// than half a turn between calls, otherwise wrap detection misjudges direction.
class AngleSensor {
public:
    bool begin();

    // Poll the sensor and refresh the cached cumulative position.
    void update();

    // Last cached cumulative position, in AS5600 counts.
    int32_t position() const { return _position; }

    // Define the current physical position as the cumulative zero.
    void zero();

    bool isConnected() const { return _connected; }

private:
    AS5600  _as5600;          // defaults to the global Wire instance
    int32_t _position = 0;
    bool    _connected = false;
};
