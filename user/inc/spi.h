#ifndef SPI
#define SPI

#include "compiler_defs.h"
#include "C8051F930_defs.h"
#include "peripherals.h"

#include "nRF24L01_config.h"

#define SPI_CK_PIN    nRF_CK_PIN
#define SPI_MISO_PIN  nRF_MISO_PIN
#define SPI_MOSI_PIN  nRF_MOSI_PIN
#define SPI_CS_PIN    nRF_CS_PIN

#define CPOL   0 // logic of clock at idle state is CPOL value
#define CPHA 0 // 0->sample at rising edge, 1->sample at falling edge

#define SPI_REG_LEN 8



void SPI_Init();
void SPI_cs(unsigned char level);
unsigned char SPI_transfer(unsigned char);

#endif
