#include <msp430g2553.h>
#include <stdint.h>


// Delays required for NRFL2401+
//--------------------------------------------

#define	LONG_DELAY					0x01	// Delay ~ 42.67 ms
#define	SECURITY_DELAY				0x02	// Delay ~ 5.33  ms
#define	NRF24L01_START_UP_DELAY		0x02	// Delay ~ 5.33  ms


// Function Prototypes
//--------------------------------------------

uint8_t Write_Byte_UCB0_SPI (uint8_t );
void Delay_WDT ( uint16_t  );
