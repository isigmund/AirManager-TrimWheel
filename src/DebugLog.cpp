#include "DebugLog.h"

#if DEBUG_MODE

#include <lwip/sockets.h>  // raw send() + MSG_DONTWAIT for non-blocking writes

DebugLog& DebugLog::get() {
    static DebugLog instance;  // lazy init avoids static-init-order issues
    return instance;
}

void DebugLog::begin() {
    if (_started) return;

    // Stand-alone access point (no internet, just a console link).
    WiFi.mode(WIFI_AP);
    WiFi.softAP(DEBUG_AP_SSID, DEBUG_AP_PASS, DEBUG_AP_CHANNEL);

    _server.begin();
    _server.setNoDelay(true);
    _started = true;
}

void DebugLog::waitForClient() {
    if (!_started) return;
    // Block here (setup() context, no control loop running yet) so the client
    // catches the log from the very first line.
    while (!(_client && _client.connected())) {
        service();
        delay(50);
    }
}

void DebugLog::service() {
    if (!_started) return;

    // Keep a single active telnet connection; refuse extras.
    if (_server.hasClient()) {
        if (_client && _client.connected()) {
            _server.available().stop();
        } else {
            _client = _server.available();
            _client.setNoDelay(true);
            _client.printf("[DebugLog] TrimWheel debug console connected (%s)\n",
                           WiFi.softAPIP().toString().c_str());
        }
    }

    // Drain anything the client sends (telnet negotiation / keystrokes); we
    // are output-only, but unread bytes would otherwise pile up.
    if (_client && _client.connected()) {
        while (_client.available()) _client.read();
    }
}

size_t DebugLog::write(uint8_t c) {
    return write(&c, 1);
}

size_t DebugLog::write(const uint8_t* buffer, size_t size) {
    // Telnet client only, and strictly non-blocking. WiFiClient::write() can
    // stall the loop for up to ~10 s on a full TCP send buffer, which would
    // corrupt the AS5600 multi-turn count — so send directly with MSG_DONTWAIT
    // and just drop whatever doesn't fit right now (best-effort logging).
    if (!(_client && _client.connected())) return size;
    const int fd = _client.fd();
    if (fd < 0) return size;

    // Translate bare LF -> CRLF so telnet terminals break lines, but don't
    // double the CR on output that is already CRLF (Print::println emits \r\n).
    size_t start = 0;
    for (size_t i = 0; i < size; ++i) {
        if (buffer[i] == '\n' && (i == 0 || buffer[i - 1] != '\r')) {
            if (i > start) ::send(fd, buffer + start, i - start, MSG_DONTWAIT);
            ::send(fd, "\r\n", 2, MSG_DONTWAIT);
            start = i + 1;
        }
    }
    if (start < size) ::send(fd, buffer + start, size - start, MSG_DONTWAIT);
    return size;
}

#endif  // DEBUG_MODE
