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
#include "config.h"

#include <avr/io.h>
#include <stdint.h>

/******************************************************************************
*************** G L O B A L   V A R S   D E F I N I T I O N S *****************
******************************************************************************/

btn_s btnX, btnY, btnZ;

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

// Button pins
#define BUTTON_X		(PINB & (1<<PINB7))
#define BUTTON_Y		(PINB & (1<<PINB6))
#define BUTTON_Z		(PINB & (1<<PINB5))

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

/*===========================================================================*/
void buttons_init(void)
{
	// STRUCTURE - btnX
	btnX.query = FALSE;
	btnX.action = FALSE;
	btnX.lock = FALSE;
	btnX.state = BTN_IDLE;
	btnX.count = 0;
	btnX.delay1 = FALSE;
	btnX.delay2 = FALSE;
	btnX.delay3 = FALSE;
	btnX.check = btn_check_x;
	// STRUCTURE - btnY
	btnY.query = FALSE;
	btnY.action = FALSE;
	btnY.lock = FALSE;
	btnY.state = BTN_IDLE;
	btnY.count = 0;
	btnY.delay1 = FALSE;
	btnY.delay2 = FALSE;
	btnY.delay3 = FALSE;
	btnY.check = btn_check_y;
	// STRUCTURE - btnZ
	btnZ.query = FALSE;
	btnZ.action = FALSE;
	btnZ.lock = FALSE;
	btnZ.state = BTN_IDLE;
	btnZ.count = 0;
	btnZ.delay1 = FALSE;
	btnZ.delay2 = FALSE;
	btnZ.delay3 = FALSE;
	btnZ.check = btn_check_z;
}

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