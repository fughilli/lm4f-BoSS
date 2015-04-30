/*
 * osprogram.h
 *
 *  Created on: Apr 16, 2015
 *      Author: Kevin
 */

#ifndef OSPROGRAM_H_
#define OSPROGRAM_H_

#include "../fast_utils.h"
#include "../thread.h"
#include "../file.h"
#include <stdint.h>

extern inline tid_t sys_get_tid();
extern inline tpri_t sys_get_pri();
extern inline void sys_set_pri(tpri_t pri);
extern inline void sys_yield();
extern inline void sys_putc(char c);
extern inline uint32_t sys_puts(const char* str, uint32_t len);
extern inline bool sys_lock(lock_t* l);
extern inline void sys_unlock(lock_t* l);
extern inline uint32_t sys_sleep(uint32_t ms);
extern inline uint32_t sys_fork();
extern inline void sys_set_port_dirs(uint32_t portbase, uint32_t dirs);
extern inline void sys_write_port(uint32_t portbase, uint32_t portmask, uint32_t levels);
extern inline uint32_t sys_read_port(uint32_t portbase);
extern inline void sys_close(fd_t fd);
extern inline int32_t sys_read(fd_t fd, uint8_t* buf, int32_t len);
extern inline int32_t sys_write(fd_t fd, const uint8_t* buf, int32_t len);
extern inline int32_t sys_seek(fd_t fd, int32_t pos);
extern inline uint32_t sys_ioctl(fd_t fd, uint32_t mask, void* arg);
extern inline tid_t sys_spawn(void (*entry)(void*), void* arg);

__attribute__((noreturn()))
extern inline void sys_exit(int status);

__attribute__((noreturn()))
extern inline void sys_reset();

#endif /* OSPROGRAM_H_ */
