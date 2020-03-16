/*
 ===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
 ===============================================================================
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>
#include "console.h"
#include "I2C.h"
#include "DigitalIoPin.h"
#include "LiquidCrystal.h"

#include <cstring>
#include <cstdio>

#include "ModbusMaster.h"
#include "ModbusRegister.h"
#include "LpcUart.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
static volatile int counter;
static volatile uint32_t systicks;

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
void SysTick_Handler(void) {
	systicks++;
	if (counter > 0)
		counter--;
}
#ifdef __cplusplus
}
#endif

void Sleep(int ms) {
	counter = ms;
	while (counter > 0) {
		__WFI();
	}
}

/* this function is required by the modbus library */
uint32_t millis() {
	return systicks;
}

#if 0
void printRegister(ModbusMaster& node, uint16_t reg)
{
	uint8_t result;
	// slave: read 16-bit registers starting at reg to RX buffer
	result = node.readHoldingRegisters(reg, 1);

	// do something with data if read is successful
	if (result == node.ku8MBSuccess)
	{
		console::log(50, "R%d=%04X\n", reg, node.getResponseBuffer(0));
	}
	else {
		console::log(50, "R%d=???\n", reg);
	}
}

bool setFrequency(ModbusMaster& node, uint16_t freq)
{
	uint8_t result;
	int ctr;
	bool atSetpoint;
	const int delay = 500;

	node.writeSingleRegister(1, freq); // set motor frequency

	console::log(50, "Set freq = %d\n", freq/40); // for debugging

	// wait until we reach set point or timeout occurs
	ctr = 0;
	atSetpoint = false;
	do {
		Sleep(delay);
		// read status word
		result = node.readHoldingRegisters(3, 1);
		// check if we are at setpoint
		if (result == node.ku8MBSuccess) {
			if(node.getResponseBuffer(0) & 0x0100) atSetpoint = true;
		}
		ctr++;
	} while(ctr < 20 && !atSetpoint);

	console::log(50, "Elapsed: %d\n", ctr * delay); // for debugging

	return atSetpoint;
}


void abbModbusTest()
{
	ModbusMaster node(2); // Create modbus object that connects to slave id 2
	node.begin(9600); // set transmission rate - other parameters are set inside the object and can't be changed here

	printRegister(node, 3); // for debugging

	node.writeSingleRegister(0, 0x0406); // prepare for starting

	printRegister(node, 3); // for debugging

	Sleep(1000); // give converter some time to set up
	// note: we should have a startup state machine that check converter status and acts per current status
	//       but we take the easy way out and just wait a while and hope that everything goes well

	printRegister(node, 3); // for debugging

	node.writeSingleRegister(0, 0x047F); // set drive to start mode

	printRegister(node, 3); // for debugging

	Sleep(1000); // give converter some time to set up
	// note: we should have a startup state machine that check converter status and acts per current status
	//       but we take the easy way out and just wait a while and hope that everything goes well

	printRegister(node, 3); // for debugging

	int i = 0;
	int j = 0;
	const uint16_t fa[20] = { 1000, 2000, 3000, 3500, 4000, 5000, 7000, 8000, 10000, 15000, 20000, 9000, 8000, 7000, 6000, 5000, 4000, 3000, 2000, 1000 };

	while (1) {
		uint8_t result;

		// slave: read (2) 16-bit registers starting at register 102 to RX buffer
		j = 0;
		do {
			result = node.readHoldingRegisters(102, 2);
			j++;
		} while(j < 3 && result != node.ku8MBSuccess);
		// note: sometimes we don't succeed on first read so we try up to threee times
		// if read is successful print frequency and current (scaled values)
		if (result == node.ku8MBSuccess) {
			console::log(50, "F=%4d, I=%4d  (ctr=%d)\n", node.getResponseBuffer(0), node.getResponseBuffer(1),j);
		}
		else {
			console::log(50, "ctr=%d\n",j);
		}

		Sleep(3000);
		i++;
		if(i >= 20) {
			i=0;
		}
		// frequency is scaled:
		// 20000 = 50 Hz, 0 = 0 Hz, linear scale 400 units/Hz
		setFrequency(node, fa[i]);
	}
}
#else
bool setFrequency(ModbusMaster &node, uint16_t freq) {
	int result;
	int ctr;
	bool atSetpoint;
	const int delay = 300;

	ModbusRegister Frequency(&node, 1); // reference 1
	ModbusRegister StatusWord(&node, 3);

	Frequency = freq; // set motor frequency

	//TODO
	//console::log(50, "Set freq = %d\n", freq / 40); // for debugging

	// wait until we reach set point or timeout occurs
	ctr = 0;
	atSetpoint = false;
	do {
		Sleep(delay);
		// read status word
		result = StatusWord;
		// check if we are at setpoint
		if (result >= 0 && (result & 0x0100))
			atSetpoint = true;
		ctr++;
	} while (ctr < 20 && !atSetpoint);

	//TODO
	//console::log(50, "Elapsed: %d\n", ctr * delay); // for debugging

	return atSetpoint;
}
#endif

void modbusTest() {
	ModbusMaster node(2); // Create modbus object that connects to slave id 2

	while (1) {
		static uint32_t i;
		uint8_t j, result;
		uint16_t data[6];

		for (j = 0; j < 6; j++) {
			i++;
			// set word(j) of TX buffer to least-significant word of counter (bits 15..0)
			node.setTransmitBuffer(j, i & 0xFFFF);
		}
		// slave: write TX buffer to (6) 16-bit registers starting at register 0
		result = node.writeMultipleRegisters(0, j);

		// slave: read (6) 16-bit registers starting at register 2 to RX buffer
		result = node.readHoldingRegisters(2, 6);

		// do something with data if read is successful
		if (result == node.ku8MBSuccess) {
			for (j = 0; j < 6; j++) {
				data[j] = node.getResponseBuffer(j);
			}
			console::log(50, "%6d, %6d, %6d, %6d, %6d, %6d\n", data[0], data[1],
					data[2], data[3], data[4], data[5]);
		}
		Sleep(1000);
	}
}

/**
 * @brief	Main UART program body
 * @return	Always returns 1
 */
int main(void) {
	SystemCoreClockUpdate();
	Board_Init();
	Chip_RIT_Init(LPC_RITIMER);

	LpcPinMap none = { -1, -1 }; // unused pin has negative values in it
	LpcPinMap txpin = { 0, 18 }; // transmit pin that goes to debugger's UART->USB converter
	LpcPinMap rxpin = { 0, 13 }; // receive pin that goes to debugger's UART->USB converter
	LpcUartConfig cfg = { LPC_USART0, 115200, UART_CFG_DATALEN_8
			| UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1, false, txpin, rxpin,
			none, none };
	LpcUart dbgu(cfg);

	/* Set up SWO to PIO1_2 */
	Chip_SWM_MovablePortPinAssign(SWM_SWO_O, 1, 2); // Needed for SWO printf

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / 1000);

	//Board_LED_Set(0, false);
	//Board_LED_Set(1, true);
	console::log(50, "Started\n"); // goes to ITM console if retarget_itm.c is included
	dbgu.write("Hello, world\n");

	ModbusMaster node(2); // Create modbus object that connects to slave id 2
	node.begin(9600); // set transmission rate - other parameters are set inside the object and can't be changed here

	ModbusRegister ControlWord(&node, 0);
	ModbusRegister StatusWord(&node, 3);
	ModbusRegister OutputFrequency(&node, 102);
	ModbusRegister Current(&node, 103);

	// need to use explicit conversion since printf's variable argument doesn't automatically convert this to an integer
	console::log(50, "Status=%04X\n", (int) StatusWord); // for debugging

	ControlWord = 0x0406; // prepare for starting

	console::log(50, "Status=%04X\n", (int) StatusWord); // for debugging

	Sleep(1000); // give converter some time to set up
	// note: we should have a startup state machine that check converter status and acts per current status
	//       but we take the easy way out and just wait a while and hope that everything goes well

	console::log(50, "Status=%04X\n", (int) StatusWord); // for debugging

	ControlWord = 0x047F; // set drive to start mode

	console::log(50, "Status=%04X\n", (int) StatusWord); // for debugging

	Sleep(1000); // give converter some time to set up
	// note: we should have a startup state machine that check converter status and acts per current status
	//       but we take the easy way out and just wait a while and hope that everything goes well

	console::log(50, "Status=%04X\n", (int) StatusWord); // for debugging

	int i = 0;

	I2C i2c { { } };
	uint8_t i2cdata[3];
	uint8_t write_cmd = 0xF1;

	int pressure = 0;

	DigitalIoPin b1(0, 10, 1, 1, 1);
	DigitalIoPin b2(0, 16, 1, 1, 1);
	DigitalIoPin b3(1, 3, 1, 1, 1);

	int b1timer = 0;
	int b2timer = 0;
	int b3timer = 0;

	DigitalIoPin rs(0, 8, false, true, false);
	DigitalIoPin en(1, 6, false, true, false);
	DigitalIoPin d4(1, 8, false, true, false);
	DigitalIoPin d5(0, 5, false, true, false);
	DigitalIoPin d6(0, 6, false, true, false);
	DigitalIoPin d7(0, 7, false, true, false);
	LiquidCrystal lcd(&rs, &en, &d4, &d5, &d6, &d7);
	lcd.begin(16, 2);
	lcd.setCursor(0, 0);
	lcd.print("Automatic");
	lcd.setCursor(1, 1);
	lcd.print("Pressure:");
	//13,1 dynamic number
	char pressurearr[7];

	while (1) {
		i2c.write(0x40, &write_cmd, 1);
		Sleep(10);
		i2c.read(0x40, i2cdata, 3);
		int16_t val = i2cdata[0] << 8 | i2cdata[1];
		val = val / 240; //Convert to Pascal
		val = val * 0.95; //Atmospheric correction
		console::log(100, "Sensor1: %u\nSensorFLAG: %u\n", val, i2cdata[2]);
		// just print the value without checking if we got a -1
		//console::log(50, "F=%4d, I=%4d\n", (int) OutputFrequency,
		//		(int) Current);
		if (val < pressure - 3 || val > pressure + 3) {
			if (pressure > 0) {
				if (val < pressure) { //50 pascal
					i += 500;
				} else if (val > pressure) {
					i -= 500;
				}
			} else {
				i = 0;
			}
		}
		if (b1.read() && b1timer == 0) {
			if (pressure < 120) {
				pressure += 5;
				b1timer = 3;
			}
		}
		if (b2.read() && b2timer == 0) {
			b2timer = 3;
		}
		if (b3.read() && b3timer == 0) {
			if (pressure > 0) {
				pressure -= 5;
				b3timer = 3;
			}
		}
		lcd.setCursor(10, 1);
		snprintf(pressurearr, 7, "%3d\pa", pressure);
		lcd.print(pressurearr);
		// frequency is scaled:
		// 20000 = 50 Hz, 0 = 0 Hz, linear scale 400 units/Hz
		setFrequency(node, i);

		b1timer--;
		b2timer--;
		b3timer--;
		if (b1timer < 0) {
			b1timer = 0;
		}
		if (b2timer < 0) {
			b2timer = 0;
		}
		if (b3timer < 0) {
			b3timer = 0;
		}
	}
	return 1;
}

