
/*
 ***************************************************************
 * This file is autogenerated. Modifications to its contents   *
 * will not be persistent. Modify the template source instead. *
 ***************************************************************
 */ 
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
extern inline int32_t sys_wait(fd_t fd);
extern inline bool sys_kill(tid_t tid);
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
extern inline fd_t sys_open(const char* fname, fmode_t mode, fflags_t flags);
extern inline bool sys_listdir(char* fnamebuf, size_t fnamebuflen);
extern inline void sys_rwdir();
extern inline bool sys_chdir(const char* dirname);
extern inline bool sys_mkdir(const char* dirname);
extern inline bool sys_unlink(const char* fname);
extern inline tid_t sys_spawn(void (*entry)(void*), void* arg);
extern inline tid_t sys_spawn2(void (*entry)(void*), void* arg, fd_t stdin, fd_t stdout, fd_t stderr);
 __attribute__((noreturn()))
 extern inline void sys_exit(int status);
 __attribute__((noreturn()))
 extern inline void sys_reset();


#endif /* OSPROGRAM_H_ */
