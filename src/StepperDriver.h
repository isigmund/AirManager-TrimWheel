#pragma once

#include <Arduino.h>
#include <TMCStepper.h>

// StepperDriver — wraps the TMC2209 (configured over UART) and provides a
// non-blocking, velocity-controlled step generator.
//
// Usage:
//   begin()                  configure the driver (stays disabled)
//   enable() / disable()     EN pin; disable() leaves the coils open -> freewheel
//   runCW/runCCW(speed)      start stepping at |speed| steps/s in a direction
//   stop()                   stop generating pulses
//   service()                call every loop; emits STEP pulses when due
class StepperDriver {
public:
    StepperDriver();

    bool begin();

    void enable();    // energise coils (EN low)
    void disable();   // release coils -> freewheel (EN high)
    void brake();     // energise coils but stop stepping -> holding brake
    bool isEnabled() const { return _enabled; }

    void runCW(float stepsPerSec);
    void runCCW(float stepsPerSec);
    void stop();

    // Generate step pulses according to the current speed/direction.
    void service();

    bool driverOk();  // UART round-trip check

private:
    void setSpeed(bool cw, float stepsPerSec);

    TMC2209Stepper _driver;
    bool     _enabled = false;
    bool     _running = false;
    uint32_t _stepIntervalUs = 0;
    uint32_t _lastStepUs = 0;
};
