/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file external_interrupt.c
 * @brief External interrupt config
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "external_interrupt.h"

#include <avr/io.h>
#include <stdint.h>

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

/*===========================================================================*/
/*
	EXT_PWR		- PC2 - PCINT18	| -> PCI2
	BTN_X		- PB5 - PCINT13 | -> PCI1
	BTN_Y		- PB6 - PCINT14 | -> PCI1
	BTN_Z		- PB7 - PCINT15 | -> PCI1
*/
void pin_change_isr_init(void)
{
	PCICR |= (1<<PCIE1) | (1<<PCIE2);	// Enables pin toggle interrupts for:
										// - External power sensing
										// - buttons push
	PCMSK1 |= (1<<PCINT15) | (1<<PCINT14) | (1<<PCINT13);
	PCMSK2 |= (1<<PCINT18);
}

/*===========================================================================*/
void buttons_set(uint8_t state)
{
	if(state){
		// Enable ISR for buttons
		PCMSK1 |= (1<<PCINT15) | (1<<PCINT14) | (1<<PCINT13);
		// clear possible triggered interrupts
		PCIFR |= (1<<PCIF1);
	} else {
		// Disable ISR for buttons
		PCMSK1 &= ~((1<<PCINT15) | (1<<PCINT14) | (1<<PCINT13));
	}
}

/*===========================================================================*/
uint8_t inline btn_check_x(void)
{
	return BUTTON_X;
}

/*===========================================================================*/
uint8_t inline btn_check_y(void)
{
	return BUTTON_Y;
}

/*===========================================================================*/
uint8_t inline btn_check_z(void)
{
	return BUTTON_Z;
}