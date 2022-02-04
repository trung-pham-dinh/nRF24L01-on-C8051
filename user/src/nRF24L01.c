#include "nRF24L01.h"

#include "nRF24L01_config.h"
#include "spi.h"

sbit ce  = nRF_CE_PIN;
sbit cs  = nRF_CS_PIN;
sbit clk = nRF_CK_PIN;
sbit miso = nRF_MISO_PIN;
sbit mosi = nRF_MOSI_PIN;
sbit irq = nRF_IRQ_PIN;

static data uint8_t timer_flag; // data for faster access
static data unsigned long int timer_count; // data for faster access
////////////////////////////////////////
static uint8_t datapipe_addr[] = {RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5};
static uint8_t datapipe_en[] = {ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5};
static uint8_t datapipe_payload[] = {RX_PW_P0, RX_PW_P1, RX_PW_P2, RX_PW_P3, RX_PW_P4, RX_PW_P5};
static uint8_t payload_arr[6];
static uint8_t dynamic_en_arr[6];

static uint8_t datapipe[6][32];
static uint8_t datapipe_vld[6];
///////////////////////////////////////////////////////////////////////////////////////////////////

static uint8_t temp;
static uint8_t temp_arr[32];

///////////////////////////////////////////////////////////////////////////////////////////////////
void nRF_write_bit(uint8_t *reg, uint8_t pos, uint8_t bit_val);
void nRF_delay_us(unsigned long int us);
uint8_t nRF_write_reg(uint8_t reg, uint8_t value);
uint8_t nRF_writeBuff_reg(uint8_t reg, uint8_t* buf, uint8_t len);
uint8_t nRF_read_reg(uint8_t reg);
uint8_t nRF_readBuff_reg(uint8_t reg, uint8_t* buf, uint8_t len);
void nRF_setPALevel(PA_level level);
void nRF_setAirDR(air_datarate rate);
void nRF_setCRCLength(crc_length crc);
void nRF_setChannel(uint8_t channel);
uint8_t nRF_flush_tx(void);
uint8_t nRF_flush_rx(void);
uint8_t nRF_getStatus(void);

///////////////////////////////////////////////////////////////////////////////////////////////////
void nRF_testClk() {
	clk = 0;
	nRF_delay_us(100);
	clk=1;
	nRF_delay_us(100);
}
void nRF_timer_run() { // call in timer ISR
	if(timer_count) {
		timer_count--;
		if(timer_count == 0) {
			timer_flag = 1;
		}
	}
}

void nRF_write_bit(uint8_t *reg, uint8_t pos, uint8_t bit_val) {
	*reg = (bit_val)? (*reg)|(1<<pos) : (*reg)&~(1<<pos); 
}

void nRF_delay_us(unsigned long int us) {
	timer_flag = 0;
	timer_count = us / nRF_INTERRUPT_PERIOD;
	while(!timer_flag) {}
	timer_flag = 0;
}

uint8_t nRF_write_reg(uint8_t reg, uint8_t value) {
	uint8_t status=0;
	SPI_cs(0);
	status = SPI_transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
	SPI_transfer(value);
	SPI_cs(1);
	
	return status;
}

uint8_t nRF_writeBuff_reg(uint8_t reg, uint8_t* buf, uint8_t len) {
	uint8_t status=0;
	SPI_cs(0);
	status = SPI_transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
	while(len--) {
		SPI_transfer(*(buf++));
	}
	SPI_cs(1);
	
	return status;
}

uint8_t nRF_read_reg(uint8_t reg) {
	uint8_t val;
	SPI_cs(0);
	SPI_transfer( R_REGISTER | ( REGISTER_MASK & reg ) );
	val = SPI_transfer(NOP_CMD);
	SPI_cs(1);
	return(val);
}

uint8_t nRF_readBuff_reg(uint8_t reg, uint8_t* buf, uint8_t len) {
	uint8_t status=0;
	SPI_cs(0);
	status=SPI_transfer( R_REGISTER | ( REGISTER_MASK & reg ) );
	while(len--) {
		*(buf++) = SPI_transfer(NOP_CMD);
	}
	SPI_cs(1);
	
	return status;
}

void nRF_setPALevel(PA_level level) {
	uint8_t setup = nRF_read_reg(RF_SETUP);
	
	nRF_write_bit(&setup, RF_PWR, level&(1<<0));
	nRF_write_bit(&setup, RF_PWR+1, level&(1<<1));
	
	nRF_write_reg(RF_SETUP, setup);
}

void nRF_setAirDR(air_datarate rate) {
	uint8_t setup = nRF_read_reg(RF_SETUP);
	
	nRF_write_bit(&setup, RF_DR, rate);
	
	nRF_write_reg(RF_SETUP, setup);
}

void nRF_setCRCLength(crc_length crc) {
	uint8_t config = nRF_read_reg(CONFIG);
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

void nRF_setChannel(uint8_t channel) { // F= 2400 + RF_CH [MHz]
	nRF_write_reg(RF_CH, (channel < 127)? channel:127);
}

uint8_t nRF_flush_tx(void) {
	uint8_t status = 0;
	SPI_cs(0);
	status = SPI_transfer(FLUSH_TX);
	SPI_cs(1);
	
	return status;
}
uint8_t nRF_flush_rx(void) {
	uint8_t status = 0;
	SPI_cs(0);
	status=SPI_transfer(FLUSH_RX);
	SPI_cs(1);
	
	return status;
}

uint8_t nRF_getStatus(void) { // page 47 "The content of the status register is always read to MISO after a high to low transition on CSN."
	uint8_t status;
	SPI_cs(0);
	status = SPI_transfer(NOP_CMD);
	SPI_cs(1);
	return status;
}

void nRF_powerDown(void)
{
  nRF_write_reg(CONFIG, nRF_read_reg(CONFIG) & ~(1<<PWR_UP));
}

void nRF_powerUp(void)
{
	nRF_write_reg(CONFIG, ( nRF_read_reg(CONFIG) | (1<<PWR_UP) ) & ~(1<<PRIM_RX) );
	nRF_delay_us(160);
}

void nRF_init_var() {
	uint8_t i=0;
	for(;i<6;i++) {
		payload_arr[i] = 0;
		dynamic_en_arr[i] = 0;
		datapipe_vld[i] = 0;
		temp_arr[i%10] = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////
void nRF_begin(void) {
	miso = 1; // input mode
	irq = 1; // input mode
	
	SPI_Init();
	
	ce = 0;
	SPI_cs(1);
	clk = 0;
	nRF_init_var(); // initialize array and variable
	
	nRF_delay_us(5000);
	
  nRF_write_reg(SETUP_RETR, (0x4 << ARD) | (0xF << ARC));
	
	nRF_setPALevel(nRF_PA_MAX); // set output of power amplifier
	
	nRF_setAirDR(nRF_1MBPS); // air data rate, this speed support all kind of module
	
	nRF_setCRCLength(nRF_CRC_16); // set checksum
	
	nRF_write_reg(DYNPD, 0); // Disable dynamic payloads
	
	nRF_write_reg(STATUS, (1<<RX_DR) | (1<<TX_DS) | (1<<MAX_RT)); // Reset current status
	
	nRF_setChannel(76); // set freq of RF
	
	nRF_flush_tx();
	nRF_flush_rx();
}

void nRF_setWritingPipe(uint8_t* tx_addr) {
	nRF_writeBuff_reg(TX_ADDR, tx_addr, 5);
}

void nRF_setReadingPipe(uint8_t* rx_addr, uint8_t payload, uint8_t pipe) { // payload <=32, pipe <= 6
	uint8_t vld_payload;
	if(pipe <= 1) { // page 35 7.7 multiceiver
		nRF_writeBuff_reg(datapipe_addr[pipe], rx_addr, 5);
	}
	else if(pipe <= 6){
		nRF_writeBuff_reg(datapipe_addr[pipe], rx_addr, 1);
	}
	else return;
	
	vld_payload = (payload<32)? payload:32;
	nRF_write_reg(datapipe_payload[pipe], vld_payload);
	payload_arr[pipe] = vld_payload;
	
	//temp = nRF_read_reg(datapipe_payload[pipe]);
	
	nRF_write_reg(EN_RXADDR, nRF_read_reg(EN_RXADDR) | (1<<datapipe_en[pipe]) );
	
	//temp = nRF_read_reg(EN_RXADDR);
}

void nRF_startListening(void) {
	nRF_write_reg(CONFIG, nRF_read_reg(CONFIG) | (1<<PWR_UP) | (1<<PRIM_RX));
	nRF_write_reg(STATUS, (1<<RX_DR) | (1<<TX_DS) | (1<<MAX_RT));

	nRF_flush_tx();
	nRF_flush_rx();
	
	ce = 1;
	nRF_delay_us(140);
}

void nRF_stopListening(void) {
  ce = 0;
  nRF_flush_tx();
  nRF_flush_rx();
}

uint8_t nRF_available(uint8_t* pipe_num) {
	uint8_t status = nRF_getStatus();
	uint8_t result = (status&(1<<RX_DR));
	
	if(result) {
		if(pipe_num) {
			*pipe_num = (status >> RX_P_NO) & 0x07; // pipe_number
		}
		nRF_write_reg(STATUS, (1<<RX_DR)); // clear rx fifo flag
		
		if(status & (1<<TX_DS)) {
      nRF_write_reg(STATUS,(1<<TX_DS)); // clear ack if auto_ack is activated(page 55)
    }
	}
	return result!=0;
}


void nRF_read(void) {
	uint8_t payload;
	uint8_t* current;

	uint8_t pipe=0;
	while(nRF_available(&pipe)) {
		datapipe_vld[pipe] = 1;
		payload = payload_arr[pipe];
		current = datapipe[pipe];
		
		SPI_cs(0);
		SPI_transfer(R_RX_PAYLOAD);
		while(payload--) {
			*(current++) = SPI_transfer(NOP_CMD);
		}	
		SPI_cs(1);
	}
}

uint8_t nRF_read_pipe(void* buf, uint8_t len, uint8_t pipe) {
	uint8_t data_len;
	uint8_t *current, *buff_current;
	
	if(!datapipe_vld[pipe]) return 0;
	
	
	datapipe_vld[pipe] = 0;
	data_len = (len < payload_arr[pipe])? len:payload_arr[pipe];
	
	current = (uint8_t*)buf;
	buff_current = datapipe[pipe];
	while(data_len--) {
		*(current++) = *(buff_current++);
	}	
	return 1;
}

void nRF_write(const void* buf, uint8_t len) {
	uint8_t status, data_len, blank_len;
	const uint8_t* current = (uint8_t*)buf;
	
	nRF_powerUp();

	// send payload
	data_len = (len<32)? len:32;
	blank_len = 32-data_len;
	
	SPI_cs(0);
	SPI_transfer(W_TX_PAYLOAD);
	while(data_len--) {
		SPI_transfer(*(current++));
	}
	while(blank_len--) {
		SPI_transfer(0);
	}
	SPI_cs(1);
	
	
	ce = 1;
  nRF_delay_us(20);
  ce = 0;
	
	///////////////////////////////
	
	do {
		status = nRF_getStatus();
	}while(!(status & ((1<<TX_DS) | (1<<MAX_RT))));
	
	
	nRF_write_reg(STATUS, (1<<MAX_RT) | (1<<TX_DS)); // clear flag
	
	nRF_powerDown();
	nRF_flush_tx();
}