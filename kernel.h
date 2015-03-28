/*
 * kernel.h
 *
 *  Created on: Mar 16, 2015
 *      Author: Kevin
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include "thread.h"
#include "syscall_numbers.h"

#define KERNEL_SCHEDULER_IRQ_FREQ (1000)
#define SYSTIME_CYCLES_PER_MS (1000/KERNEL_SCHEDULER_IRQ_FREQ)

void kernel_init();
void kernel_run(thread_t* thread);

#endif /* KERNEL_H_ */
