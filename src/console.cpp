/*
 * console.c
 *
 *  Created on: Jan 19, 2020
 *      Author: MStefan99
 */

#include "console.h"


console::console() {
	ITM_init();
}


console console::getInstance() {
	static console instance;
	return instance;
}


void console::print(const char* d) {\
	ITM_write(d);
}


void console::log(const char* c) {
	console::getInstance().print(c);
}


void console::log(const std::string& d) {
	console::getInstance().print(d.c_str());
}


void console::log(bool b) {
	if (b) {
		ITM_write("true\n");
	} else {
		ITM_write("false\n");
	}
}


void console::log(int n, const char* f, ...) {
	char data[n] {};

	va_list va;
	va_start(va, f);
	vsnprintf(data, n, f, va);
	log(data);
	va_end(va);
}
