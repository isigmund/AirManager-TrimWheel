#include "TrimWheel.h"

#include "Config.h"

namespace {
// Linear interpolation that also extrapolates; callers clamp the inputs.
float linMap(float x, float x0, float x1, float y0, float y1) {
    if (x1 == x0) return y0;
    return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}
}  // namespace

void TrimWheel::begin() {
    Serial.println("[TrimWheel] init");

    _sim.begin();          // bring the Air Manager link up early for logging
    _pd.begin();           // request 5V; driver stays disabled
    _stepper.begin();      // configure TMC2209 (disabled / freewheel)
    _sensor.begin();
    _inputs.begin();
    _leds.begin();

    setState(State::WaitPower);
}

void TrimWheel::setState(State next) {
    _state = next;
    _phaseStartMs = millis();
}

void TrimWheel::startCalibration() {
    // Fresh calibration run: clear any follow/centre activity, re-zero the
    // sensor reference and seek the endstops again.
    _driving = false;
    _centering = false;
    _sensor.zero();
    _stepper.enable();
    setState(State::CalibrateS1);
}

void TrimWheel::update() {
    // Keep all subsystems serviced every loop.
    _sim.tick();
    _sensor.update();
    _inputs.update();

    switch (_state) {
        case State::WaitPower:
            if (_pd.isPowerGood()) {
                Serial.println("[TrimWheel] power good -> calibrating");
                _sim.debug("TrimWheel: power good, calibrating");
                startCalibration();
            } else if (PD_POWER_GOOD_TIMEOUT_MS &&
                       (millis() - _phaseStartMs) > PD_POWER_GOOD_TIMEOUT_MS) {
                Serial.println("[TrimWheel] FAULT: power good timeout");
                setState(State::Fault);
            }
            break;

        case State::CalibrateS1:
            // Seek the CW (Nose Down) limit.
            if (_inputs.s1Triggered()) {
                _stepper.stop();
                _posS1 = _sensor.position();
                Serial.printf("[TrimWheel] S1 hit at %ld\n", (long)_posS1);
                setState(State::CalibrateS2);
            } else if ((millis() - _phaseStartMs) > CALIB_PHASE_TIMEOUT_MS) {
                Serial.println("[TrimWheel] FAULT: S1 not found");
                setState(State::Fault);
            } else {
                _stepper.runCW(CALIB_SPEED_SPS);
            }
            break;

        case State::CalibrateS2:
            // Seek the CCW (Nose Up) limit, measuring the full travel.
            if (_inputs.s2Triggered()) {
                _stepper.stop();
                _posS2 = _sensor.position();
                _posCenter = _posS1 + (_posS2 - _posS1) / 2;
                Serial.printf("[TrimWheel] S2 hit at %ld, centre %ld (travel %ld)\n",
                              (long)_posS2, (long)_posCenter,
                              (long)(_posS2 - _posS1));
                setState(State::CalibrateCenter);
            } else if ((millis() - _phaseStartMs) > CALIB_PHASE_TIMEOUT_MS) {
                Serial.println("[TrimWheel] FAULT: S2 not found");
                setState(State::Fault);
            } else {
                _stepper.runCCW(CALIB_SPEED_SPS);
            }
            break;

        case State::CalibrateCenter:
            // Drive back to half travel (CW from the S2 end) to centre the wheel.
            if (driveTo(_posCenter)) {
                Serial.println("[TrimWheel] centred -> following sim");
                _sim.debug("TrimWheel: calibrated, following sim");
                _referenceSim = SIM_CENTER;
                enterFreewheel();
                setState(State::FollowSim);
            } else if ((millis() - _phaseStartMs) > CALIB_PHASE_TIMEOUT_MS) {
                Serial.println("[TrimWheel] FAULT: centring timeout");
                setState(State::Fault);
            }
            break;

        case State::FollowSim:
            runFollowSim();
            break;

        case State::Fault:
            _stepper.disable();
            break;
    }

    // Reflect the current state on the status LEDs.
    updateLeds();

    // Periodic status so the board is observable over the serial bridge even
    // when it is sitting idle in a state that otherwise prints nothing.
    heartbeat();

    // Emit step pulses for whatever motion the state machine requested.
    _stepper.service();
}

const char* TrimWheel::stateName() const {
    switch (_state) {
        case State::WaitPower:       return "WaitPower";
        case State::CalibrateS1:     return "CalibrateS1";
        case State::CalibrateS2:     return "CalibrateS2";
        case State::CalibrateCenter: return "CalibrateCenter";
        case State::FollowSim:       return "FollowSim";
        case State::Fault:           return "Fault";
    }
    return "?";
}

void TrimWheel::heartbeat() {
    const uint32_t now = millis();
    if ((now - _lastHeartbeatMs) < 1000) return;
    _lastHeartbeatMs = now;

    Serial.printf("[hb] state=%-15s pg=%d pos=%ld refSim=%.4f drv=%d bko=%d\n",
                  stateName(), _pd.isPowerGood() ? 1 : 0,
                  (long)_sensor.position(), _referenceSim, _driving ? 1 : 0,
                  _backingOff ? 1 : 0);
}

void TrimWheel::updateLeds() {
    StatusLeds::Mode mode;
    if (!_pd.isPowerGood()) {
        // Power not good (incl. the initial WaitPower phase or a power loss).
        mode = StatusLeds::Mode::PowerFault;
    } else if (_state == State::FollowSim) {
        mode = StatusLeds::Mode::Running;
    } else {
        // Calibration phases, and any non-power fault, blink slowly.
        mode = StatusLeds::Mode::Calibrating;
    }
    _leds.set(mode);
    _leds.service();
}

void TrimWheel::runFollowSim() {
    // 0) Long press of S3 re-runs the full calibration sequence.
    if (_inputs.recalibrateRequested()) {
        Serial.println("[TrimWheel] S3 long press -> recalibrating");
        _sim.debug("TrimWheel: recalibrating");
        startCalibration();
        return;
    }

    // 1) S3 short press centres the wheel and behaves like a commanded move.
    if (_inputs.centerPressed()) {
        Serial.println("[TrimWheel] centre button");
        _stepper.enable();
        _driving = true;
        _centering = true;
    }

    // 2) A genuine change from the sim (not our own echo) starts a follow move.
    if (_sim.hasNewValue()) {
        const float s = _sim.latest();
        _sim.clearNew();
        if (!_centering && fabsf(s - _referenceSim) > SIM_FOLLOW_DEADBAND) {
            _stepper.enable();
            _driving = true;
        }
    }

    if (_driving) {
        // Centring locks onto the mechanical centre; otherwise track the latest
        // sim value (it may keep moving while we close on it).
        const int32_t target =
            _centering ? _posCenter : simToPos(_sim.latest());

        if (driveTo(target)) {
            _driving = false;
            if (_centering) {
                _sim.sendTrim(SIM_CENTER);  // tell the sim where we parked
                _referenceSim = SIM_CENTER;
                _centering = false;
            } else {
                _referenceSim = _sim.latest();
            }
            enterFreewheel();
        }
        return;
    }

    // 3) Manual / idle handling.
    const int32_t pos = _sensor.position();

    // 3a) Finish an in-progress back-off from an endstop, then freewheel so the
    //     wheel is clear of the switch and free to turn back by hand.
    if (_backingOff) {
        if (driveTo(_backoffTarget)) {
            _backingOff = false;
            enterFreewheel();
            Serial.println("[TrimWheel] backed off endstop -> freewheel");
        }
        return;
    }

    // 3b) A manual turn has run the wheel onto a limit switch. Pinning the wheel
    //     on the switch and freeing it by hand needs a hard shove that mis-wraps
    //     the multi-turn count, so instead actively drive it ENDSTOP_BACKOFF_COUNTS
    //     back toward centre (3a) — a clear "you hit the limit" push that leaves
    //     the wheel free to turn back. If the back-off doesn't release the switch
    //     the next loop simply steps off again.
    if (_inputs.s1Triggered() || _inputs.s2Triggered()) {
        const bool s1 = _inputs.s1Triggered();
        // S1 is the CW/Nose-Down limit -> centre lies toward S2, and vice versa.
        const int32_t toCentre =
            ((_posS2 >= _posS1) ? 1 : -1) * (s1 ? 1 : -1) * ENDSTOP_BACKOFF_COUNTS;
        _backoffTarget = pos + toCentre;
        Serial.printf("[TrimWheel] manual over-travel onto %s -> back off %ld\n",
                      s1 ? "S1" : "S2", (long)ENDSTOP_BACKOFF_COUNTS);
        _stepper.enable();
        _backingOff = true;
        return;
    }

    // 3c) Normal freewheel: relay a deliberate manual turn to the sim.
    if (labs(pos - _freewheelBaseline) > MANUAL_MOVE_COUNTS) {
        const float s = posToSim(pos);
        _sim.sendTrim(s);
        _referenceSim = s;          // suppress the echo coming back from the sim
        _freewheelBaseline = pos;   // stream further movement incrementally
    }
}

void TrimWheel::enterFreewheel() {
    _stepper.disable();  // release coils so the wheel turns by hand
    _backingOff = false;
    _freewheelBaseline = _sensor.position();
}

bool TrimWheel::driveTo(int32_t targetCounts) {
    const int32_t error = targetCounts - _sensor.position();
    if (labs(error) <= POS_DEADBAND_COUNTS) {
        _stepper.stop();
        return true;
    }

    // Proportional speed, clamped to a sane band.
    float speed = fabsf((float)error) * DRIVE_KP_SPS;
    speed = constrain(speed, DRIVE_MIN_SPS, DRIVE_MAX_SPS);

    // Choose the rotation sense that moves the cumulative position toward the
    // target. Which sense increases the count was learned during calibration:
    // CCW carried us from S1 to S2.
    const bool ccwIncreases = (_posS2 > _posS1);
    const bool needIncrease = (error > 0);
    const bool moveCCW = (needIncrease == ccwIncreases);

    if (moveCCW) {
        _stepper.runCCW(speed);
    } else {
        _stepper.runCW(speed);
    }
    return false;
}

int32_t TrimWheel::simToPos(float sim) const {
    sim = constrain(sim, SIM_NOSE_DOWN, SIM_NOSE_UP);
    float pos;
    if (sim <= SIM_CENTER) {
        pos = linMap(sim, SIM_NOSE_DOWN, SIM_CENTER,
                     (float)_posS1, (float)_posCenter);
    } else {
        pos = linMap(sim, SIM_CENTER, SIM_NOSE_UP,
                     (float)_posCenter, (float)_posS2);
    }
    return (int32_t)lroundf(pos);
}

float TrimWheel::posToSim(int32_t pos) const {
    // Position fraction along the travel, direction-agnostic.
    const int32_t span = _posS2 - _posS1;
    if (span == 0) return SIM_CENTER;

    float t = (float)(pos - _posS1) / (float)span;          // 0 at S1, 1 at S2
    t = constrain(t, 0.0f, 1.0f);
    const float tCenter = (float)(_posCenter - _posS1) / (float)span;

    float sim;
    if (t <= tCenter) {
        sim = linMap(t, 0.0f, tCenter, SIM_NOSE_DOWN, SIM_CENTER);
    } else {
        sim = linMap(t, tCenter, 1.0f, SIM_CENTER, SIM_NOSE_UP);
    }
    return constrain(sim, SIM_NOSE_DOWN, SIM_NOSE_UP);
}
