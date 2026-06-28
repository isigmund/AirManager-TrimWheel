#include "SimLink.h"

#include "Config.h"

SimLink* SimLink::_self = nullptr;

void SimLink::begin() {
    _self = this;

    // The library no longer opens the port itself: begin Serial2 on the
    // Air Manager pins first, then construct the message port on channel A.
    Serial2.begin(AM_BAUD, SERIAL_8N1, PIN_AM_RX, PIN_AM_TX);
    _port = new SiMessagePort(SI_MESSAGE_PORT_DEVICE_ESP32,
                              SI_MESSAGE_PORT_CHANNEL_A,
                              &SimLink::onMessage,
                              &Serial2);

    Serial.println("[SimLink] Air Manager port ready on Serial2");
}

void SimLink::tick() {
    if (_port) _port->Tick();
}

void SimLink::onMessage(uint16_t id, struct SiMessagePortPayload* payload) {
    if (_self) _self->handleMessage(id, payload);
}

void SimLink::handleMessage(uint16_t id, struct SiMessagePortPayload* payload) {
    if (id != MSG_TRIM_FROM_SIM || payload == nullptr) return;

    // Accept the trim position whether the script sends it as a float or an int.
    if (payload->type == SI_MESSAGE_PORT_DATA_TYPE_FLOAT && payload->len >= 1) {
        _latest = payload->data_float[0];
        _hasNew = true;
    } else if (payload->type == SI_MESSAGE_PORT_DATA_TYPE_INTEGER && payload->len >= 1) {
        _latest = (float)payload->data_int[0];
        _hasNew = true;
    }
}

void SimLink::sendTrim(float value) {
    if (_port) _port->SendMessage(MSG_TRIM_TO_SIM, value);
}

void SimLink::debug(const String& message) {
    if (_port) _port->DebugMessage(SI_MESSAGE_PORT_LOG_LEVEL_INFO, message);
}
