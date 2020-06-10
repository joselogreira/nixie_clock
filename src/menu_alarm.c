/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file menu_alarm.c
 * @brief Alarm-related functions
 *
 * Utility functions to change the alarm options
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "menu_alarm.h"

#include <stdint.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

typedef struct {
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
} snooze_s;

#define SNOOZE_TIME 	5		// in minutes

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static void increment_alarm(uint8_t what);
static void change_theme(uint8_t dir);
static void init_snooze_time(snooze_s *p1, snooze_s *p2);
static uint8_t check_snooze_time(snooze_s *p);

/*===========================================================================*/
/*
* ALARM ENABLED
* User configures wether the alarm is enabled or disabled
*/
state_t set_alarm_active(state_t state)
{
	static uint8_t intro = TRUE;
	static uint16_t count = 0;
	static uint8_t toggle = 0;

	if(intro){
		intro = FALSE;
		display.set = ON;
		display.fade_level[0] = 5;
		display.fade_level[1] = 5;
		display.fade_level[2] = 5;
		display.fade_level[3] = 5;
		display.d1 = BLANK;
		display.d2 = BLANK;
		display.d3 = BLANK;
		count = 0;
		timer_leds_set(ENABLE, 0, 50, 50);
	}

	/*
	*	DISPLAY TRANSITIOS
	*	The animation simply consist of blinking the option as if there were
	*	a cursor, to indicate that the option can be changed. 	
	*/
	if(alarm.active) display.d4 = 1;
	else display.d4 = 0;
	if(toggle) display.set = ON;
	else display.set = OFF;

	/*
	*	BUTTONS ACTIONS
	*/
	// If X pressed, return to the menu
	if((btnX.action) && (btnX.state == BTN_RELEASED) && (!btnX.delay1)){
		btnX.action = FALSE;
		state = DISPLAY_MENU;
		intro = TRUE;
	}
	// If X pressed and hold, return to the time display
	if((btnX.action) && (btnX.delay3)){
		state = DISPLAY_TIME;
		btnX.action = FALSE;
		intro = TRUE;
	}
	// If Y or Z pressed, toggle the alarm state (enable/disable)
	if(((btnY.action) && (!btnY.delay1)) || ((btnZ.action) && (!btnZ.delay1))){
		btnY.action = FALSE;
		btnZ.action = FALSE;
		alarm.active ^= 1;
		count = 0;
	}

	/*
	* 	GENERAL FUNCTION COUNTER
	*/
	count++;
	if(!(count % 100)){
		toggle ^= 1;
		if(count >= 30000){
			state = DISPLAY_MENU;
			count = 0;
			intro = TRUE;
		}
	}
	
	return state;
}

/*===========================================================================*/
/*
* SET ALARM TIME
* User configures the time the alarm is to be triggered
*/
state_t set_alarm(state_t state)
{
	static uint8_t intro = TRUE;
	static uint16_t count = 0;
	static uint8_t toggle = 0;
	static uint8_t selection = 1;
	static uint8_t display_mode = DISP_MODE_1;

	if(intro){
		intro = FALSE;
		display.set = ON;
		display_mode = DISP_MODE_1;
		display.fade_level[0] = 5;
		display.fade_level[1] = 5;
		display.fade_level[2] = 5;
		display.fade_level[3] = 5;
		count = 0;
		selection = 1;
		if(alarm.day_period == PERIOD_AM) timer_leds_set(ENABLE, 0, 150, 0);
		else if(alarm.day_period == PERIOD_PM) timer_leds_set(ENABLE, 0, 0, 150);
	}

	/*
	*	DISPLAY TRANSITIOS
	*	The animation simply consist of blinking the digits as if there were
	*	a cursor, to indicate that the quantity can be changed. If the button
	* 	is kept pushed, blinking stops and fast increment occurs
	*/
	switch(display_mode){

		case DISP_MODE_0:

			if((toggle) || (btnZ.state == BTN_PUSHED)){
				display.d1 = alarm.h_tens;
				display.d2 = alarm.h_units;
				display.d3 = alarm.m_tens;
				display.d4 = alarm.m_units;
			} else {
				if(selection){
					display.d1 = BLANK;
					display.d2 = BLANK;
				} else {
					display.d3 = BLANK;
					display.d4 = BLANK;
				}
			}
			break;

		case DISP_MODE_1:

			if(!(count % 30)){
				display.fade_level[0]--;
				display.fade_level[1]--;
				display.fade_level[2]--;
				display.fade_level[3]--;
				if(display.fade_level[0] == 0){
					count = 0;
					display.d1 = BLANK;
					display.d2 = BLANK;
					display.d3 = BLANK;
					display.d4 = BLANK;	
					display.fade_level[0] = 5;
					display.fade_level[1] = 5;
					display.fade_level[2] = 5;
					display.fade_level[3] = 5;
					display_mode = DISP_MODE_0;
				}
			}
			break;
	}

	/*
	*	BUTTONS ACTIONS
	*/
	// If X pressed, return to menu
	if((btnX.action) && (btnX.state == BTN_RELEASED) && (!btnX.delay1)){
		btnX.action = FALSE;
		state = DISPLAY_MENU;
		intro = TRUE;
	}
	// If X pressed and hold, return to display the time
	if((btnX.action) && (btnX.delay3)){
		btnX.action = FALSE;
		state = DISPLAY_TIME;
		intro = TRUE;
	}
	// If Y pressed, toggle selection between hours and minutes
	if((btnY.action) && (!btnY.delay1)){
		btnY.action = FALSE;
		selection ^= 1;
		count = 0;
	}
	// If Z pressed, increment the selected quantity. If pressed and hold,
	// fast increment of the quantity
	if(btnZ.action){
		if(btnZ.state == BTN_RELEASED){
			if(selection) {
				increment_alarm(INC_HOUR);
				update_time_variables();
			} else {
				increment_alarm(INC_MIN);
				update_time_variables();
			}
			btnZ.action = FALSE;
		} else if((btnZ.delay1) && (btnZ.delay2)){
			btnZ.delay2 = FALSE;
			if(selection){
				increment_alarm(INC_HOUR);
				update_time_variables();
			} else {
				increment_alarm(INC_MIN);
				update_time_variables();
			}
		}
		count = 0;
	}

	/*
	* 	GENERAL FUNCTION COUNTER
	*/
	count++;
	if(!(count % 100)){
		toggle ^= 1;
		if(count >= 30000){
			state = DISPLAY_MENU;
			count = 0;
			intro = TRUE;
		}
	}
	
	return state;
}

/*===========================================================================*/
/*
* ALARM TRIGGERED
* When alarm is enabled and triggered, the clock automatically enters this
* function, the music sounds and the LEDs toggle colors up to 1 minute.
* The function implements two snooze times at a predefined value of 5 mins
* and 10 mins ahead of the current alarm. The behavior of the snooze time
* and buttons is handled within the switch() statement
*/
state_t alarm_triggered(state_t state)
{
	static uint8_t intro = TRUE;
	static uint16_t count = 0;
	static uint16_t count2 = 0;
	static uint8_t toggle = 0;
	static uint8_t leds_toggle = 0;
	static uint8_t buzz_state = ENABLE;
	static uint8_t step = 0;
	static uint8_t snooze = 0;
	static snooze_s snooze_time_1, snooze_time_2;

	if(intro){
		intro = FALSE;
		display.set = ON;
		display.fade_level[0] = 5;
		display.fade_level[1] = 5;
		display.fade_level[2] = 5;
		display.fade_level[3] = 5;
		count = 0;
		count2 = 0;
		buzz_state = ENABLE;
		step = 0;
		snooze = 0;
		init_snooze_time(&snooze_time_1, &snooze_time_2);
	}

	/*
	*	LEDs sequence
	*	Toggle LEDs every 300ms. 
	*/
	if(!(count % 300)){
		leds_toggle ^= 1;
		if(leds_toggle) timer_leds_set(ENABLE, 250, 10, 0);
		else timer_leds_set(ENABLE, 10, 250, 0);
	}

	/*
	*	DISPLAY ALARM animation
	*   Just shows the time
	*/
	display.d1 = time.h_tens;
	display.d2 = time.h_units;
	display.d3 = time.m_tens;
	display.d4 = time.m_units;

	/* 
	* The alarm and snooze time are handled like a pseudo states-machine.
	* The alarm is triggered at the time set by the user, and immediately 
	* two snooze times are computed. 
	* The states-machine only has 2 states, but with a slightly different
	* behavior according to the value of "snooze", which represents the
	* cycle the state machin is, at a certain time (One "cycle" is the 
	* transition from state 0 to state 1 and back to state 0).
	* - State 0: The MCU triggers the alarm theme and loops until either a
	*   button is pressed or 1 min has elapsed. If so, jump to state 1 and 
	*   increment the cycle counter (snooze). After 2 cycles, tha alarm gets
	*   disabled and execution returns to normal clock operation.
	* - State 1: The alarm theme is stopped and the MCU loops until either
	*   the following snooze_time_X is reached, or a button is pressed. If
	*   snooze time is reached, jump back to state 0 where the alarm is 
	*   triggered; if a button is pressed, exit the alarm and return back 
	*   to normal operation
	*/
	switch(step){

		// Alarm triggered. Wait for a button press or 1 minute elapsed
		case 0:
			count2++;
			if((count2 >= 60000) || (btnX.action && !btnX.delay3) || (btnY.action && !btnY.delay3) || (btnZ.action && !btnZ.delay3)){
			   	btnX.action = FALSE;
				btnY.action = FALSE;
				btnZ.action = FALSE;
				count2 = 0;
				buzz_state = DISABLE;
				if((snooze == 0) || (snooze == 1)){
					step = 1;
				} else if(snooze == 2){
					alarm.triggered = FALSE;
					state = DISPLAY_TIME;
					buzz_state = DISABLE;
					intro = TRUE;
				}
				snooze++;
			}
			break;

		// Alarm sound disable. Wait for next snooze time or a button press.
		case 1:
			if(time.update){
				time.update = FALSE;
				if(snooze == 1){
					if(check_snooze_time(&snooze_time_1)){
						step = 0;
						buzz_state = ENABLE;
						count2 = 0;
					}
				} else if(snooze == 2){
					if(check_snooze_time(&snooze_time_2)){
						step = 0;
						buzz_state = ENABLE;
						count2 = 0;
					}
				}
			}
			if((btnX.action && !btnX.delay3) || (btnY.action && !btnY.delay3) || (btnZ.action && !btnZ.delay3)){
				btnX.action = FALSE;
				btnY.action = FALSE;
				btnZ.action = FALSE;
				alarm.triggered = FALSE;
				state = DISPLAY_TIME;
				buzz_state = DISABLE;
				intro = TRUE;
			}
			break;
	}
	
	/*
	* 	GENERAL FUNCTION COUNTER
	*/
	if(!(count % 100))
		toggle ^= 1;
	count++;

	/*
	* 	ALARM sound
	*   buzzer_music() is implemented so that, according to the state of the
	*	flag "buzz_state", the music plays or mutes
	*/
	buzzer_music(alarm.theme, buzz_state);

	return state;
}

/*===========================================================================*/
/*
* ALARM MUSIC
* User can choose from among 6 different pre-loaded tones to choose
* as the alarm music
*/
state_t set_alarm_theme(state_t state)
{
	static uint8_t intro = TRUE;
	static uint16_t count = 0;
	static uint8_t toggle = 0;

	if(intro){
		intro = FALSE;
		display.set = ON;
		display.fade_level[0] = 5;
		display.fade_level[1] = 5;
		display.fade_level[2] = 5;
		display.fade_level[3] = 5;
		display.d1 = BLANK;
		display.d2 = BLANK;
		display.d3 = BLANK;
		count = 0;
		timer_leds_set(ENABLE, 50, 50, 50);
	}

	/*
	*	DISPLAY TRANSITIOS
	*	The selected tone blinks to indicate that it can be changed.
	*	Display values are updated according to the buttons pressed
	*/
	if(toggle) {
		if(alarm.theme == SIMPLE_ALARM) display.d4 = 1;
		else if(alarm.theme == MAJOR_SCALE) display.d4 = 2;
		else if(alarm.theme == STAR_WARS) display.d4 = 3;
		else if(alarm.theme == IMPERIAL_MARCH) display.d4 = 4;
		else if(alarm.theme == SUPER_MARIO) display.d4 = 5;
		else if(alarm.theme == USA_ANTHEM) display.d4 = 6;
		else if(alarm.theme == DIOMEDES) display.d4 = 7;
	} else {
		display.d4 = BLANK;
	}

	// Play the selected music tone while in this menu option
	buzzer_music(alarm.theme, ENABLE);
	
	/* 
	*	BUTTONS
	*/
	// If Y pressed, switch to the previous tone
	if((btnY.action) && (!btnY.delay1)){
		btnY.action = FALSE;
		change_theme(DOWN);
		count = 0;
	}
	// If Y pressed, switch to the next tone
	if((btnY.action) && (!btnY.delay1)){
		btnZ.action = FALSE;
		change_theme(UP);
		count = 0;
	}
	// If X pressed, return to the menu
	if((btnX.action) && (btnX.state == BTN_RELEASED) && (!btnX.delay1)){
		btnX.action = FALSE;
		state = DISPLAY_MENU;
		intro = TRUE;
		buzzer_music(alarm.theme, DISABLE);
	}
	// If X pressed and hold, return to display the time 
	if((btnX.action) && (btnX.delay3)){
		btnX.action = FALSE;
		state = DISPLAY_TIME;
		intro = TRUE;
		buzzer_music(alarm.theme, DISABLE);
	}

	/*
	* 	GENERAL FUNCTION COUNTER
	*/
	count++;
	if(!(count % 100)){
		toggle ^= 1;
		if(count >= 30000){
			state = DISPLAY_MENU;
			buzzer_music(alarm.theme, DISABLE);
			count = 0;
			intro = TRUE;
		}
	}
	
	return state;
}

/*-----------------------------------------------------------------------------
-------------------------- L O C A L   F U N C T I O N S ----------------------
-----------------------------------------------------------------------------*/

/*===========================================================================*/
static void increment_alarm(uint8_t what)
{
	if(what == INC_HOUR){
		if(alarm.hour_mode == MODE_12H){
			if(alarm.hour == 11){
				alarm.hour++;
				if(alarm.day_period == PERIOD_AM) alarm.day_period = PERIOD_PM;
				else alarm.day_period = PERIOD_AM;
			} else if(alarm.hour == 12){
				alarm.hour = 1;
			} else {
				alarm.hour++;
			}
		} else if(alarm.hour_mode == MODE_24H){
			if(alarm.hour == 11){
				alarm.hour++;
				alarm.day_period = PERIOD_PM;
			} else if(alarm.hour == 23){
				alarm.hour = 0;
				alarm.day_period = PERIOD_AM;
			} else {
				alarm.hour++;
			}
		}
	} else if(what == INC_MIN){
		if(alarm.min == 59) alarm.min = 0;
		else alarm.min++;
	} else if(what == INC_SEC){
		if(alarm.sec == 59) alarm.sec = 0;
		else alarm.sec++;
	}

	// LEDs update
	if(alarm.day_period == PERIOD_AM) timer_leds_set(ENABLE, 0, 150, 0);
	else if(alarm.day_period == PERIOD_PM) timer_leds_set(ENABLE, 0, 0, 150);
}

/*===========================================================================*/
static void change_theme(uint8_t dir)
{
	if(dir){
		if(alarm.theme == SIMPLE_ALARM) alarm.theme = MAJOR_SCALE;
		else if(alarm.theme == MAJOR_SCALE) alarm.theme = STAR_WARS;
		else if(alarm.theme == STAR_WARS) alarm.theme = IMPERIAL_MARCH;
		else if(alarm.theme == IMPERIAL_MARCH) alarm.theme = SUPER_MARIO;
		else if(alarm.theme == SUPER_MARIO) alarm.theme = USA_ANTHEM;
		else if(alarm.theme == USA_ANTHEM) alarm.theme = DIOMEDES;
		else if(alarm.theme == DIOMEDES) alarm.theme = SIMPLE_ALARM;
	} else {
		if(alarm.theme == SIMPLE_ALARM) alarm.theme = DIOMEDES;
		else if(alarm.theme == MAJOR_SCALE) alarm.theme = SIMPLE_ALARM;
		else if(alarm.theme == STAR_WARS) alarm.theme = MAJOR_SCALE;
		else if(alarm.theme == IMPERIAL_MARCH) alarm.theme = STAR_WARS;
		else if(alarm.theme == SUPER_MARIO) alarm.theme = IMPERIAL_MARCH;
		else if(alarm.theme == USA_ANTHEM) alarm.theme = SUPER_MARIO;
		else if(alarm.theme == DIOMEDES) alarm.theme = USA_ANTHEM;
	}	

	buzzer_music(alarm.theme, DISABLE);
}

/*===========================================================================*/
static void init_snooze_time(snooze_s *p1, snooze_s *p2)
{
	// Snooze Alarm 1: SNOOZE_TIME minutes ahead of the current alarm
	(*p1).sec = 0;
	(*p1).min = alarm.min + SNOOZE_TIME;
	if((*p1).min > 59) {
		(*p1).min = alarm.min + SNOOZE_TIME - 60;
		if(alarm.hour_mode == MODE_24H){
			if(alarm.hour == 23) (*p1).hour = 0;
			else (*p1).hour = alarm.hour + 1;
		} else {
			if(alarm.hour == 12) (*p1).hour = 1;
			else  (*p1).hour = alarm.hour + 1;
		}		
	} else {
		(*p1).hour = alarm.hour;
	}

	// Snooze Alarm 2: 2*SNOOZE_TIME minutes ahead of the current alarm
	(*p2).sec = 0;
	(*p2).min = alarm.min + (2*SNOOZE_TIME);
	if((*p2).min > 59) {
		(*p2).min = alarm.min + (2*SNOOZE_TIME) - 60;
		if(alarm.hour_mode == MODE_24H){
			if(alarm.hour == 23) (*p2).hour = 0;
			else (*p2).hour = alarm.hour + 1;
		} else {
			if(alarm.hour == 12) (*p2).hour = 1;
			else  (*p2).hour = alarm.hour + 1;
		}
	} else {
		(*p2).hour = alarm.hour;
	}
}

/*===========================================================================*/
static uint8_t check_snooze_time(snooze_s *p)
{
	uint8_t match = FALSE;

	if((*p).hour == time.hour){
		if((*p).min == time.min){
			if((*p).sec == time.sec){	
				match = TRUE;
				alarm.triggered = TRUE;
			}
		}
	}
	
	return match;
}