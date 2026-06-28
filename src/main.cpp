#include <Arduino.h>

#include "TrimWheel.h"

static TrimWheel trimWheel;

void setup() {
    Serial.begin(115200);
    delay(200);  // give the USB CDC a moment to enumerate
    Serial.println("\nAirManager TrimWheel starting...");

    trimWheel.begin();
}

void loop() {
    trimWheel.update();
}
