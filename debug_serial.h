/*
 * debug_serial.h
 *
 *  Created on: Jan 3, 2015
 *      Author: Kevin
 */

#ifndef DEBUG_SERIAL_H_
#define DEBUG_SERIAL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Serial_init();
void Serial_putc(char c);
int Serial_getc();
void Serial_puts(const char * s, uint16_t maxlen);
void Serial_flush();

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_SERIAL_H_ */
