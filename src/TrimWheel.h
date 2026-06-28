#pragma once

#include <Arduino.h>

#include "Config.h"
#include "AngleSensor.h"
#include "Inputs.h"
#include "PowerDelivery.h"
#include "SimLink.h"
#include "StatusLeds.h"
#include "StepperDriver.h"

// TrimWheel — the top-level controller.
//
// Lifecycle:
//   WaitPower      : hold the driver disabled until USB-PD reports 5V good.
//   CalibrateS1    : run CW until the S1 (Nose Down) limit; record position.
//   CalibrateS2    : run CCW until the S2 (Nose Up) limit; record position.
//                    The travel between S1 and S2 defines the usable range.
//   CalibrateCenter: drive back CW to the half-travel point (the centre).
//   FollowSim      : normal operation — follow the sim, relay manual turns,
//                    and centre on demand (S3). The motor freewheels whenever
//                    it is not actively driving to a target.
class TrimWheel {
public:
    void begin();
    void update();

private:
    enum class State {
        WaitPower,
        CalibrateS1,
        CalibrateS2,
        CalibrateCenter,
        FollowSim,
        Fault,
    };

    void setState(State next);
    void startCalibration();
    void runFollowSim();

    // Closed-loop move toward a cumulative-position target.
    // Returns true once the target is reached (motor stopped).
    bool driveTo(int32_t targetCounts);

    // Piecewise-linear maps between sim trim value and cumulative position,
    // anchored on the three calibration points (Nose Down / centre / Nose Up).
    int32_t simToPos(float sim) const;
    float   posToSim(int32_t pos) const;

    void enterFreewheel();
    void updateLeds();
    void heartbeat();
    const char* stateName() const;

    PowerDelivery _pd;
    StepperDriver _stepper;
    AngleSensor   _sensor;
    Inputs        _inputs;
    SimLink       _sim;
    StatusLeds    _leds;

    State    _state = State::WaitPower;
    uint32_t _phaseStartMs = 0;
    uint32_t _lastHeartbeatMs = 0;

    // Calibration results (cumulative AS5600 counts).
    int32_t _posS1 = 0;      // CW extreme  / Nose Down
    int32_t _posS2 = 0;      // CCW extreme / Nose Up
    int32_t _posCenter = 0;  // half travel

    // FollowSim bookkeeping.
    bool    _driving = false;        // actively closing on a target
    bool    _centering = false;      // current drive is an S3 centre request
    float   _referenceSim = SIM_CENTER;  // sim value the wheel currently reflects
    int32_t _freewheelBaseline = 0;  // position when freewheel began
};
