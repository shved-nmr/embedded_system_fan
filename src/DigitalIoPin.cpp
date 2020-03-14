/*
 * DigitalIoPin.cpp
 *
 *  Created on: 19 Jan 2020
 *      Author: marti
 */

#include "DigitalIoPin.h"
#include "board.h"

DigitalIoPin::DigitalIoPin(int port, int pin, bool input = true, bool pullup =
		true, bool invert = false) :
		port(port), pin(pin), isInverted(invert) {

	if (pullup) {
		if (invert) {
			Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin,
					(IOCON_MODE_PULLUP | IOCON_DIGMODE_EN | IOCON_INV_EN));
		} else {
			Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin,
					(IOCON_MODE_PULLUP | IOCON_DIGMODE_EN));
		}
	} else {
		if (invert) {
			Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin,
					(IOCON_MODE_PULLDOWN | IOCON_DIGMODE_EN | IOCON_INV_EN));
		} else {
			Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin,
					(IOCON_MODE_PULLDOWN | IOCON_DIGMODE_EN));
		}
	}
	if (input) {
		isInverted = false;
		Chip_GPIO_SetPinDIRInput(LPC_GPIO, port, pin);
	} else {
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, port, pin);
	}
}

bool DigitalIoPin::read() {
	return Chip_GPIO_GetPinState(LPC_GPIO, port, pin);
}

void DigitalIoPin::write(bool state) {
	if (isInverted) {
		state = !state;
		Chip_GPIO_SetPinState(LPC_GPIO, port, pin, state);
	} else {
		Chip_GPIO_SetPinState(LPC_GPIO, port, pin, state);
	}
}

DigitalIoPin::~DigitalIoPin() {
	// TODO Auto-generated destructor stub
}

