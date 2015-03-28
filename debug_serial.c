/*
 * debug_serial.c
 *
 *  Created on: Jan 3, 2015
 *      Author: Kevin
 */

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "inc/hw_uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include <stdint.h>

#include "debug_serial.h"

//#define RECEIVE_BUFFER_SIZE (64)
//
//char RXBuffer[RECEIVE_BUFFER_SIZE];
//int16_t RXBuffer_head, RXBuffer_tail;
//
//void Serial_ISR();

void Serial_init()
{
//	RXBuffer_head = RXBuffer_tail = 0;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
			UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE);
	GPIOPinConfigure(GPIO_PA0_U0RX | GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	UARTIntDisable(UART0_BASE, 0xFFFFFFFF);

//	UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
//	UARTIntRegister(UART0_BASE, Serial_ISR);
//	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
//	IntEnable(INT_UART0);

	UARTEnable(UART0_BASE);
}

void Serial_putc(char c)
{
	UARTCharPut(UART0_BASE, c);
}

int Serial_getc()
{
	if(!UARTCharsAvail(UART0_BASE))
		return -1;
	return (int)UARTCharGet(UART0_BASE);
}

void Serial_puts(const char * s, uint16_t maxlen)
{
	uint16_t i;
	for(i = 0; i < maxlen; i++)
	{
		if(s[i] == 0)
			break;
		UARTCharPut(UART0_BASE, s[i]);
	}
}

//uint8_t Serial_available()
//{
//	return((RXBuffer_head >= RXBuffer_tail) ?
//			(RXBuffer_head - RXBuffer_tail) : RECEIVE_BUFFER_SIZE - (RXBuffer_tail - RXBuffer_head));
//}
//
//void Serial_ISR()
//{
//
//}
