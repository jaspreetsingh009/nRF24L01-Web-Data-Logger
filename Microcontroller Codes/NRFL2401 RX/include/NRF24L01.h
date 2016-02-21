
#include <msp430.h>
#include "UCB0_SPI.h"


#define	DATA_BYTES			8		/* No. of BYTES per Packet */

#define RX_ADDR_0			0x12
#define RX_ADDR_1			0x13
#define RX_ADDR_2			0x14

// REGISTERS
//-----------------------------------

#define	CONFIG				0x00 	/* Configuration Register */
#define EN_AA				0x01	/* Enable ‘Auto Acknowledgment’ Function. Disable this functionality to be compatible with nRF2401 */
#define	EN_RXADDR			0x02	/* Enabled RX Addresses */
#define	SETUP_AW			0x03	/* Setup of Address Widths (common for all data pipes) */
#define	SETUP_RETR			0x04	/* Setup of Automatic Retransmission */
#define	RF_CH				0x05	/* RF Channel */
#define	RF_SETUP			0x06	/* RF Setup Register */
#define	STATUS				0x07	/* Status Register  */
#define	OBSERVE_TX			0x08	/* Transmit observe register  */
#define	CD					0x09
#define	RX_ADDR_P0			0x0A	/*  Receive address data pipe 0  */
#define	RX_ADDR_P1			0x0B	/*  Receive address data pipe 1  */
#define	RX_ADDR_P2			0x0C	/*  Receive address data pipe 2  */
#define	RX_ADDR_P3			0x0D	/*  Receive address data pipe 3  */
#define	RX_ADDR_P4			0x0E	/*  Receive address data pipe 4  */
#define	RX_ADDR_P5			0x0F	/*  Receive address data pipe 5  */
#define	TX_ADDR				0x10	/*  Transmit address  */
#define	RX_PW_P0			0x11
#define	RX_PW_P1			0x12
#define	RX_PW_P2			0x13
#define	RX_PW_P3			0x14
#define	RX_PW_P4			0x15
#define	RX_PW_P5			0x16
#define	FIFO_STATUS			0x17	/*  FIFO Status Register  */
#define	DYNPD				0x1C	/*  Enable dynamic payload length  */
#define	FEATURE				0x1D	/*  Feature Register  */


// MNEMONIC REGISTER CONFIG
//-------------------------------
#define	MASK_RX_DR			BIT6
#define	MASK_TX_DS			BIT5
#define	MASK_MAX_RT			BIT4
#define	EN_CRC				BIT3
#define	CRCO				BIT2
#define	PWR_UP				BIT1
#define	PRIM_RX				BIT0

// MNEMONIC REGISTER EN_AA
//-------------------------------
#define	ENAA_P5				BIT5
#define	ENAA_P4				BIT4
#define	ENAA_P3				BIT3
#define	ENAA_P2				BIT2
#define	ENAA_P1				BIT1
#define	ENAA_P0				BIT0

// MNEMONIC REGISTER EN_RXADDR
//-------------------------------
#define	ERX_P5				BIT5
#define	ERX_P4				BIT4
#define	ERX_P3				BIT3
#define	ERX_P2				BIT2
#define	ERX_P1				BIT1
#define	ERX_P0				BIT0


// MNEMONIC REGISTRO STATUS
//-------------------------------
#define	RX_DR				BIT6
#define	TX_DS				BIT5
#define	MAX_RT				BIT4
#define	RX_P_NO_3			BIT3
#define	RX_P_NO_2			BIT2
#define	RX_P_NO_1			BIT1
#define	TX_FULL				BIT0


//	MNEMONIC REGISTRO OBSERVE_TX
//-------------------------------
#define	PLOS_CNT			BIT4
#define	ARC_CNT				BIT0


// MNEMONIC REGISTRO CD
//-------------------------------
#define	nmCD				BIT0


// MNEMONIC REGISTRO FIFO_STATUS
//-------------------------------
#define	TX_REUSE			BIT6
#define	nmTX_FULL			BIT5
#define	TX_EMPTY			BIT4
#define	RX_FULL				BIT1
#define	RX_EMPTY			BIT0


// MNEMONIC REGISTRO DYNPD
//-------------------------------
#define	DPL_P5				BIT5
#define	DPL_P4				BIT4
#define	DPL_P3				BIT3
#define	DPL_P2				BIT2
#define	DPL_P1				BIT1
#define	DPL_P0				BIT0


// MNEMONIC REGISTRO FEATURE
//-------------------------------
#define	EN_DPL				BIT2
#define	EN_ACK_PAY			BIT1
#define	EN_DYN_ACK			BIT0


// COMANDOS
//-------------------------------
#define	R_REGISTER			0x00	/*  Read  Register   */
#define	W_REGISTER			0x20	/*  Write Register   */
#define	R_RX_PAYLOAD		0x61	/*  Read  Packets    */
#define	W_TX_PAYLOAD		0xA0	/*  Write Packets  	 */
#define FLUSH_TX			0xE1	/*  EMPTY TX Buffer  */
#define FLUSH_RX			0xE2	/*  EMPTY RX Buffer  */
#define REUSE_TX_PL			0xE3
#define ACTIVATE			0x50
#define R_RX_PL_WID			0x60
#define W_ACK_PAYLOAD		0xA8	/**<  Rx: Payload  +  AUTO ACK  */
#define W_TX_PAYLOAD_NOACK	0xB0	/**<  Tx: DEACTIVE TX AUTO ACK */
#define NOP					0xFF	/**<  This can be used to read the STATUS register */


// Function Prototypes
//-------------------------------

void nRF24L01_init ();
void nRF24L01_reset ();

uint8_t *nRF24L01_WriteRead (uint16_t , uint16_t , uint8_t *, uint16_t );
uint8_t nRF24L01_send ( uint8_t *, uint16_t );
void nRF24L01_setAddress ( uint8_t ADDR );

