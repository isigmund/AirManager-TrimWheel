#include "PowerDelivery.h"

#include "Config.h"
#include "DebugLog.h"

void PowerDelivery::begin() {
    // Select 5V on the CH224K before anything downstream draws current.
    pinMode(PIN_PD_CFG1, OUTPUT);
    pinMode(PIN_PD_CFG2, OUTPUT);
    pinMode(PIN_PD_CFG3, OUTPUT);
    digitalWrite(PIN_PD_CFG1, PD_CFG1_FOR_5V);
    digitalWrite(PIN_PD_CFG2, PD_CFG2_FOR_5V);
    digitalWrite(PIN_PD_CFG3, PD_CFG3_FOR_5V);

    // PG is open-drain; rely on the internal pull-up so we read a clean high
    // until the controller pulls it low to signal "power good".
    pinMode(PIN_PD_PG, INPUT_PULLUP);

    LOGLN("[PD] Requested 5V, waiting for power good...");
}

bool PowerDelivery::isPowerGood() const {
    const int level = digitalRead(PIN_PD_PG);
    return PD_POWER_GOOD_ACTIVE_LOW ? (level == LOW) : (level == HIGH);
}
