#include <Arduino.h>

#include "TrimWheel.h"
#include "DebugLog.h"

static TrimWheel trimWheel;

void setup() {
    DBG_BEGIN();             // WiFi SoftAP + telnet server (no-op if DEBUG off)
    DBG_WAIT_FOR_CLIENT();   // block until a telnet client attaches (debug only)

    LOGLN("\nAirManager TrimWheel starting...");

    trimWheel.begin();
}

void loop() {
    DBG_SERVICE();
    trimWheel.update();
}
