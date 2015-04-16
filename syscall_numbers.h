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
#define SYSCALL_SPAWN         	(3)
#define SYSCALL_FORK          	(4)
#define SYSCALL_RESET         	(5)
#define SYSCALL_LOCK          	(6)
#define SYSCALL_UNLOCK        	(7)
#define SYSCALL_GET_TID       	(8)
#define SYSCALL_SET_PRI       	(9)
#define SYSCALL_GET_PRI       	(10)
#define SYSCALL_PUTS          	(11)
#define SYSCALL_PUTC          	(12)
#define SYSCALL_WRITE         	(13)
#define SYSCALL_READ          	(14)
#define SYSCALL_SEEK          	(15)
#define SYSCALL_IOCTL         	(16)
#define SYSCALL_CLOSE         	(17)
#define SYSCALL_POPEN         	(18)
#define SYSCALL_FOPEN         	(19)
#define SYSCALL_DOPEN         	(20)
#define SYSCALL_MOUNT         	(21)
#define SYSCALL_UNMOUNT       	(22)
#define SYSCALL_FLUBBER       	(23)
#define SYSCALL_SET_PORT_DIRS 	(24)
#define SYSCALL_READ_PORT     	(25)
#define SYSCALL_WRITE_PORT    	(26)

#endif /* SYSCALL_NUMBERS_H_ */
