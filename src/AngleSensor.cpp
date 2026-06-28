#include "AngleSensor.h"

#include <Wire.h>

#include "Config.h"

bool AngleSensor::begin() {
    Wire.begin(PIN_AS5600_SDA, PIN_AS5600_SCL, AS5600_I2C_HZ);

    _as5600.begin();
    _as5600.setDirection(AS5600_CLOCK_WISE);
    _connected = _as5600.isConnected();
    if (!_connected) {
        Serial.println("[AS5600] sensor not detected on I2C bus!");
        return false;
    }

    _as5600.resetCumulativePosition(0);
    _position = 0;
    Serial.println("[AS5600] ready");
    return true;
}

void AngleSensor::update() {
    // getCumulativePosition() reads the raw angle and folds revolution
    // wrap-arounds into the running total in one call.
    _position = _as5600.getCumulativePosition();
}

void AngleSensor::zero() {
    _as5600.resetCumulativePosition(0);
    _position = 0;
}
