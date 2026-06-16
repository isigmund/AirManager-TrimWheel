#include "si_message_port_driver.h"

#include "Arduino.h"

static HardwareSerial* _serial = NULL;

void si_message_port_driver_init(void* serial_port) {
	_serial = (HardwareSerial*) serial_port;
}

void si_message_port_driver_sync(struct SiCircularData* input_buffer, struct SiCircularData* output_buffer) {
	uint8_t byte;
	if (si_circular_poll(output_buffer, 0, &byte) == SI_OK) {
		_serial->write(byte);
	}

	while ( (_serial->available() > 0) && (si_circular_data_free(input_buffer) > 0) ) {
		si_circular_push(input_buffer, _serial->read());
	}
}
