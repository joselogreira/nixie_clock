/**
 * @file timers.c
 * @brief Timer counter configurations
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "timers.h"

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

/******************************************************************************
*************** G L O B A L   V A R S   D E F I N I T I O N S *****************
******************************************************************************/

display_s display;

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

/*===========================================================================*/
void display_init(void)
{
	display.mode = DISP_MODE_4;
	display.d1 = 0;
	display.d2 = 0;
	display.d3 = 0;
	display.d4 = 0;
	display.set = ON;	
	display.fade_level[0] = 5;
	display.fade_level[1] = 5;
	display.fade_level[2] = 5;
	display.fade_level[3] = 5;
}

/*===========================================================================*/
/*
* TIMER COUNTER 2
* Asynchronous operation using external 32.767KHz crystal
* Normal mode for TC2
*/
void timer_rtc_set(uint8_t state){

	// disable interrupts if enabled
	TIMSK2 &= ~((1<<OCIE2B) | (1<<OCIE2A) | (1<<TOIE2));
	// asynchronous operation:
	ASSR |= (1<<AS2);
	TCNT2 = 0;
	OCR2A = 0;
	OCR2B = 0;

	if(state){
		TCCR2B |= (1<<CS22) | (1<<CS20);	// Prescaler 128. Start TC2
		// wait register to be written
		while(ASSR & ((1<<TCN2UB) | (1<<OCR2AUB) | (1<<OCR2BUB) | (1<<TCR2BUB)));	
		// clear flags if set
		TIFR2 |= (1<<TOV2) | (1<<OCF2B) | (1<<OCF2A);
		TIMSK2 |= (1<<TOIE2);				// Overflow interrupts
		// wait for crystal to stabilize
		_delay_ms(100);	
	} else {
		TCCR2B &= ~((1<<CS22) | (1<<CS20));	// Prescaler STOPPED
		// wait register to be written
		while(ASSR & ((1<<TCN2UB) | (1<<OCR2AUB) | (1<<OCR2BUB) | (1<<TCR2BUB)));	
		// clear flags if set
		TIFR2 |= (1<<TOV2) | (1<<OCF2B) | (1<<OCF2A);
		TIMSK2 &= ~(1<<TOIE2);				// Disabe overflow interrupts
	}
}

/*===========================================================================*/
/*
* TIMER COUNTER 3
*/
void timer_base_init(void)
{
	TCCR3B |= (1<<WGM32);	// CTC mode, TOP: OCR3A
	TCNT3 = 0;
	OCR3A = 250;			// 250 -> isr freq = 2MHz/8/250 = 1KHz
	TIFR3 |= (1<<OCF3A);	// clear interrupt flag, if set.
	TIMSK3 |= (1<<OCIE3A);	// Interrupts for compare match
	//TCCR3B |= (1<<CS31); 	// Prescaler 8. Start TC3
}

/*===========================================================================*/
/*
* TIMER COUNTER 4
*/
void timer_buzzer_init(void)
{
	TCCR4B |= (1<<WGM42);	// CTC mode, TOP: OCR4A
	TCCR4A |= (1<<COM4A0);	// toggle pin OC4A on compare match
	TCNT4 = 0;
	// no interrupts used
	OCR4A = 0xFFFF;
	//TCCR4B |= (1<<CS40);	// Prescaler 1; Start TC4
}

/*===========================================================================*/
/*
* TIMER COUNTER 0 (OC0A & OC0B)
*/
void timer_leds_init(void)
{
	TCCR0A |= (1<<WGM01) | (1<<WGM00);		// Fast PWM; TOP: 0xFF
	TCCR0A |= (1<<COM0A1) | (1<<COM0B1);	// Non-inverting mode
	// no interrupts used
	TCNT0 = 0;
	OCR0A = 0;
	OCR0B = 0;
	//TCCR0B |= (1<<CS00);		// Prescaler 1; Start TC0

	// TIMER COUNTER 1 (OC1A)
	TCCR1A |= (1<<WGM10);		// Fast PWM, 8 bits; TOP: 0x00FF
	TCCR1B |= (1<<WGM12);
	TCCR1A |= (1<<COM1A1);		// Non-inverting mode
	// no interrupts used
	TCNT1 = 0;
	OCR1A = 0;
	//TCCR1B |= (1<<CS10);		// Prescaler 1; start TC1
}

/*===========================================================================*/
void timer_base_set(uint8_t state)
{
	if(state){
		TIMSK3 |= (1<<OCIE3A);	// Interrupts for compare match
		TCNT3 = 0;
		TCCR3B |= (1<<CS31); 	// Prescaler 8. Start TC3
	} else {
		TCCR3B &= ~((1<<CS32) | (1<<CS31) | (1<<CS31));	// Stop prescaler
		TIMSK3 &= ~(1<<OCIE3A);	// Disable interrupts
	}
}

/*===========================================================================*/
void timer_buzzer_set(uint8_t state, uint16_t note)
{
	if(state){
		TCNT4 = 0;
		OCR4A = note;
		TCCR4A |= (1<<COM4A0);
		TCCR4B |= (1<<CS40);	// No prescaler, start PWM
	} else {
		TCCR4B &= ~((1<<CS42) | (1<<CS41) | (1<<CS40));
		TCCR4A &= ~((1<<COM4A1) | (1<<COM4A0));
	}
}

/*===========================================================================*/
void timer_leds_set(uint8_t state, uint8_t r, uint8_t g, uint8_t b)
{
	if(state){
		if((B_LED != b) || (R_LED != r) || (G_LED != g)){
			R_LED = r;
			G_LED = g;
			B_LED = b;
			TCNT0 = 0;
			TCNT1 = 0;
			TCCR0A |= (1<<COM0A1) | (1<<COM0B1);
			TCCR1A |= (1<<COM1A1);
			TCCR0B |= (1<<CS00);		// no prescaler
			TCCR1B |= (1<<CS10);		// no prescaler
		}		
	} else {
		TCCR0B &= ~((1<<CS01) | (1<<CS00));
		TCCR1B &= ~((1<<CS11) | (1<<CS10));
		TCCR0A &= ~((1<<COM0A1) | (1<<COM0A0) | (1<<COM0B1) | (1<<COM0B0));
		TCCR1A &= ~((1<<COM1A1) | (1<<COM1A0));
		R_LED = r;
		G_LED = g;
		B_LED = b;
	}
}