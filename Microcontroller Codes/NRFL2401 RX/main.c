
#include <msp430g2553.h>
#include <stdint.h>
#include <stdlib.h>
#include "NRF24L01.h"

#include "stdio.h"

/**
 * Receiver Source Code
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
 */


// Variables
//--------------------------

volatile uint16_t   TX_Buff = 0;
volatile uint8_t	*pBuff;
volatile uint8_t	*sBuff;

volatile int16_t    ADC_DATA[5]   =  {};
volatile int16_t    TX_Buffer[30] =  {};

volatile int16_t    Upper = 0;
volatile int16_t    Lower = 0;

volatile int16_t    PIPE_NUM = -1;

unsigned char DHT[6];                    // Data from DHT11

char data_A[4] = {};
char data_B[4] = {};
char data_C[4] = {};

char DATA_A_CHAR = ' ';
char DATA_B_CHAR = ' ';
char DATA_C_CHAR = ' ';
char DATA_D_CHAR = ' ';
char DATA_E_CHAR = ' ';

// Functions
//--------------------------

void RX_FORMAT_DATA();

// Interrupt Sub-Routines
//--------------------------

void USCI0TX_ISR(void) 	__attribute__((interrupt(USCIAB0TX_VECTOR)));
void P2_ISR(void) 		__attribute__((interrupt(PORT2_VECTOR)));


void main( void )
{
	// Watchdog Timer Configuration
	//--------------------------------

	WDTCTL = WDTPW + WDTHOLD;     		// Stop watchdog timer


	// Clock Configuration
	//--------------------------------

	DCOCTL    =   CALDCO_16MHZ;			// MCLK = DCO (~16MHz)
	BCSCTL1   =   CALBC1_16MHZ;
	BCSCTL3   =   LFXT1S_2;         	// ACLK = VLOCLK (~12kHz)


	// Configure GPIO
	//--------------------------------

	P1SEL    |=   BIT2 + BIT5 + BIT6 + BIT7;
	P1SEL2   |=   BIT2 + BIT5 + BIT6 + BIT7;

	P1DIR	 |=   BIT0;
	P2DIR	 |=	  BIT0 + BIT1;

	P1OUT    &=	 ~BIT0;				   // LED1 OFF
	P2OUT	 |=	  BIT1 + BIT2;		   // nRF24L01+ no seleccionado y #IRQ = 1 ( activa en bajo )

	P2DIR    &=  ~BIT2;
	P2OUT	 &=	 ~BIT0;				   // nRF24L01+ desactivado

	P2IES    |=   BIT2;        		   // P2.2 Hi/lo edge
	P2IFG    &=  ~BIT2;       		   // P2.2 IFG cleared
	P2IE     |=   BIT2;         	   // P2.2 interrupt enabled


	// Configure SPI (USCI_B0)
	//---------------------------------

	UCB0CTL1 =   UCSWRST;
	UCB0CTL1 |=  UCSSEL_2 + UCSWRST;
	UCB0CTL0 |=  UCCKPH + UCMSB + UCMST + UCMODE_0 + UCSYNC;
	UCB0BR0  |=  0x02;                          					// SMCLK/2 = 16/2 = 8 MHz
	UCB0BR1 	=  0;

	UCB0CTL1 &= ~UCSWRST;


	// Configure UART
	//---------------------------------

	UCA0CTL1 |= UCSSEL_2; 			// use SMCLK for USCI clock
	UCA0BR0   = 130; 				// 16MHz 9600
	UCA0BR1   = 6; 				    // 16MHz 9600
	UCA0MCTL  = UCBRS1 + UCBRS0; 	// Modulation UCBRSx = 3
	UCA0CTL1 &= ~UCSWRST; 			// **Initialize USCI state machine**


	// Initialize NRFL2401+
	//----------------------------------

	nRF24L01_init ();				   	  // Initialize nRF24L01+ Module


	__enable_interrupt();            	  // Interrupts ON.


	while(1)
	{
		LPM4;

		P1OUT	|=	BIT0;			   // LED1 ON

		sBuff    =  nRF24L01_WriteRead (R_REGISTER, STATUS, 0, 1);					// Get Data From Status Register
		PIPE_NUM = (sBuff[0] & 0x0E)>>1;											// Get the pipe receiving data

		switch (PIPE_NUM) {
			case  0 :  DATA_A_CHAR = 'A'; DATA_B_CHAR = 'B'; DATA_C_CHAR = 'C'; DATA_D_CHAR = 'D'; DATA_E_CHAR = 'E';
						break;
			case  1 :  DATA_A_CHAR = 'F'; DATA_B_CHAR = 'G'; DATA_C_CHAR = 'H'; DATA_D_CHAR = 'I'; DATA_E_CHAR = 'J';
						break;
			default :
			case  2 :  DATA_A_CHAR = 'K'; DATA_B_CHAR = 'L'; DATA_C_CHAR = 'M'; DATA_D_CHAR = 'N'; DATA_E_CHAR = 'O';
					   break;
		}

		pBuff 	 = 	nRF24L01_WriteRead (R_REGISTER, R_RX_PAYLOAD, 0, DATA_BYTES);	// Reading nRF24L01 + ( 2-Bytes )

		nRF24L01_reset ();															// Reset NRF25L01


		//--------------------------------------//
		// 	    Converting Acquired Data	    //
		//--------------------------------------//

		RX_FORMAT_DATA();

		//--------------------------------------//
		// 	    Transmit Data via UART		    //
		//--------------------------------------//

		TX_Buff  =	0;
		IE2     |=  UCA0TXIE;     	   // Tx interrupt ON (starts sending packets over the UART)
	}
}

/*
 * UART : ISR
 */

void USCI0TX_ISR(void)
{
	if (TX_Buff < 26)							// UART Transmit 2-Bytes
	{
		UCA0TXBUF  = TX_Buffer[TX_Buff];    	// Copy BYTE to UART TX BUF

		TX_Buff++;
		pBuff++;
	}

	else if(TX_Buff == 26)
	{
		UCA0TXBUF  = '\n';    					// Copy BYTE to UART TX BUF
		TX_Buff++;
	}

	else
	{
		// UART Data Transmission Complete
		IE2     &= ~UCA0TXIE;    // Disable UART TX Interrupt
		TX_Buff  =	0;			 // RESET TX_Buff;
		P2OUT	|=  BIT0;		 // Activate nRF24L01+ ( CE HIGH )
		P2IE    |=  BIT2;		 // P2.2 Interrput Enable
		P1OUT	&= ~BIT0;	 	 // LED1 OFF
	}
}

/*
 * PORT 2: ISR
 */

void P2_ISR(void)
{
	P2IE  &= ~BIT2;		  // P2.2 interrupt OFF
	P2IFG &= ~BIT2;       // P2.2 IFG cleared

	LPM4_EXIT;            // Clear CPUOFF bit from 0(SR)
}


void RX_FORMAT_DATA()
{
	// DHT[1] <- RH | DHT[3] <- TEMP.
	//---------------------------------------

	const div_t da = div(pBuff[6], 10);
	const div_t db = div(pBuff[7], 10);

	/* DHT11 Data RH */
	TX_Buffer[18]   =  '0' + da.quot;
	TX_Buffer[19]   =  '0' + da.rem;
	TX_Buffer[20]   =   DATA_D_CHAR;
	TX_Buffer[21]   =   ',';

	/* DHT11 Data TEMP. */
	TX_Buffer[22]   =  '0' + db.quot;
	TX_Buffer[23]   =  '0' + db.rem;
	TX_Buffer[24]   =   DATA_E_CHAR;
	TX_Buffer[25]   =   ',';


	// ADC Data
	//---------------------------------------

	Upper = pBuff[4]; Lower = pBuff[5]<<8; ADC_DATA[0] = Upper + Lower;
	Upper = pBuff[2]; Lower = pBuff[3]<<8; ADC_DATA[1] = Upper + Lower;
	Upper = pBuff[0]; Lower = pBuff[1]<<8; ADC_DATA[2] = Upper + Lower;

	sprintf(data_A, "%d", (ADC_DATA[0] + 1000));
	sprintf(data_B, "%d", (ADC_DATA[1] + 1000));
	sprintf(data_C, "%d", (ADC_DATA[2] + 1000));

	/* ADC Data P1.0 */
	TX_Buffer[0]    =  (uint8_t) data_A[0];
	TX_Buffer[1]    =  (uint8_t) data_A[1];
	TX_Buffer[2]    =  (uint8_t) data_A[2];
	TX_Buffer[3]    =  (uint8_t) data_A[3];
	TX_Buffer[4]    =  DATA_A_CHAR;
	TX_Buffer[5]    =  ',';

	/* ADC Data P1.1 */
	TX_Buffer[6]    =  (uint8_t) data_B[0];
	TX_Buffer[7]    =  (uint8_t) data_B[1];
	TX_Buffer[8]    =  (uint8_t) data_B[2];
	TX_Buffer[9]    =  (uint8_t) data_B[3];
	TX_Buffer[10]   =  DATA_B_CHAR;
	TX_Buffer[11]   =  ',';

	/* ADC Data P1.2 */
	TX_Buffer[12]   =  (uint8_t) data_C[0];
	TX_Buffer[13]   =  (uint8_t) data_C[1];
	TX_Buffer[14]   =  (uint8_t) data_C[2];
	TX_Buffer[15]   =  (uint8_t) data_C[3];
	TX_Buffer[16]    =  DATA_C_CHAR;
	TX_Buffer[17]   =  ',';

}
