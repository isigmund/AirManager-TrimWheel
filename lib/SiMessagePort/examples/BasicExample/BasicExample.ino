/*
	Sim Innovations Message Port example (modified for configurable serial port):

	In this example we communicate with Air Manager or Air Player over a hardware
	serial port of your choice. On ESP32 this is typically Serial2 (pins 16/17),
	which keeps Serial (USB) free for debug output.

	See the code below on how to implement this library.

	More information on how to implement the Air Manager or Air Player side can be found here:
	https://siminnovations.com/wiki/index.php?title=Hw_message_port_add

	NOTE:
	Call serial_port.begin() BEFORE constructing SiMessagePort.
	The library no longer calls Serial.begin() internally.
*/

#include <si_message_port.hpp>

SiMessagePort* messagePort;

static void new_message_callback(uint16_t message_id, struct SiMessagePortPayload* payload) {
	// Do something with a message from Air Manager or Air Player

	// The arguments are only valid within this function!
	// Make a clone if you want to store it

	if (payload == NULL) {
		messagePort->DebugMessage(SI_MESSAGE_PORT_LOG_LEVEL_INFO, (String)"Received without payload");
	}
	else {
		switch(payload->type) {
			case SI_MESSAGE_PORT_DATA_TYPE_BYTE:
				messagePort->DebugMessage(SI_MESSAGE_PORT_LOG_LEVEL_INFO, (String)"Received " + payload->len + " bytes: " + payload->data_byte[0]);
				break;
			case SI_MESSAGE_PORT_DATA_TYPE_STRING:
				messagePort->DebugMessage(SI_MESSAGE_PORT_LOG_LEVEL_INFO, (String)"Received string: " + payload->data_string);
				break;
			case SI_MESSAGE_PORT_DATA_TYPE_INTEGER:
				messagePort->DebugMessage(SI_MESSAGE_PORT_LOG_LEVEL_INFO, (String)"Received " + payload->len + " integers" + payload->data_int[0]);
				break;
			case SI_MESSAGE_PORT_DATA_TYPE_FLOAT:
				messagePort->DebugMessage(SI_MESSAGE_PORT_LOG_LEVEL_INFO, (String)"Received " + payload->len + " floats" + payload->data_float[0]);
				break;
		}
	}
}

void setup() {
	// Optional: keep Serial free for debug output
	Serial.begin(115200);

	// Start the serial port used for Air Manager communication
	// ESP32 Serial2 defaults to RX=16, TX=17
	Serial2.begin(115200);

	// Pass the serial port instance to the library
	messagePort = new SiMessagePort(SI_MESSAGE_PORT_DEVICE_ESP32, SI_MESSAGE_PORT_CHANNEL_A, new_message_callback, &Serial2);
}

void loop() {
	// Make sure this function is called regularly
	messagePort->Tick();

	// You can send your own messages to Air Manager or Air Player
	//messagePort->SendMessage(123);
	//messagePort->SendMessage(123, "hello");
	//messagePort->SendMessage(123, (int32_t)1000);
	//messagePort->SendMessage(123, 2.5f);
	//messagePort->SendMessage(123, (uint8_t) 0xAA);
}