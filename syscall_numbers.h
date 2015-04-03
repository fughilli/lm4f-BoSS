/*
 * syscall_numbers.h
 *
 *  Created on: Mar 18, 2015
 *      Author: Kevin
 */

#ifndef SYSCALL_NUMBERS_H_
#define SYSCALL_NUMBERS_H_

#define SYSCALL_EXIT          	(0)
#define SYSCALL_YIELD         	(1)
#define SYSCALL_SLEEP         	(2)
#define SYSCALL_FORK          	(3)
#define SYSCALL_RESET         	(4)
#define SYSCALL_LOCK          	(5)
#define SYSCALL_UNLOCK        	(6)
#define SYSCALL_GET_TID       	(7)
#define SYSCALL_SET_PRI       	(8)
#define SYSCALL_GET_PRI       	(9)
#define SYSCALL_PUTS          	(10)
#define SYSCALL_PUTC          	(11)
#define SYSCALL_WRITE         	(12)
#define SYSCALL_READ          	(13)
#define SYSCALL_SEEK          	(14)
#define SYSCALL_IOCTL         	(15)
#define SYSCALL_CLOSE         	(16)
#define SYSCALL_POPEN         	(17)
#define SYSCALL_FOPEN         	(18)
#define SYSCALL_DOPEN         	(19)
#define SYSCALL_MOUNT         	(20)
#define SYSCALL_UNMOUNT       	(21)
#define SYSCALL_FLUBBER       	(22)
#define SYSCALL_SET_PORT_DIRS 	(23)
#define SYSCALL_READ_PORT     	(24)
#define SYSCALL_WRITE_PORT    	(25)

#endif /* SYSCALL_NUMBERS_H_ */
