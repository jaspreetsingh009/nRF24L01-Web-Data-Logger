
#include "UCB0_SPI.h"

/**
 *  \brief     uint8_t Write_Byte_UCB0_SPI (uint8_t Byte)
 *  \details   This function sends a Byte by USCI_BO SPI module.
 *
 *	@ param    uint8_t Byte - Byte sent by the SPI module.
 *  @ return   UCB0RXBUF
 *
 *  \note      This function uses the Watchdog Timer for security .
 *  \note	   SMCLK <- 8MHz
 */

uint8_t Write_Byte_UCB0_SPI (uint8_t Byte)
{
	uint8_t value = 0;

	Delay_WDT ( SECURITY_DELAY );

	UCB0TXBUF   = 	 Byte;										// Copy Byte to UCSI_B0 TX Buffer
	while ( !( IFG2 & UCB0RXIFG ) && !( IFG1 & WDTIFG ));		// USCI_B0 rX buffer ready?

	WDTCTL 		= 	 WDTPW + WDTHOLD;     								// Stop watchdog timer
	value		=	 UCB0RXBUF;

	return value;
}


/**
 *  \brief     void Delay_WDT ( void )
 *  \details   This function causes the necessary delay for the device nRF24L01 +
 * 			   Generates delays and security to avoid crashing the MCU.
 *
 *  \note      WDT clock must be associated with ACLK = VLOCLK ( ~ 12 kHz ).
 */

void Delay_WDT ( uint16_t DELAY )
{
	IFG1 &=	~WDTIFG;														// RESET WDT flag

	if (DELAY == LONG_DELAY)
		WDTCTL = WDTPW + WDTTMSEL + WDTSSEL + WDTCNTCL + WDTIS1;			// Large Delay ~ 42.67 ms
	else
		WDTCTL = WDTPW + WDTTMSEL + WDTSSEL + WDTCNTCL + WDTIS0 + WDTIS1;	// NRF24L01_START_UP_DELAY ~  5.33 ms

	if ( DELAY != SECURITY_DELAY )
	{
		while ( !( IFG1 & WDTIFG ) );										// Wait until the delay is done

		WDTCTL = WDTPW + WDTHOLD;     										// Stop watchdog timer
	}
}
