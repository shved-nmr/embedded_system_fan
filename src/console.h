/*
 * console.h
 *
 *  Created on: Jan 19, 2020
 *      Author: MStefan99
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_


#include <string>
#include "ITM_write.h"
#include <cstdarg>


class console {
public:
	static void log(const char* data);
	static void log(const std::string& data);
	static void log(bool data);
	static void log(int n, const char* format, ...);
	static console getInstance();

protected:
	void print(const char* d);


private:
	console();
};


#endif /* CONSOLE_H_ */
