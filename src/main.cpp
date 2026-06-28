#include <Arduino.h>

#include "TrimWheel.h"

static TrimWheel trimWheel;

void setup() {
    Serial.begin(115200);
    delay(200);  // give the USB CDC a moment to enumerate

#if ARDUINO_USB_CDC_ON_BOOT
    // Debug runs over the native USB CDC. By default a write blocks until the
    // host drains the TX buffer; with no host attached (or a slow one) that
    // stalls the loop for up to the tx timeout. A long stall lets the wheel
    // turn >half an AS5600 turn between samples, corrupting the multi-turn
    // count and breaking closed-loop centring. A 0 ms timeout makes writes
    // drop bytes instead of blocking, so debug output can never stall control.
    Serial.setTxTimeoutMs(0);
#endif

    Serial.println("\nAirManager TrimWheel starting...");

    trimWheel.begin();
}

void loop() {
    trimWheel.update();
}
