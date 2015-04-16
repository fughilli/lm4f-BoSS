/*
 * debug_serial.h
 *
 *  Created on: Jan 3, 2015
 *      Author: Kevin
 */

#ifndef DEBUG_SERIAL_H_
#define DEBUG_SERIAL_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UART0_MODULE (0)
#define UART1_MODULE (1)
#define UART2_MODULE (2)
#define UART3_MODULE (3)
#define UART4_MODULE (4)
#define UART5_MODULE (5)
#define UART6_MODULE (6)
#define UART7_MODULE (7)

#define UART_DEBUG_MODULE UART0_MODULE

void Serial_init(uint8_t module, uint32_t baud);
void Serial_putc(uint8_t module, char c);
int Serial_getc(uint8_t module);
void Serial_puts(uint8_t module, const char * s, uint16_t maxlen);
void Serial_writebuf(uint8_t module, const uint8_t* buf, uint32_t len);
void Serial_flush(uint8_t module);
bool Serial_avail(uint8_t module);

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_SERIAL_H_ */
