//*****************************************************************************
//
// Startup code for use with TI's Code Composer Studio and GNU tools.
//
// Copyright (c) 2011-2013 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
//*****************************************************************************

#include <stdint.h>
#include "debug_serial.h"
#include "fast_utils.h"
#include "thread.h"

//*****************************************************************************
//
// Fault strings
//
//*****************************************************************************

extern const char __k_kp_str_hdr[];
extern const char __k_kp_str_hardfault[];
extern const char __k_kp_str_nmi[];
extern const char __k_kp_str_default[];
extern const char __k_kp_str_nl[];
extern const char __k_kp_str_regtrace[];
extern const char __k_kp_str_regnames[][4];
extern const uint8_t __k_reg_sp_offsets[];

//*****************************************************************************
//
// Forward declaration of the default fault handlers.
//
//*****************************************************************************
void ResetISR(void);
static void NmiSR(void);
static void FaultISR(void);
static void IntDefaultHandler(void);

#ifndef HWREG
#define HWREG(x) (*((volatile uint32_t *)(x)))
#endif

//*****************************************************************************
//
// The entry point for the application.
//
//*****************************************************************************
extern int main(void);

extern void svc_interrupt_handler();

//*****************************************************************************
//
// Reserve space for the system stack.
//
//*****************************************************************************
static uint32_t pui32Stack[128];

//*****************************************************************************
//
// External declarations for the interrupt handlers used by the application.
//
//*****************************************************************************
// To be added by user

//*****************************************************************************
//
// The vector table.  Note that the proper constructs must be placed on this to
// ensure that it ends up at physical address 0x0000.0000 or at the start of
// the program if located at a start address other than 0.
//
//*****************************************************************************
__attribute__ ((section(".isr_vector")))
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))((uint32_t)pui32Stack + sizeof(pui32Stack)),
                                            // The initial stack pointer
    ResetISR,                               // The reset handler
    NmiSR,                                  // The NMI handler
    FaultISR,                               // The hard fault handler
    IntDefaultHandler,                      // The MPU fault handler
    IntDefaultHandler,                      // The bus fault handler
    IntDefaultHandler,                      // The usage fault handler
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    svc_interrupt_handler,                      // SVCall handler
    IntDefaultHandler,                      // Debug monitor handler
    0,                                      // Reserved
    IntDefaultHandler,                      // The PendSV handler
    svc_interrupt_handler,                      // The SysTick handler
    IntDefaultHandler,                      // GPIO Port A
    IntDefaultHandler,                      // GPIO Port B
    IntDefaultHandler,                      // GPIO Port C
    IntDefaultHandler,                      // GPIO Port D
    IntDefaultHandler,                      // GPIO Port E
    IntDefaultHandler,                      // UART0 Rx and Tx
    IntDefaultHandler,                      // UART1 Rx and Tx
    IntDefaultHandler,                      // SSI0 Rx and Tx
    IntDefaultHandler,                      // I2C0 Master and Slave
    IntDefaultHandler,                      // PWM Fault
    IntDefaultHandler,                      // PWM Generator 0
    IntDefaultHandler,                      // PWM Generator 1
    IntDefaultHandler,                      // PWM Generator 2
    IntDefaultHandler,                      // Quadrature Encoder 0
    IntDefaultHandler,                      // ADC Sequence 0
    IntDefaultHandler,                      // ADC Sequence 1
    IntDefaultHandler,                      // ADC Sequence 2
    IntDefaultHandler,                      // ADC Sequence 3
    IntDefaultHandler,                      // Watchdog timer
    IntDefaultHandler,                      // Timer 0 subtimer A
    IntDefaultHandler,                      // Timer 0 subtimer B
    IntDefaultHandler,                      // Timer 1 subtimer A
    IntDefaultHandler,                      // Timer 1 subtimer B
    IntDefaultHandler,                      // Timer 2 subtimer A
    IntDefaultHandler,                      // Timer 2 subtimer B
    IntDefaultHandler,                      // Analog Comparator 0
    IntDefaultHandler,                      // Analog Comparator 1
    IntDefaultHandler,                      // Analog Comparator 2
    IntDefaultHandler,                      // System Control (PLL, OSC, BO)
    IntDefaultHandler,                      // FLASH Control
    IntDefaultHandler,                      // GPIO Port F
    IntDefaultHandler,                      // GPIO Port G
    IntDefaultHandler,                      // GPIO Port H
    IntDefaultHandler,                      // UART2 Rx and Tx
    IntDefaultHandler,                      // SSI1 Rx and Tx
    IntDefaultHandler,                      // Timer 3 subtimer A
    IntDefaultHandler,                      // Timer 3 subtimer B
    IntDefaultHandler,                      // I2C1 Master and Slave
    IntDefaultHandler,                      // Quadrature Encoder 1
    IntDefaultHandler,                      // CAN0
    IntDefaultHandler,                      // CAN1
    IntDefaultHandler,                      // CAN2
    0,                                      // Reserved
    IntDefaultHandler,                      // Hibernate
    IntDefaultHandler,                      // USB0
    IntDefaultHandler,                      // PWM Generator 3
    IntDefaultHandler,                      // uDMA Software Transfer
    IntDefaultHandler,                      // uDMA Error
    IntDefaultHandler,                      // ADC1 Sequence 0
    IntDefaultHandler,                      // ADC1 Sequence 1
    IntDefaultHandler,                      // ADC1 Sequence 2
    IntDefaultHandler,                      // ADC1 Sequence 3
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // GPIO Port J
    IntDefaultHandler,                      // GPIO Port K
    IntDefaultHandler,                      // GPIO Port L
    IntDefaultHandler,                      // SSI2 Rx and Tx
    IntDefaultHandler,                      // SSI3 Rx and Tx
    IntDefaultHandler,                      // UART3 Rx and Tx
    IntDefaultHandler,                      // UART4 Rx and Tx
    IntDefaultHandler,                      // UART5 Rx and Tx
    IntDefaultHandler,                      // UART6 Rx and Tx
    IntDefaultHandler,                      // UART7 Rx and Tx
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // I2C2 Master and Slave
    IntDefaultHandler,                      // I2C3 Master and Slave
    IntDefaultHandler,                      // Timer 4 subtimer A
    IntDefaultHandler,                      // Timer 4 subtimer B
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    IntDefaultHandler,                      // Timer 5 subtimer A
    IntDefaultHandler,                      // Timer 5 subtimer B
    IntDefaultHandler,                      // Wide Timer 0 subtimer A
    IntDefaultHandler,                      // Wide Timer 0 subtimer B
    IntDefaultHandler,                      // Wide Timer 1 subtimer A
    IntDefaultHandler,                      // Wide Timer 1 subtimer B
    IntDefaultHandler,                      // Wide Timer 2 subtimer A
    IntDefaultHandler,                      // Wide Timer 2 subtimer B
    IntDefaultHandler,                      // Wide Timer 3 subtimer A
    IntDefaultHandler,                      // Wide Timer 3 subtimer B
    IntDefaultHandler,                      // Wide Timer 4 subtimer A
    IntDefaultHandler,                      // Wide Timer 4 subtimer B
    IntDefaultHandler,                      // Wide Timer 5 subtimer A
    IntDefaultHandler,                      // Wide Timer 5 subtimer B
    IntDefaultHandler,                      // FPU
    IntDefaultHandler,                      // PECI 0
    IntDefaultHandler,                      // LPC 0
    IntDefaultHandler,                      // I2C4 Master and Slave
    IntDefaultHandler,                      // I2C5 Master and Slave
    IntDefaultHandler,                      // GPIO Port M
    IntDefaultHandler,                      // GPIO Port N
    IntDefaultHandler,                      // Quadrature Encoder 2
    IntDefaultHandler,                      // Fan 0
    0,                                      // Reserved
    IntDefaultHandler,                      // GPIO Port P (Summary or P0)
    IntDefaultHandler,                      // GPIO Port P1
    IntDefaultHandler,                      // GPIO Port P2
    IntDefaultHandler,                      // GPIO Port P3
    IntDefaultHandler,                      // GPIO Port P4
    IntDefaultHandler,                      // GPIO Port P5
    IntDefaultHandler,                      // GPIO Port P6
    IntDefaultHandler,                      // GPIO Port P7
    IntDefaultHandler,                      // GPIO Port Q (Summary or Q0)
    IntDefaultHandler,                      // GPIO Port Q1
    IntDefaultHandler,                      // GPIO Port Q2
    IntDefaultHandler,                      // GPIO Port Q3
    IntDefaultHandler,                      // GPIO Port Q4
    IntDefaultHandler,                      // GPIO Port Q5
    IntDefaultHandler,                      // GPIO Port Q6
    IntDefaultHandler,                      // GPIO Port Q7
    IntDefaultHandler,                      // GPIO Port R
    IntDefaultHandler,                      // GPIO Port S
    IntDefaultHandler,                      // PWM 1 Generator 0
    IntDefaultHandler,                      // PWM 1 Generator 1
    IntDefaultHandler,                      // PWM 1 Generator 2
    IntDefaultHandler,                      // PWM 1 Generator 3
    IntDefaultHandler                       // PWM 1 Fault
};

//*****************************************************************************
//
// The following are constructs created by the linker, indicating where the
// the "data" and "bss" segments reside in memory.  The initializers for the
// for the "data" segment resides immediately following the "text" segment.
//
//*****************************************************************************
extern uint32_t __etext;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;

//*****************************************************************************
//
// This is the code that gets called when the processor first starts execution
// following a reset event.  Only the absolutely necessary set is performed,
// after which the application supplied entry() routine is called.  Any fancy
// actions (such as making decisions based on the reset cause register, and
// resetting the bits in that register) are left solely in the hands of the
// application.
//
//*****************************************************************************
void
ResetISR(void)
{
    uint32_t *pui32Src, *pui32Dest;

    //
    // Copy the data segment initializers from flash to SRAM.
    //
    pui32Src = &__etext;
    for(pui32Dest = &__data_start__; pui32Dest < &__data_end__; )
    {
        *pui32Dest++ = *pui32Src++;
    }

    //
    // Zero fill the bss segment.
    //
    __asm("    ldr     r0, =__bss_start__\n"
          "    ldr     r1, =__bss_end__\n"
          "    mov     r2, #0\n"
          "    .thumb_func\n"
          "zero_loop:\n"
          "        cmp     r0, r1\n"
          "        it      lt\n"
          "        strlt   r2, [r0], #4\n"
          "        blt     zero_loop");

    //
    // Enable the floating-point unit.  This must be done here to handle the
    // case where main() uses floating-point and the function prologue saves
    // floating-point registers (which will fault if floating-point is not
    // enabled).  Any configuration of the floating-point unit using DriverLib
    // APIs must be done here prior to the floating-point unit being enabled.
    //
    // Note that this does not use DriverLib since it might not be included in
    // this project.
    //
    HWREG(0xE000ED88) = ((HWREG(0xE000ED88) & ~0x00F00000) | 0x00F00000);
    
    //
    // Call the application's entry point.
    //
    main();
}

void dump_core(uint32_t* saved_sp)
{
	char regvalbuf[32];

	Serial_puts(UART_DEBUG_MODULE, __k_kp_str_regtrace,
			fast_strlen(__k_kp_str_regtrace));

	Serial_putc(UART_DEBUG_MODULE, '\t');
	Serial_puts(UART_DEBUG_MODULE, __k_kp_str_regnames[0],
			fast_strlen(__k_kp_str_regnames[0]));
	fast_snprintf(regvalbuf, 32, ":= 0x%08x\n\r", (uint32_t) saved_sp);
	Serial_puts(UART_DEBUG_MODULE, regvalbuf, fast_strlen(regvalbuf));

	uint32_t i;
	for (i = 1; i < (THREAD_SAVED_REGISTERS_NUM); i++)
	{
		Serial_putc(UART_DEBUG_MODULE, '\t');
		Serial_puts(UART_DEBUG_MODULE, __k_kp_str_regnames[i],
				fast_strlen(__k_kp_str_regnames[i]));
		fast_snprintf(regvalbuf, 32, ":= 0x%08x\n\r",
				saved_sp[__k_reg_sp_offsets[i]]);
		Serial_puts(UART_DEBUG_MODULE, regvalbuf, fast_strlen(regvalbuf));
	}
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives a NMI.  This
// simply enters an infinite loop, preserving the system state for examination
// by a debugger.
//
//*****************************************************************************
static void
NmiSR(void)
{
	uint32_t* saved_sp;
	// Push the context of the trapping thread
	asm volatile (
			"dsb\n\t"
			"isb\n\t"
			// Push the remaining registers;
			// R0,R1,R2,R3,R12,LR,PC+4,xPSR are pushed
			// R7 is pushed on call entry
			// Remaining registers are:
			"push {R4, R5, R6, R8, R9, R10, R11}\n\t"
			// Save the stack pointer to SP
			"mov %0,SP"
			: "=r" (saved_sp) : : "memory"
	);

    //
    // Enter an infinite loop.
    //
	Serial_puts(UART_DEBUG_MODULE, __k_kp_str_hdr, fast_strlen(__k_kp_str_hdr));
	Serial_puts(UART_DEBUG_MODULE, __k_kp_str_nmi, fast_strlen(__k_kp_str_nmi));
	Serial_puts(UART_DEBUG_MODULE, __k_kp_str_nl,fast_strlen(__k_kp_str_nl));

	dump_core(saved_sp);

    while(1)
    {
    }
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives a fault
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
static void
FaultISR(void)
{
	uint32_t* saved_sp;
	// Push the context of the trapping thread
	asm volatile (
			"dsb\n\t"
			"isb\n\t"
			// Push the remaining registers;
			// R0,R1,R2,R3,R12,LR,PC+4,xPSR are pushed
			// R7 is pushed on call entry
			// Remaining registers are:
			"push {R4, R5, R6, R8, R9, R10, R11}\n\t"
			// Save the stack pointer to SP
			"mov %0,SP"
			: "=r" (saved_sp) : : "memory"
	);

	Serial_puts(UART_DEBUG_MODULE, __k_kp_str_hdr,fast_strlen(__k_kp_str_hdr));
	Serial_puts(UART_DEBUG_MODULE, __k_kp_str_hardfault,fast_strlen(__k_kp_str_hardfault));
	Serial_puts(UART_DEBUG_MODULE, __k_kp_str_nl,fast_strlen(__k_kp_str_nl));

	dump_core(saved_sp);

	// R4 = oldsp[0];
	// R5 = oldsp[1];
	// R6 = oldsp[2];
	// R8 = oldsp[3];
	// R9 = oldsp[4];
	// R10 = oldsp[5];
	// R11 = oldsp[6];

	// R7 = oldsp[9];

	// R0 = oldsp[11];
	// R1 = oldsp[12];
	// R2 = oldsp[13];
	// R3 = oldsp[14];
	// R12 = oldsp[15];
	// LR = oldsp[16];
	// PC = oldsp[17];
	// PSR = oldsp[18];

	//
	// Enter an infinite loop.
	//
    while(1)
    {
    }
}

//*****************************************************************************
//
// This is the code that gets called when the processor receives an unexpected
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
static void
IntDefaultHandler(void)
{
	uint32_t* saved_sp;
	// Push the context of the trapping thread
	asm volatile (
			"dsb\n\t"
			"isb\n\t"
			// Push the remaining registers;
			// R0,R1,R2,R3,R12,LR,PC+4,xPSR are pushed
			// R7 is pushed on call entry
			// Remaining registers are:
			"push {R4, R5, R6, R8, R9, R10, R11}\n\t"
			// Save the stack pointer to SP
			"mov %0,SP"
			: "=r" (saved_sp) : : "memory"
	);

    //
    // Go into an infinite loop.
    //
	Serial_puts(UART_DEBUG_MODULE, __k_kp_str_hdr, fast_strlen(__k_kp_str_hdr));
	Serial_puts(UART_DEBUG_MODULE, __k_kp_str_default, fast_strlen(__k_kp_str_default));
	Serial_puts(UART_DEBUG_MODULE, __k_kp_str_nl,fast_strlen(__k_kp_str_nl));

	dump_core(saved_sp);

    while(1)
    {
    }
}
