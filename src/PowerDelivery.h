#pragma once

#include <Arduino.h>

// PowerDelivery — configures the CH224K USB-PD sink to request 5V and reports
// when the negotiated voltage is good. The stepper driver must stay disabled
// until isPowerGood() returns true.
class PowerDelivery {
public:
    // Drive the CFG pins to request 5V and set PG up as an input.
    void begin();

    // True once the PD controller signals that the requested voltage is stable.
    bool isPowerGood() const;
};
