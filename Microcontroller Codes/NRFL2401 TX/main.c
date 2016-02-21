
#include <msp430g2553.h>
#include <stdint.h>
#include <stdlib.h>

#include "NRF24L01.h"

#include "stdio.h"

/** Change NODE_ID in NRF24L01.h file to Select the PIPE Number (0, 1 or 2)

/**
 * Trasmitter Source Code
 * [PINOUT]
 *
 * 1. SPI Module.
 *  - P1.5 :  UCB0CLK   |  NRF24L01+ : SCK
 *  - P1.6 :  UCB0SOMI  |  NRF24L01+ : MI
 *  - P1.7 :  UCB0SIMO  |  NRF24L01+ : MO
 *
 *  - P2.0 :  GPIO 	    |  NRF24L01+ : CE
 *  - P2.1 :  GPIO      |  NRF24L01+ : CSN
 *  - P2.2 :  GPIO      |  NRF24L01+ : #IRQ
 *
 * 2. DHT Module:
 *  - P1.4 : DATA IN (Optional)
 */

// Defines
//------------------------

#define SAMPLE_ADC		0
#define TX_RESET		1
#define	TX_START		2
#define	TX_CHECK		3
#define	TX_STATUS		4
#define	DHT_CHECK		5

#define	ERROR			0xFF
#define	RESET			0x00

// Variables
//--------------------------

volatile uint16_t STATE = 0;
uint8_t W_Buffer [16]    = {0};

unsigned int ADC_DATA[4];
unsigned char DHT[6];                    // Data from DHT11

unsigned char PAST_DHT_1;
unsigned char PAST_DHT_3;


// Interrupt Sub-Routines
//--------------------------

void TA0_ISR(void) 		__attribute__((interrupt(TIMER0_A1_VECTOR)));
void ADC10_ISR(void) 	__attribute__((interrupt(ADC10_VECTOR)));

// User Functions
//--------------------------

int READ_DHT(unsigned char *p);


void main( void )
{
	uint8_t *pBuff;


	// Watchdog Timer Configuration
	//--------------------------------

	WDTCTL = WDTPW + WDTHOLD;     		// Stop watchdog timer


	// Clock Configuration
	//--------------------------------

	BCSCTL1   =  CALBC1_1MHZ;           // MCLK = DCO (~1MHz)
	DCOCTL    =  CALDCO_1MHZ;
	BCSCTL3   =  LFXT1S_2;         	    // ACLK = VLOCLK (~12kHz)


	// Configure GPIO
	//--------------------------------
	P1SEL    |=  BIT5 + BIT6 + BIT7;    // USCI_B0 GPIO Initialize
	P1SEL2   |=  BIT5 + BIT6 + BIT7;

	P2DIR    |=  BIT3;					// Transmission Status LED
	P2OUT	 |=  BIT3;

	P2DIR	 |=	 BIT0 + BIT1;
	P2OUT	 |=	 BIT1 + BIT2;		    // nRF24L01+ unselected and IRQ = 1 ( active low )

	P2DIR    &= ~BIT2;
	P2OUT	 &=	~BIT0;				    // nRF24L01+ STOP


	// Configure SPI (USCI_B0)
	//---------------------------------

	UCB0CTL1 =   UCSWRST;
	UCB0CTL1 |=  UCSSEL_2 + UCSWRST;
	UCB0CTL0 |=  UCCKPH + UCMSB + UCMST + UCMODE_0 + UCSYNC;
	UCB0BR0  |=  0x02;                          					// SMCLK/2 = 16/2 = 8 MHz
	UCB0BR1 	=  0;

	UCB0CTL1 &= ~UCSWRST;


	// Configure Timer A0
	//----------------------------------

	TA0CCR0 = 12000;		                	// Timer Period = TA0CCR0/ACLK = 12000/12kHz = 1s
	TA0CTL  = TASSEL_1 + MC_1 + TACLR + TAIE;	// ACLK, upmode, TA0 interrupt ON


	// Configure Timer A1
	//----------------------------------

	TA1CCTL0 = OUT;
	TA1CTL = TASSEL_2 | MC_2;                   // TimerA SMCLK, continuous


	// Configure ADC
	//----------------------------------

	uint16_t degC = 0;

	ADC10CTL1 = INCH_2 + CONSEQ_1;             // A2/A1/A0, single sequence
	ADC10CTL0 = ADC10SHT_2 + MSC + ADC10ON + ADC10IE;
	ADC10DTC1 = 0x03;                          // 3 conversions
	ADC10AE0 |= 0x03;                          // Disable digital I/O on P1.0 to P1.2

	for(degC = 30; degC > 0; degC-- );


	// Initialize NRFL2401+
	//----------------------------------

	nRF24L01_init ();				 // Initialize nRF24L01+ Module

	__enable_interrupt();            // Interrupts ON.


	while(1)
	{
		LPM3;

		switch ( STATE ){
		case TX_RESET:

			//-----------------------------------------------//
			// 		 RESET NRFL2401+ Status Register		 //
			//-----------------------------------------------//

			nRF24L01_reset ();

			TA0CCR0 = 12;		                			 // TAIFG on around ~ 1 ms
			TA0CTL  = TASSEL_1 + MC_1 + TACLR + TAIE;		 // ACLK, upmode
			break;


		case TX_START:

			//-----------------------------------------------//
			// 			Transmit Acquired ADC Data			 //
			//-----------------------------------------------//

			pBuff = &W_Buffer [ 0 ];
			nRF24L01_send ( pBuff, DATA_BYTES );			 // Transmit ADC Data ( 2-Bytes )

			TA0CCR0 = 12;		                			 // TAIFG on around ~ 1 ms
			TA0CTL  = TASSEL_1 + MC_1 + TACLR + TAIE;		 // ACLK, upmode
			break;


		case TX_CHECK:

			//-----------------------------------------------//
			//		  Check for Transmission Success		 //
			//-----------------------------------------------//

			*pBuff = RESET;
			pBuff = nRF24L01_WriteRead (R_REGISTER, STATUS, 0, 1);

			if ( ( *pBuff & MAX_RT ) != 0 || ( *pBuff & TX_DS ) == 0 )
			{
				// TX FAULT: No Acknowledgement

				*pBuff = ERROR;
			}

			TA0CCR0 = 12;		                			 // TAIFG on around ~ 1 ms
			TA0CTL  = TASSEL_1 + MC_1 + TACLR + TAIE;		 // ACLK, upmode
			break;


		case TX_STATUS:

			//-----------------------------------------------//
			// 		  Toggle STATUS LED if TX SUCCESS        //
			//-----------------------------------------------//

			if ( *pBuff == ERROR )
				P2OUT	&=	~BIT3;
			else
				P2OUT	|=	 BIT3;

			TA0CCR0 = 1200;		                			 // TAIFG on around ~ 0.1 s (LED ON TIME)
			TA0CTL  = TASSEL_1 + MC_1 + TACLR + TAIE;		 // ACLK, upmode
			break;


		case DHT_CHECK:

			//-----------------------------------------------//
			// 		  	   Get Data from DHT11               //
			//-----------------------------------------------//

			READ_DHT(DHT);

			TA0CCR0 = 10800;		                		 // TAIFG on around ~ 0.9 s
			TA0CTL  = TASSEL_1 + MC_1 + TACLR + TAIE;		 // ACLK, upmode

			break;

		default:
		case SAMPLE_ADC:

			//-----------------------------------------------//
			// 			 	 Sample ADC Data			     //
			//-----------------------------------------------//

			P2OUT	&=	~BIT3;								 // TURN Status LED OFF

			TA0CCR0 = 6000;		                		     // TAIFG on around ~ 0.50 s
			TA0CTL  = TASSEL_1 + MC_1 + TACLR + TAIE;		 // ACLK, upmode
			break;
		}
	}
}


/*
 * READ_DHT : Read RH & Temp. Data From DHT11 Sensor
 */

int READ_DHT(unsigned char *p)
{                                                               // Note: TimerA must be continuous mode (MC_2) at 1 MHz
	const unsigned b = BIT4;                                    // I/O bit
	const unsigned char *end = p + 6;                           // End of data buffer
	register unsigned char m = 1;                               // First byte will have only start bit
	register unsigned st, et;                                   // Start and end times

	p[0] = p[1] = p[2] = p[3] = p[4] = p[5] = 0;                // Clear data buffer

	P1OUT &= ~b;                                                // Pull low
	P1DIR |= b;                                                 // Output
	P1REN &= ~b;                                                // Drive low
	st = TA1R; while((TA1R - st) < 18000);                      // Wait 18 ms
	P1REN |= b;                                                 // Pull low
	P1OUT |= b;                                                 // Pull high
	P1DIR &= ~b;                                                // Input
	//
	st = TA1R;                                                  // Get start time for timeout
	while(P1IN & b) if((TA1R - st) > 100) return -1;            // Wait while high, return if no response
	et = TA1R;                                                  // Get start time for timeout
	do {                                                        //
		st = et;                                                // Start time of this bit is end time of previous bit
		while(!(P1IN & b)) if((TA1R - st) > 100) return -2;     // Wait while low, return if stuck low
		while(P1IN & b) if((TA1R - st) > 200) return -3;        // Wait while high, return if stuck high
		et = TA1R;                                              // Get end time
		if((et - st) > 110) *p |= m;                            // If time > 110 us, then it is a one bit
		if(!(m >>= 1)) m = 0x80, ++p;                           // Shift mask, move to next byte when mask is zero
	} while(p < end);                                           // Do until array is full

	p -= 6;                                                     // Point to start of buffer
	if(p[0] != 1) return -4;                                    // No start bit
	if(((p[1] + p[2] + p[3] + p[4]) & 0xFF) != p[5]) return -5; // Bad checksum

	return 0;                                                   // Good read
}



/*
 * TIMER A0 : ISR
 */

void TA0_ISR (void)
{
	TA0CTL   =  MC_0;   				// STOP Timer A0
	TA0CTL  &= ~( TAIFG + TAIE );   	// Reset Interrupt Flag

	switch ( STATE ){
	case TX_RESET:

		STATE = TX_START;
		break;

	case TX_START:

		STATE = TX_CHECK;
		break;

	case TX_CHECK:

		STATE = TX_STATUS;
		break;

	case TX_STATUS:

		STATE = DHT_CHECK;
		break;

	case DHT_CHECK:

		STATE = SAMPLE_ADC;
		break;

	default:
	case SAMPLE_ADC:

		ADC10CTL0 &= ~ENC;
		while (ADC10CTL1 & BUSY);               // Wait if ADC10 core is active
		ADC10SA = (unsigned int) ADC_DATA;		// Copies data in ADC10SA to [unsigned int] ADC_DATA Array
		ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start

		// Copy ADC HEX Data to W_Buffer
		//---------------------------------------

		W_Buffer[0]  =  ADC_DATA[0];
		W_Buffer[1]  =  ADC_DATA[0]>>8;
		W_Buffer[2]  =  ADC_DATA[1];
		W_Buffer[3]  =  ADC_DATA[1]>>8;
		W_Buffer[4]  =  ADC_DATA[2];
		W_Buffer[5]  =  ADC_DATA[2]>>8;

		if(DHT[1] == 0) {
			DHT[1] = PAST_DHT_1;
		}

		else {
			PAST_DHT_1 = DHT[1];
		}

		if(DHT[3] == 0) {
			DHT[3] = PAST_DHT_3;
		}

		else {
			PAST_DHT_3 = DHT[3];
		}

		W_Buffer[6]	 =  DHT[1];     		// RH
		W_Buffer[7]	 =  DHT[3];	    		// Temperature

		STATE 	 =  TX_RESET;
		break;
	}


	if ( STATE != TX_RESET )
		LPM3_EXIT;                    // Clear CPUOFF bit from SR
}


/*
 * ADC : ISR
 */

void ADC10_ISR (void)
{
	ADC10CTL0 &= ~ENC;            		// ADC10 OFF
	LPM3_EXIT;							// Clear CPUOFF bit from 0(SR)
}

/*
 * TIMER1 : ISR
 */

#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A0(void)
{
	TA1CCTL0 &= ~CCIE;
}
