/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
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
#include "buzzer.h"
#include "config.h"
#include "timers.h"
#include "uart.h"

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

//- - - - - L O C A L   S C O P E   F U N C T I O N S - - - - -

static void tubes_off(void){
// Disables all tubes' anodes

	PORTA &= ~(1<<PORTA4);
	PORTE &= ~(1<<PORTE5);
	PORTD &= ~(1<<PORTD0);
	PORTD &= ~(1<<PORTD1);
}

static void digits_off(void){
// Disables all tubes' cathodes

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

//- - - - - G L O B A L   S C O P E   F U N C T I O N S - - - - -

void set_tube(uint8_t t){
// Sets only one tubes' anode

	tubes_off();
	if(t == TUBE_A) PORTA |= (1<<PORTA4);
	else if (t == TUBE_B) PORTE |= (1<<PORTE5);
	else if (t == TUBE_C) PORTD |= (1<<PORTD0);
	else if (t == TUBE_D) PORTD |= (1<<PORTD1);
}

void set_digit(uint8_t n){
// Sets only one tubes' cathode

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

void buttons_check(void){ 	
		/*	BTN_X - PB5
    		BTN_Y - PB6
    		BTN_Z - PB7 */

	// BUTTON X
	if(btnX.query){

		switch(btnX.state){

			case BTN_IDLE:
				if(!BUTTON_X) btnX.count++;
				else if(btnX.count > 0) btnX.count--;

				if(btnX.count == 7){
					btnX.action = TRUE;
					btnX.lock = TRUE;
					btnX.state = BTN_PUSHED;
					btnX.count = 0;
					buzzer_set(ENABLE, N_C8);
				} else if(btnX.count == 0){
					btnX.action = FALSE;
					btnX.lock = FALSE;
					btnX.query = FALSE;
				}
				break;

			case BTN_PUSHED:
				if(!BUTTON_X){
					btnX.count++;	
				} else {
					btnX.count = 0;
					btnX.state = BTN_RELEASED;
				}
				if(btnX.count == BTN_BEEP_TIME) buzzer_set(DISABLE, N_C8);
				if(btnX.count == BTN_DLY1_TIME) btnX.delay1 = TRUE;
				if((btnX.delay1) && (!(btnX.count % BTN_DLY2_TIME))) btnX.delay2 = TRUE;
				if(btnX.count >= BTN_DLY3_TIME) btnX.delay3 = TRUE;
				break;

			case BTN_RELEASED:
				if(BUTTON_X)
					btnX.count++;
				if(btnX.count == BTN_LOCK_TIME){
					btnX.query = FALSE;
					btnX.action = FALSE;
					btnX.lock = FALSE;
					btnX.state = BTN_IDLE;
					btnX.count = 0;
					btnX.delay1 = FALSE;
					btnX.delay2 = FALSE;
					btnX.delay3 = FALSE;
					buzzer_set(DISABLE, N_C8);
				}
				break;
			default:
				break;
		}
	}

	// BUTTON Y
	if(btnY.query){

		switch(btnY.state){

			case BTN_IDLE:
				if(!BUTTON_Y) btnY.count++;
				else if(btnY.count > 0) btnY.count--;

				if(btnY.count == 7){
					btnY.action = TRUE;
					btnY.lock = TRUE;
					btnY.state = BTN_PUSHED;
					btnY.count = 0;
					buzzer_set(ENABLE, N_C8);
				} else if(btnY.count == 0){
					btnY.action = FALSE;
					btnY.lock = FALSE;
					btnY.query = FALSE;
				}
				break;

			case BTN_PUSHED:
				if(!BUTTON_Y){
					btnY.count++;	
				} else {
					btnY.count = 0;
					btnY.state = BTN_RELEASED;
				}
				if(btnY.count == BTN_BEEP_TIME) buzzer_set(DISABLE, N_C8);
				if(btnY.count == BTN_DLY1_TIME) btnY.delay1 = TRUE;
				if((btnY.delay1) && (!(btnY.count % BTN_DLY2_TIME))) btnY.delay2 = TRUE;
				if(btnY.count >= BTN_DLY3_TIME) btnY.delay3 = TRUE;
				break;

			case BTN_RELEASED:
				if(BUTTON_Y)
					btnY.count++;
				if(btnY.count == BTN_LOCK_TIME){
					btnY.query = FALSE;
					btnY.action = FALSE;
					btnY.lock = FALSE;
					btnY.state = BTN_IDLE;
					btnY.count = 0;
					btnY.delay1 = FALSE;
					btnY.delay2 = FALSE;
					btnY.delay3 = FALSE;
					buzzer_set(DISABLE, N_C8);
				}
				break;
			default:
				break;
		}
	}

	// BUTTON Z
	if(btnZ.query){

		switch(btnZ.state){

			case BTN_IDLE:
				if(!BUTTON_Z) btnZ.count++;
				else if(btnZ.count > 0) btnZ.count--;

				if(btnZ.count == 7){
					btnZ.action = TRUE;
					btnZ.lock = TRUE;
					btnZ.state = BTN_PUSHED;
					btnZ.count = 0;
					buzzer_set(ENABLE, N_C8);
				} else if(btnZ.count == 0){
					btnZ.action = FALSE;
					btnZ.lock = FALSE;
					btnZ.query = FALSE;
				}
				break;

			case BTN_PUSHED:
				if(!BUTTON_Z){
					btnZ.count++;	
				} else {
					btnZ.count = 0;
					btnZ.state = BTN_RELEASED;
				}
				if(btnZ.count == BTN_BEEP_TIME) buzzer_set(DISABLE, N_C8);
				if(btnZ.count == BTN_DLY1_TIME) btnZ.delay1 = TRUE;
				if((btnZ.delay1) && (!(btnZ.count % BTN_DLY2_TIME))) btnZ.delay2 = TRUE;
				if(btnZ.count >= BTN_DLY3_TIME) btnZ.delay3 = TRUE;
				break;

			case BTN_RELEASED:
				if(BUTTON_Z)
					btnZ.count++;
				if(btnZ.count == BTN_LOCK_TIME){
					btnZ.query = FALSE;
					btnZ.action = FALSE;
					btnZ.lock = FALSE;
					btnZ.state = BTN_IDLE;
					btnZ.count = 0;
					btnZ.delay1 = FALSE;
					btnZ.delay2 = FALSE;
					btnZ.delay3 = FALSE;
					buzzer_set(DISABLE, N_C8);
				}
				break;
			default:
				break;
		}
	}
}

void led_blink(uint8_t n, uint8_t time){

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

uint8_t check_alarm(void){

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

void update_time_variables(void){

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

uint8_t random_number(uint8_t seed){

	uint8_t rand;

	rand = ((4 * seed) + 1) % 9;

	return rand;

}

/*-------------- DEBUG FUNCTIONS ----------------*/

void bin_to_ascii(char *p, uint8_t bin){

	uint8_t tmp = bin;

	for(uint8_t i = 0; i < 8; i++){
		if(tmp & 0x80) *(p + i) = '1';
		else *(p + i) = '0';
		tmp <<= 1;
	}
}