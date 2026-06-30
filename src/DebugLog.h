#pragma once

// DebugLog — optional WiFi/telnet debug console, compiled in only when
// DEBUG_MODE is set in Config.h.
//
// When enabled, the board brings up a WiFi SoftAP and a TCP server on
// DEBUG_TCP_PORT. Connect with `telnet <DEBUG_AP_IP> <port>` (the AP IP is
// printed at start-up, default 192.168.4.1) to watch the log. Output is also
// mirrored to the USB CDC Serial. Writes are best-effort and never block, so a
// slow or absent reader can't stall the control loop.
//
// Use the LOGF()/LOGLN()/LOG() macros for ALL debug output. With DEBUG_MODE == 0
// they expand to nothing and no WiFi code is linked in.

#include "Config.h"

#if DEBUG_MODE

#include <Arduino.h>
#include <WiFi.h>

class DebugLog : public Print {
public:
    static DebugLog& get();

    void begin();          // bring up the SoftAP + TCP server
    void service();        // accept/drop the telnet client; call every loop
    void waitForClient();  // block until a telnet client connects

    // Print interface — output goes to the connected telnet client only.
    size_t write(uint8_t c) override;
    size_t write(const uint8_t* buffer, size_t size) override;
    using Print::write;

private:
    DebugLog() : _server(DEBUG_TCP_PORT) {}
    DebugLog(const DebugLog&) = delete;
    DebugLog& operator=(const DebugLog&) = delete;

    WiFiServer _server;
    WiFiClient _client;
    bool       _started = false;
};

#define DBG_BEGIN()            DebugLog::get().begin()
#define DBG_SERVICE()          DebugLog::get().service()
#define DBG_WAIT_FOR_CLIENT()  DebugLog::get().waitForClient()
#define LOGF(...)              DebugLog::get().printf(__VA_ARGS__)
#define LOGLN(x)               DebugLog::get().println(x)
#define LOG(x)                 DebugLog::get().print(x)

#else  // DEBUG_MODE == 0 -> logging stripped out entirely

#define DBG_BEGIN()            ((void)0)
#define DBG_SERVICE()          ((void)0)
#define DBG_WAIT_FOR_CLIENT()  ((void)0)
#define LOGF(...)              ((void)0)
#define LOGLN(x)               ((void)0)
#define LOG(x)                 ((void)0)

#endif  // DEBUG_MODE
