#pragma once

#include "si_base.h"
#include "si_circular_data.h"
#include "sim_extern_shared.h"

#ifdef __cplusplus
extern "C" {
#endif

// serial_port: pointer to the HardwareSerial instance to use (e.g. &Serial2).
// The caller must call serial_port->begin() before constructing SiMessagePort.
void si_message_port_driver_init(void* serial_port);

void si_message_port_driver_sync(struct SiCircularData* input_buffer, struct SiCircularData* output_buffer);

#ifdef __cplusplus
}
#endif
