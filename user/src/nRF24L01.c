#include "nRF24L01.h"

#include "nRF24L01_config.h"
#include "spi.h"

sbit ce  = nRF_CE_PIN;
sbit cs  = nRF_CS_PIN;
sbit clk = nRF_CK_PIN;
sbit miso = nRF_MISO_PIN;
sbit mosi = nRF_MOSI_PIN;
sbit irq = nRF_IRQ_PIN;

unsigned char timer_flag;
unsigned int timer_count;

unsigned char temp;

void nRF_delay_us(unsigned int us);
unsigned char nRF_read_reg(unsigned char reg);
void nRF_write_reg(unsigned char reg, unsigned char value);

///////////////////////////////////////////////////////////////////////////////////////////////////

void nRF_timer_run() { // call in timer ISR
	if(timer_count) {
		timer_count--;
		if(timer_count == 0) {
			timer_flag = 1;
		}
	}
}

void nRF_write_bit(unsigned char *reg, unsigned char pos, unsigned char bit_val) {
	*reg = (bit_val)? (*reg)|(1<<pos) : (*reg)&~(1<<pos); 
}

void nRF_delay_us(unsigned int us) {
	timer_flag = 0;
	timer_count = us / nRF_INTERRUPT_PERIOD;
	while(!timer_flag) {}
	timer_flag = 0;
}

void nRF_write_reg(unsigned char reg, unsigned char value) {
	SPI_cs(0);
	SPI_transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
	SPI_transfer(value);
	SPI_cs(1);
}

void nRF_writeBuff_reg(unsigned char reg, unsigned char* buf, unsigned char len) {
	SPI_cs(0);
	SPI_transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
	while(len--) {
		SPI_transfer(*(buf++));
	}
	SPI_cs(1);
}

unsigned char nRF_read_reg(unsigned char reg) {
	unsigned char val;
	SPI_cs(0);
	SPI_transfer( R_REGISTER | ( REGISTER_MASK & reg ) );
	val = SPI_transfer(NOP_CMD);
	SPI_cs(1);
	return(val);
}

void nRF_setPALevel(PA_level level) {
	unsigned char setup = nRF_read_reg(RF_SETUP);
	
	nRF_write_bit(&setup, RF_PWR, level&(1<<0));
	nRF_write_bit(&setup, RF_PWR+1, level&(1<<1));
	
	nRF_write_reg(RF_SETUP, setup);
}

void nRF_setAirDR(air_datarate rate) {
	unsigned char setup = nRF_read_reg(RF_SETUP);
	
	nRF_write_bit(&setup, RF_DR, rate);
	
	nRF_write_reg(RF_SETUP, setup);
}

void nRF_setCRCLength(crc_length crc) {
	unsigned char config = nRF_read_reg(CONFIG);
	if(crc == nRF_CRC_DISABLED) {
		nRF_write_bit(&config, EN_CRC, 0);
	}
	else {
		nRF_write_bit(&config, EN_CRC, 1);
		if(crc == nRF_CRC_8) {
			nRF_write_bit(&config, CRCO, 0);
		}
		else if(crc == nRF_CRC_16){
			nRF_write_bit(&config, CRCO, 1);
		}
	}
	nRF_write_reg(CONFIG, config);
}

void nRF_setChannel(unsigned char channel) { // F= 2400 + RF_CH [MHz]
	nRF_write_reg(RF_CH, (channel < 127)? channel:127);
}

void nRF_flush_tx() {
	SPI_cs(0);
	SPI_transfer(FLUSH_TX);
	SPI_cs(1);
}
void nRF_flush_rx() {
	SPI_cs(0);
	SPI_transfer(FLUSH_RX);
	SPI_cs(1);
}

void nRF_begin() {
	miso = 1; // input mode
	irq = 1; // input mode
	
	SPI_Init();
	
	ce = 0;
	SPI_cs(1);
	clk = 0;
	
	nRF_delay_us(5000);
	
  nRF_write_reg(SETUP_RETR, (0x4 << ARD) | (0xF << ARC));
	
	nRF_setPALevel(nRF_PA_MAX); // set output of power amplifier
	
	nRF_setAirDR(nRF_1MBPS); // air data rate, this speed support all kind of module
	
	nRF_setCRCLength(nRF_CRC_16); // set checksum
	
	nRF_write_reg(DYNPD, 0); // Disable dynamic payloads
	
	nRF_write_reg(STATUS, (1<<RX_DR) | (1<<TX_DS) | (1<<MAX_RT)); // Reset current status
	
	nRF_setChannel(76); // set freq of RF
	
	temp = nRF_read_reg(RF_CH);
	
	nRF_flush_tx();
	nRF_flush_rx();
}

//void nRF_openWritingPipe(unsigned long long address) {
////	unsigned char byte_addr[5];
////	nRF_writeBuff_reg(RX_ADDR_P0));
//}