#pragma once

#include <Arduino.h>

// StatusLeds — drives the two on-board PD-Stepper LEDs.
//   LED1 (GPIO10): status indicator that blinks.
//   LED2 (GPIO12): "ready" indicator at half brightness (PWM).
//
// Modes:
//   PowerFault  : LED1 blinks rapidly, LED2 off   (PG not good)
//   Calibrating : LED1 blinks slowly,  LED2 off
//   Running     : LED1 off,            LED2 at 50% (normal operation)
class StatusLeds {
public:
    enum class Mode { PowerFault, Calibrating, Running };

    void begin();
    void set(Mode mode);
    void service();  // call every loop to drive blink timing

private:
    void writeLed1(bool on);
    void writeLed2Duty(uint32_t duty);

    Mode     _mode = Mode::PowerFault;
    bool     _led1On = false;
    uint32_t _lastToggleMs = 0;
};
