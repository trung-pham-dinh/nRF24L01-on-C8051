#include "spi.h"


sbit clk = SPI_CK_PIN;
sbit miso = SPI_MISO_PIN;
sbit mosi = SPI_MOSI_PIN;
sbit cs = SPI_CS_PIN;

unsigned char shift_reg;
unsigned char i;

void SPI_init() {
	miso = 1; // input mode
	
	clk = CPOL;
	cs = 1;
}

void SPI_cs(unsigned char level) {
	cs = level;
}

unsigned char SPI_transfer(unsigned char byte) {
	shift_reg = byte;
	
	for(i=0; i<SPI_REG_LEN; i++) {
		clk =  (CPOL ^ CPHA);
		mosi = ((shift_reg & 0x80) != 0);
		shift_reg <<= 1;
		//delay(x);
		
		clk = ~(CPOL ^ CPHA);
		shift_reg = (miso)? (shift_reg | 0x01) : (shift_reg & ~0x01);
		//delay(x);
	}
	clk = CPOL;	
	return shift_reg;
}



