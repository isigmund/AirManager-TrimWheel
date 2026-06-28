#pragma once

#include <Arduino.h>

#include <si_message_port.hpp>

// SimLink — wraps the SiMessagePort library used to talk to Air Manager.
//
// Inbound  : MSG_TRIM_FROM_SIM carries the current elevator-trim position as a
//            float. The newest value is cached and a "new value" flag is set.
// Outbound : sendTrim() pushes a wheel-driven trim position back to the sim.
class SimLink {
public:
    void begin();
    void tick();

    bool  hasNewValue() const { return _hasNew; }
    float latest() const { return _latest; }
    void  clearNew() { _hasNew = false; }

    void sendTrim(float value);
    void debug(const String& message);

private:
    // SiMessagePort needs a plain C callback; route it through this singleton.
    static SimLink* _self;
    static void onMessage(uint16_t id, struct SiMessagePortPayload* payload);
    void handleMessage(uint16_t id, struct SiMessagePortPayload* payload);

    SiMessagePort* _port = nullptr;
    float _latest = 0.0f;
    bool  _hasNew = false;
};
