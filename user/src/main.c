#include "compiler_defs.h"
#include "C8051F930_defs.h"
#include "peripherals.h"
#include "spi.h"
#include "nRF24L01.h"

#include <stdio.h>

uint8_t write_addr[] = {0x01,0x01,0x01,0x01,0x01};
uint8_t read_addr[]  = {0x02,0x02,0x02,0x02,0x02};
uint8_t payload = 32;
uint8_t pipe = 0;
uint8_t read_addr2[]  = {0x04,0x04,0x04,0x04,0x04};

sbit nrf_led = P0^0;
uint8_t read_buf[32];
uint8_t write_buf[32];
int count=0;


const unsigned char low_reload = LOW_RELOAD_0;
const unsigned char high_reload = HIGH_RELOAD_0;
#pragma OT(1)
void TIMER_ISR() interrupt INTERRUPT_TIMER0 {
	nRF_timer_run();
	
	TL0 = low_reload;
  TH0 = high_reload;
}

extern void nRF_delay_us(unsigned long int us);
extern uint8_t nRF_flush_rx(void);
extern void nRF_testClk();

void main() {
	PCA0MD = 0;	// watchdog disable
	Init_Device();
	P1=P0=0x00;
	
	nRF_begin();
	nRF_setWritingPipe(write_addr);
	nRF_setReadingPipe(read_addr, payload, pipe);
	//nRF_startListening();
	nRF_stopListening();
	while(1) {
		nRF_delay_us(2000000);
		nrf_led^=1;
		//nRF_read();
		
		
//		if(nRF_available(0)) {
//			nrf_led^=1;
//			nRF_flush_rx();
//		}
		//nRF_read_pipe(read_buf, payload, 0);
		//nRF_stopListening();
		sprintf(write_buf, "%d", count++);
		nRF_write(write_buf, 6);
		//nRF_write("hello\0", 6);
		//nRF_testClk();
	}
}

