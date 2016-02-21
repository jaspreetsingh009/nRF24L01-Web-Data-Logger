
#include "NRF24L01.h"


/**
 *  \brief     uint16_t *nRF24L01_WriteRead (uint16_t , uint16_t , uint8_t *, uint16_t )
 *  \details   This function performs both READ & WRITE operations for nRF24L01 + device.
 *
 *	@param 	   uint16_t WriteRead - Selects b/w Read & Write Opeations
 * 									Valid Arguments : R_REGISTER (reading) or W_REGISTER (writing).
 *	@param 	   uint16_t reg 	  - Action Registers
 *  @param     uint8_t *val 	  -	Write Mode: Contains Data for Transmission
 *  @param     uint16_t len 	  -	Length of Data send or write to the selected register .
 *  @return	   ret
 *
 */


uint8_t *nRF24L01_WriteRead (uint16_t WriteRead, uint16_t reg, uint8_t *val, uint16_t len)
{
	static uint8_t ret [ 32 ] = {0};
	uint16_t i	= 0;


	if ( WriteRead == W_REGISTER )
	{
		reg = W_REGISTER + reg;
	}

	P2OUT		&=	~BIT1;
	Write_Byte_UCB0_SPI ( reg );

	for ( i = 0; i < len ; i++ )
	{
		if ( ( WriteRead == R_REGISTER ) && ( reg != W_TX_PAYLOAD ) )
			ret [ i ] = Write_Byte_UCB0_SPI ( NOP );
		else
			Write_Byte_UCB0_SPI ( val [ i ] );
	}

	P2OUT		|=	 BIT1;


	return ret;
}


/**
 *  \brief     void nRF24L01_init ( void )
 *  \details   This function initializes the nRF24L01 + device.
 *
 *  \note      This function uses as the Watchdog Timer, both for time delays as security .
 *  \note	   The configuration is as follows :
 *
 *  		  		- AUTO ACK Activate.
 *  				- Pipe ERX_P0 Activate.
 *  				- 5-Bytes ADDRESS ( RF_Address ).
 *  				- Channel 1 Activate.
 *
 *  				- SPEED 			   :  1 Mbps.
 *  				- POWER 			   :  0 dBm.
 *  				- RECEIVER ADDRESS     :  0x12.
 *  				- TRANSMITTER ADDRESS  :  Selectable.
 *  				- PAYLOAD 			   :  2-Byte.
 *
 *  				- AUTO Re-transmission: Every 750us (Should be less than SECURITY_DELAY)
 *
 *  				- nRF24L01+ MODE TX, POWER UP ( nRF24L01+ STATE: Standby-I ).
 *
 */

void nRF24L01_init ( void )
{
	uint8_t val [ 5 ] = {0};
	uint16_t i	= 0;

	Delay_WDT ( LONG_DELAY );		// Power on Reset ~ 42.67 ms ( Datasheet says >= 10.3 ms )


	// Enable AUTO ACK
	// NOTE:	RX addres for data pipe 0 (RX_ADDR_P0) has to be equal to the TX address (TX_ADDR) in the PTX device
	val [ 0 ]	= ENAA_P0;
	nRF24L01_WriteRead (W_REGISTER, EN_AA, val, 1);

	// Number of enabled data pipes ( 1 - 5 )
	val [ 0 ]	= ERX_P0;
	nRF24L01_WriteRead (W_REGISTER, EN_RXADDR, val, 1);

	// RF_Address width setup (how many bytes is the receiver)
	val [ 0 ]	= 0x03;										// 5 Bytes RF_Address
	nRF24L01_WriteRead (W_REGISTER, SETUP_AW, val, 1);

	// RF channel setup (frequency 2,400 - 2,527 GHz, 1 MHz/step)
	val [ 0 ]	= 0x01;										// 2,401 GHz (same on TX and RX)
	nRF24L01_WriteRead (W_REGISTER, RF_CH, val, 1);

	// RF setup (power mode and data speed)
	val [ 0 ]	= 0x07;										// 1Mbps, 0dBm
	nRF24L01_WriteRead (W_REGISTER, RF_SETUP, val, 1);

	if(NODE_ID == 0) {

		// RECEIVER RX: RF Address setup 5 Bytes (set RX_ADDR_P0 = TX_ADDR if EN_AA is ENABLED !!!)
		for ( i = 0; i < 5; i++ )
			val [ i ] = RX_ADDR_0;		// RX address: 0x12

		nRF24L01_WriteRead (W_REGISTER, RX_ADDR_P0, val, 5);	// Pipe 0 because of it was the selected channel

		// TRANSMITTER TX: RF Address setup 5 Bytes (set RX_ADDR_P0 = TX_ADDR if EN_AA is ENABLED !!!)
		nRF24L01_WriteRead (W_REGISTER, TX_ADDR, val, 5);
	}

	else if(NODE_ID == 1) {
		// RECEIVER RX: RF Address setup 5 Bytes (set RX_ADDR_P0 = TX_ADDR if EN_AA is ENABLED !!!)
		for ( i = 0; i < 5; i++ )
			val [ i ] = RX_ADDR_1;		// RX address: 0x13

		nRF24L01_WriteRead (W_REGISTER, RX_ADDR_P0, val, 5);	// Pipe 0 because of it was the selected channel

		// TRANSMITTER TX: RF Address setup 5 Bytes (set RX_ADDR_P0 = TX_ADDR if EN_AA is ENABLED !!!)
		nRF24L01_WriteRead (W_REGISTER, TX_ADDR, val, 5);
	}

	else {
		// RECEIVER RX: RF Address setup 5 Bytes (set RX_ADDR_P0 = TX_ADDR if EN_AA is ENABLED !!!)
		for ( i = 0; i < 5; i++ )
			val [ i ] = RX_ADDR_1;		// RX address: 0x12

		val[0] = RX_ADDR_2;             // RX address: 0x14 : LSB Only

		nRF24L01_WriteRead (W_REGISTER, RX_ADDR_P0, val, 5);	// Pipe 0 because of it was the selected channel

		// TRANSMITTER TX: RF Address setup 5 Bytes (set RX_ADDR_P0 = TX_ADDR if EN_AA is ENABLED !!!)
		nRF24L01_WriteRead (W_REGISTER, TX_ADDR, val, 5);
	}


	// PAYLOAD width setup ( 1 - 32 byte)
	val [ 0 ]	= DATA_BYTES;								// 2-Byte per package
	nRF24L01_WriteRead (W_REGISTER, RX_PW_P0, val, 1);


	// Setup of Automatic Retransmission
	val [ 0 ]	= 0x21;										// 750us delay between every retry (at least 500us at 250kbps and if payload > 5bytes in 1Mbps
	// and if pauload >15bytes in 2Mbps). 1 retries.
	nRF24L01_WriteRead (W_REGISTER, SETUP_RETR, val, 1);


	// CONFIG setup
	val [ 0 ]	= 0x1E;										// Transmitter, Power Up and IRQ-interrupt NOT triggered if transmission failed
	nRF24L01_WriteRead (W_REGISTER, CONFIG, val, 1);

	Delay_WDT ( NRF24L01_START_UP_DELAY );					// Start Up ~ 5.33 ms ( Datasheet says >= 1.5 ms )

	// The nEF24L01 + device is configured in Standby -I mode
}



/**
 *  \brief     uint8_t nRF24L01_send ( uint8_t *, uint16_t )
 *  \details   This function sends data to nRF24L01 + device
 *
 *  @ param    uint8_t  *Buffer 	  -	Data Buffer (for Transmission)
 *  @ param    uint16_t len 	 	  -	Data Length
 *  @ return   This function returns the following:
 *
 *  				- 0x00:	Transmission OK
 *  				- 0xFF:	Transmission FAULT
 *
 *  \note      This function uses the Watchdog Timer for timeout as security
 *  \note	   Data is sent via polling
 */


uint8_t nRF24L01_send ( uint8_t *Buffer, uint16_t len )
{
	nRF24L01_WriteRead (R_REGISTER, FLUSH_TX, Buffer, 0);
	nRF24L01_WriteRead (R_REGISTER, W_TX_PAYLOAD, Buffer, len);

	P2OUT	|=	 BIT0;											// START Transmittion

	Delay_WDT ( SECURITY_DELAY );
	while ( (P2IN & BIT2) == BIT2  && !( IFG1 & WDTIFG ) );		// Wait until the package is SENT or TimeOut
	WDTCTL = WDTPW + WDTHOLD;     								// Stop WatchDog Timer

	P2OUT	&=	~BIT0;											// nRF24L01+ mode: STANDBY

	if ( !( IFG1 & WDTIFG ) )
	{
		// Transmission OK
		return 0x00;
	}
	else
	{
		// Transmission Error
		return 0xFF;
	}
}


/**
 *  \brief     void nRF24L01_reset ( void )
 *  \details   This function resets the STATUS register flags . Required everytime  we send or receive a package.
 *
 */

void nRF24L01_reset ( void )
{
	uint8_t val [ 1 ] = {0x70};

	nRF24L01_WriteRead (W_REGISTER, STATUS, val, 1);
}

