#include "compiler_defs.h"
#include "C8051F930_defs.h"
#include "peripherals.h"
#include "spi.h"
#include "nRF24L01.h"

const unsigned char low_reload = LOW_RELOAD_0;
const unsigned char high_reload = HIGH_RELOAD_0;


#pragma OT(1)
void TIMER_ISR() interrupt INTERRUPT_TIMER0 {

	nRF_timer_run();
	
	TL0 = low_reload;
  TH0 = high_reload;
}

//extern void nRF_delay_us(unsigned int us);

void main() {
	PCA0MD = 0;	// watchdog disable
	Init_Device();
	P1=P0=0x00;
	
	nRF_begin();
	
	while(1) {
	}
}

