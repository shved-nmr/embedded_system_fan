/*
 * DigitalIoPin.h
 *
 *  Created on: 19 Jan 2020
 *      Author: marti
 */

#ifndef DIGITALIOPIN_H_
#define DIGITALIOPIN_H_


class DigitalIoPin {
public:
	DigitalIoPin(int, int, bool, bool, bool);
	virtual ~DigitalIoPin();
	bool read();
	void write(bool);
private:
	int port;
	int pin;
	bool isInverted;
};

#endif /* DIGITALIOPIN_H_ */
