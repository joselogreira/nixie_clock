/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file uart.c
 * @brief Serial communication
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "uart.h"

#include <avr/io.h>
#include <stdint.h>
#include <avr/pgmspace.h>	/* Program Memory Strings handling */
#include <util/delay.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

#define BAUD_REGISTER 	((uint16_t)(F_CPU/(8*BAUD)-1))

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static uint8_t uart_flush(void);

/*===========================================================================*/
void uart_init(void)
{
	/*Set baud rate */
	UBRR2H = (uint8_t) (BAUD_REGISTER>>8);
	UBRR2L = (uint8_t) (BAUD_REGISTER & 0x00FF);

	/* Double transmission speed */
	UCSR2A |= (1<<U2X);	// It provides more precise Baud rate

	/* Asynchronous operation, no parity, 1 stop bit */
	UCSR2C &= ~((1<<UMSEL1) | (1<<UMSEL0) | (1<<UPM1) | (1<<UPM0) | (1<<USBS));

	/* Set frame format: 8data */
	UCSR2C |= (1<<UCSZ1) | (1<<UCSZ0);
}

/*===========================================================================*/
void uart_send_char( char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR2A & (1<<UDRE)) );

	/* Put data into buffer, sends the data */
	UDR2 = data;

	/* Wait empty buffer again  */
	while ( !( UCSR2A & (1<<UDRE)) );
}

/*===========================================================================*/
void uart_send_string(const char *s)
{
	while (*s != '\0')
	{
		uart_send_char(*s);
		s++;
	}
}

/*===========================================================================*/
void uart_send_string_p(const char *s)
{
	while (pgm_read_byte(s) != '\0')
	{
		uart_send_char(pgm_read_byte(s));
		s++;
	}
}

/*===========================================================================*/
char uart_read_char( void )
{
	/* Wait for data to be received */
	while (!(UCSR2A & (1<<RXC)));

	/* Get and return received data from buffer */
	return UDR2;
}

/*===========================================================================*/
void uart_set(uint8_t state)
{
	if(state){
		/* Enable receiver and transmitter */
		UCSR2B |= (1<<RXEN)|(1<<TXEN);
		uart_flush();
	} else {
		/* Disable receiver and transmitter */
		UCSR2B &= ~((1<<RXEN)|(1<<TXEN));
	}
}

/*===========================================================================*/
uint8_t uart_check_rx(void)
{
	uint8_t x;

	// test three times. If any is wrong, return FALSE
	for(uint8_t i = 0; i < 3; i++){
		if(PINE & (1<<PINE2)) {		// If logic HIGH detected
			x = TRUE;
		} else {
			x = FALSE;
			break;
		}
		_delay_ms(1);
	}

	return x;
}

/*===========================================================================*/
static uint8_t uart_flush(void)
{
	char trash;

	while ((UCSR2A & (1<<RXC))){
		trash = UDR2;
	}

	return trash;
}