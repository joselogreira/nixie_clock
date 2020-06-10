/**
 * @file util.c
 * @brief Miscelaneous functions
 *
 * @author Jose Logreira
 * @date 07.05.2018
 *
 */
/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "util.h"

#include <stdint.h> 
#include <avr/io.h>
#include <util/delay.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

// Button time counts: These macros determine the time it takes for 
// different flags within btnXYZ structure to be set (in milliseconds)
#define BTN_LOCK_TIME	30		// lock time after button released
#define BTN_DLY1_TIME	300		// time for delay 1
#define BTN_DLY2_TIME	65		// time for delay 2
#define BTN_DLY3_TIME	2000	// time for delay 3
#define BTN_BEEP_TIME	50		// duration of beep sound

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static void tubes_off(void);
static void digits_off(void);

/*===========================================================================*/
/*
* Sets only one tubes' anode
*/
void set_tube(uint8_t t)
{
	tubes_off();
	if(t == TUBE_A) PORTA |= (1<<PORTA4);
	else if (t == TUBE_B) PORTE |= (1<<PORTE5);
	else if (t == TUBE_C) PORTD |= (1<<PORTD0);
	else if (t == TUBE_D) PORTD |= (1<<PORTD1);
}

/*===========================================================================*/
/*
* Sets only one tubes' cathode
*/
void set_digit(uint8_t n)
{
	digits_off();
	if(n == 0) PORTD |= (1<<PORTD2);
	else if(n == 1) PORTC |= (1<<PORTC3);
	else if(n == 2) PORTC |= (1<<PORTC5);
	else if(n == 3) PORTA |= (1<<PORTA7);
	else if(n == 4) PORTA |= (1<<PORTA6);
	else if(n == 5) PORTA |= (1<<PORTA5);
	else if(n == 6) PORTD |= (1<<PORTD3);
	else if(n == 7) PORTD |= (1<<PORTD4);
	else if(n == 8) //PORTD |= (1<<PORTD7);
					PORTD |= (1<<PORTD6);		// wrong in schematic
	else if(n == 9) //PORTD |= (1<<PORTD6);
					PORTD |= (1<<PORTD7);		// wrong in schematic
}

/*===========================================================================*/
/*	
* Buttons' debounce routune.
* Each button has its own structure. The function takes the respective button
* pointer and, using counters for assessing the bounce time, it determines
* the state the button is at.
*/
void buttons_check(btn_s *btn)
{
	if (btn->query) {

		switch(btn->state){

			case BTN_IDLE:
				if(!(btn->check)()) btn->count++;
				else if(btn->count > 0) btn->count--;

				if(btn->count == 7){
					btn->action = TRUE;
					btn->lock = TRUE;
					btn->state = BTN_PUSHED;
					btn->count = 0;
					buzzer_set(ENABLE);
				} else if(btn->count == 0){
					btn->action = FALSE;
					btn->lock = FALSE;
					btn->query = FALSE;
				}
				break;

			case BTN_PUSHED:
				if(!(btn->check)()){
					btn->count++;	
				} else {
					btn->count = 0;
					btn->state = BTN_RELEASED;
				}
				if(btn->count == BTN_BEEP_TIME) buzzer_set(DISABLE);
				if(btn->count == BTN_DLY1_TIME) btn->delay1 = TRUE;
				if((btn->delay1) && (!(btn->count % BTN_DLY2_TIME))) btn->delay2 = TRUE;
				if(btn->count >= BTN_DLY3_TIME) btn->delay3 = TRUE;
				break;

			case BTN_RELEASED:
				if((btn->check)())
					btn->count++;
				if(btn->count == BTN_LOCK_TIME){
					btn->query = FALSE;
					btn->action = FALSE;
					btn->lock = FALSE;
					btn->state = BTN_IDLE;
					btn->count = 0;
					btn->delay1 = FALSE;
					btn->delay2 = FALSE;
					btn->delay3 = FALSE;
					buzzer_set(DISABLE);
				}
				break;
			default:
				break;
		}
	}
}

/*===========================================================================*/
void led_blink(uint8_t n, uint8_t time)
{
	uint8_t i, j;

	PORTB &= ~(1<<PORTB1);

	RTC_SIGNAL_SET(LOW);
	for (j = 0; j < 2*n; j++){
		RTC_SIGNAL_TOGGLE();
		for (i = 0; i < time; i++){
			_delay_ms(1);
		}
	}
	RTC_SIGNAL_SET(LOW);
}

/*===========================================================================*/
uint8_t check_alarm(void)
{
	uint8_t match = FALSE;

	if(alarm.active){
		if(alarm.hour == time.hour){
			if(alarm.min == time.min){
				if(alarm.sec == time.sec){
					if(alarm.day_period == time.day_period){
						match = TRUE;
						alarm.triggered = TRUE;
					}
				}
			}
		}
	}

	return match;
}

/*===========================================================================*/
void update_time_variables(void)
{
	time.s_tens = time.sec / 10;
    time.s_units = time.sec % 10;
    time.m_tens = time.min / 10;
    time.m_units = time.min % 10;
    time.h_tens = time.hour / 10;
    time.h_units = time.hour % 10;

    alarm.s_tens = alarm.sec / 10;
    alarm.s_units = alarm.sec % 10;
    alarm.m_tens = alarm.min / 10;
    alarm.m_units = alarm.min % 10;
    alarm.h_tens = alarm.hour / 10;
    alarm.h_units = alarm.hour % 10;
}

/*===========================================================================*/
/*
* Random number algorithm.
* Based on a seed, these constant parameters selection makes the output to
* go through all numbers 0 to 9 before they start repeating.
* Can't remember where I took it, but it works nicely.
*/
uint8_t random_number(uint8_t seed)
{
	uint8_t rand;
	rand = ((4 * seed) + 1) % 9;
	return rand;
}

/*===========================================================================*/
/*
* DEBUG FUNCTIONS 
*/
void bin_to_ascii(char *p, uint8_t bin)
{
	uint8_t tmp = bin;

	for(uint8_t i = 0; i < 8; i++){
		if(tmp & 0x80) *(p + i) = '1';
		else *(p + i) = '0';
		tmp <<= 1;
	}
}

/*-----------------------------------------------------------------------------
--------------------- I N T E R N A L   F U N C T I O N S ---------------------
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*
* Disables all tubes' anodes
*/
static void tubes_off(void)
{
	PORTA &= ~(1<<PORTA4);
	PORTE &= ~(1<<PORTE5);
	PORTD &= ~(1<<PORTD0);
	PORTD &= ~(1<<PORTD1);
}

/*===========================================================================*/
/*
* Disables all tubes' cathodes
*/
static void digits_off(void)
{
	PORTD &= ~(1<<PORTD2);
	PORTC &= ~(1<<PORTC3);
	PORTC &= ~(1<<PORTC5);
	PORTA &= ~(1<<PORTA7);
	PORTA &= ~(1<<PORTA6);
	PORTA &= ~(1<<PORTA5);
	PORTD &= ~(1<<PORTD3);
	PORTD &= ~(1<<PORTD4);
	PORTD &= ~(1<<PORTD7);
	PORTD &= ~(1<<PORTD6);
}