#ifndef PERIPHERAL
#define PERIPHERAL

#define PERIPHERAL_MIL  0



#if PERIPHERAL_MIL

#define SYSCLK 								(20000000L/1) // edit this such that suitable with peripheral.c(Hz)
#define TIMER_PRESCALER_0 		4							// edit this such that suitable with peripheral.c

#define INTERRUPT_PERIOD_0 		1						// edit this(ms)

#define TICKS_PER_MIL_0 			(SYSCLK/TIMER_PRESCALER_0/1000) // ticks/mil
#define COUNTER_RELOAD_0 			(65536 - TICKS_PER_MIL_0 * INTERRUPT_PERIOD_0) // 1 <= TICKS_PER_MIL_0 * INTERRUPT_PERIOD_0 <= 65536
#define HIGH_RELOAD_0 				(COUNTER_RELOAD_0 >> 8)
#define LOW_RELOAD_0 					(COUNTER_RELOAD_0 & 0x00FF)

#else

#define SYSCLK 								(20000000L/1) // edit this such that suitable with peripheral.c(Hz)
#define TIMER_PRESCALER_0 		4							// edit this such that suitable with peripheral.c

#define INTERRUPT_PERIOD_0 		20						// edit this(us)

#define TICKS_PER_US_0 				(SYSCLK/TIMER_PRESCALER_0/1000000L) // ticks/us
#define COUNTER_RELOAD_0 			(65536 - TICKS_PER_US_0 * INTERRUPT_PERIOD_0) // 1 <= TICKS_PER_US_0 * INTERRUPT_PERIOD_0 <= 65536
#define HIGH_RELOAD_0 				(COUNTER_RELOAD_0 >> 8)
#define LOW_RELOAD_0 					(COUNTER_RELOAD_0 & 0x00FF)

#endif

// time unit = 100us = 0.1ms
//#define HIGH_RELOAD_0 				((65536-1000) >> 8)
//#define LOW_RELOAD_0 					((65536-1000) & 0x00FF)


void Init_Device(void);

#endif