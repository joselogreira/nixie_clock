/**
 * @file menu_user.c
 * @brief Various user configurations
 *
 * Utility functions to configure some user options
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "menu_user.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

// 3D Sequence of digits stored as a vector. Used for an animation
static const uint8_t animation_3d_t1[] PROGMEM = {
	3,8,9,4,0,5,7,2,6,1,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};
static const uint8_t animation_3d_t2[] PROGMEM = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,1,6,2,7,5,0,4,9,8,3
};

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static uint8_t change_transition_mode(uint8_t dir);

/*===========================================================================*/
/*
* INTRO presentation
* Short animation that combines buzzer sound and the 3D effect
* IF at the end of the animation, button X is pressed: GO TO TEST SEQUENCE
*/
void intro(volatile state_t *state)
{
    uint8_t count = 0;
    uint8_t n = 0;
    uint8_t c = 0;
    uint8_t d = 0;

	uart_send_string_p(PSTR("\n\r\n\rHello World!\n\r"));
    display.set = ON;
	display.fade_level[0] = 5;
	display.fade_level[1] = 5;
	display.fade_level[2] = 5;
	display.fade_level[3] = 5;
	timer_leds_set(ENABLE, 250, 250, 250);
    
	/*
	* INFINITE LOOP
	*/
	while(TRUE){

		/*
		*	DISPLAY animation
		* 	3D sequence digits effect.
		*/
		display.d1 = pgm_read_byte(&animation_3d_t1[n]);
		display.d2 = pgm_read_byte(&animation_3d_t2[n]);
		display.d3 = pgm_read_byte(&animation_3d_t1[n]);
		display.d4 = pgm_read_byte(&animation_3d_t2[n]);

		// Counter sequence: Every 25ms transition to a new digit. 
		// Perform the whole 3D sequence, 4 times.
		count++;
		if((count == 25) && (c < 4)){
			count = 0;
			n++; 
			if(n >= sizeof(animation_3d_t1)){
				n = 0;
				c++;
			}
		}

		// Buzzer sound: Play sound twice.
		if(d < 2){
			if(buzzer_music(MAJOR_SCALE, ENABLE)) 
				d++;
		}
		
		/* 
		* BUTTONS check: Buttons are detected using an ISR which sets btnXYZ
	    * flags. Once set, the rest of the detection and debounce routine is
	    * handled within buttons_check(), based on the 1ms execution period of
	    * the main infinite loop
	    *
	    * BUTTONS actions:
		* - executed according to the buttons state flags
		*/
		if(btnX.query) buttons_check(&btnX);
	    if(btnY.query) buttons_check(&btnY);
	    if(btnZ.query) buttons_check(&btnZ);
		// If both things are finished (3D sequence and buzzer sound), exit.
		if((c >= 4) && (d >= 2)){
			display.d1 = BLANK;
			display.d2 = BLANK;
			display.d3 = BLANK;
			display.d4 = BLANK;
			// If X is pressed, jump to TEST_TUBES
			if(btnX.action){
				btnX.action = FALSE;
				*state = USR_TEST;
			} else {
				*state = DISPLAY_TIME;
			}
		}

		/* 
		* LOOP DELAY AND INTERRUPT ENABLE TIME --------------------------------
		* All interrupts are served within the sei()-cli() block. This is to 
		* avoid the extra care required for arbitrarily triggered ISRs and the 
		* use of atomic operations. "loop" flag is set every 1ms by a timer
		* whose ISR is enabled to produce interrupts every 1ms
		*/
		sei();
		// Wait for the next ms.
		while(!loop);
		loop = FALSE;
		cli();
		// If system state changed, exit fuction.
		if(*state != SYSTEM_INTRO)
			break;

	}	/* INFINITE LOOP */
}

/*===========================================================================*/
/*
* MAIN MENU
* From this menu, all clock configurations are performed.
* It handles all the menu modes (configuration modes)
* LEDs color in menu is PINK. Color inside every menu option changes
*/
void display_menu(volatile state_t *state)
{
	uint8_t menu_mode = 1;
	uint16_t count = 0;

	display.set = ON;
	display.d1 = 0;
	display.d3 = BLANK;
	display.d4 = BLANK;
	display.fade_level[0] = 5;
	display.fade_level[1] = 5;
	display.fade_level[2] = 5;
	display.fade_level[3] = 5;
	timer_leds_set(ENABLE, 250, 0, 250);

	/*
	* INFINITE LOOP
	*/
	while(TRUE){

		/*
		*	DISPLAY TRANSITIONS:
		*	implemented very simple: the current menu mode is the digit to be 
		*   displayed (from 1 to 6)
		*/
		display.d2 = menu_mode;

		/* 
		* BUTTONS check: Buttons are detected using an ISR which sets btnXYZ
	    * flags. Once set, the rest of the detection and debounce routine is
	    * handled within buttons_check(), based on the 1ms execution period of
	    * the main infinite loop
	    *
	    * BUTTONS actions:
		* - executed according to the buttons state flags
		*/
		if(btnX.query) buttons_check(&btnX);
	    if(btnY.query) buttons_check(&btnY);
	    if(btnZ.query) buttons_check(&btnZ);
		// If Y is pressed, decrement menu mode to the previous option
		if(btnY.action){
			btnY.action = FALSE;
			menu_mode--;
			if(menu_mode == 0) menu_mode = 6;
			count = 0;
		}
		// If Z is pressed, increment menu mode to the next option
		if(btnZ.action){
			btnZ.action = FALSE;
			menu_mode++;
			if(menu_mode == 7) menu_mode = 1;
			count = 0;
		}
		// If X is pressed, enter the selected menu mode. If pressed and hold,
		// go back to DISPLAY_TIME
		if(btnX.action){ 
			if(btnX.state == BTN_RELEASED){
				switch(menu_mode){
					case 1: *state = SET_TIME; break;
					case 2: *state = SET_ALARM; break;
					case 3: *state = SET_ALARM_ACTIVE; break;
					case 4: *state = SET_HOUR_MODE; break;
					case 5: *state = SET_TRANSITIONS; break;
					case 6: *state = SET_ALARM_THEME; break;
					default: *state = DISPLAY_TIME; break;
				}
				btnX.action = FALSE;
			} else if(btnX.delay3){
				*state = DISPLAY_TIME;
				btnX.action = FALSE;
			}
			count = 0;
		}

		/*
		*	GENERAL FUNCTION COUNTER and timeout
		*/
		count++;
		if(count == 30000){
			count = 0;
			*state = DISPLAY_TIME;
		}

		/* 
		* LOOP DELAY AND INTERRUPT ENABLE TIME --------------------------------
		* All interrupts are served within the sei()-cli() block. This is to 
		* avoid the extra care required for arbitrarily triggered ISRs and the 
		* use of atomic operations. "loop" flag is set every 1ms by a timer
		* whose ISR is enabled to produce interrupts every 1ms
		*/
		sei();
		// Wait for the next ms.
		while(!loop);
		loop = FALSE;
		cli();
		// If system state changed, exit fuction.
		if(*state != DISPLAY_MENU)
			break;

	}	/* INFINITE LOOP */
}

/*===========================================================================*/
/* 
* TRANSITION selection
* Four user-selectable transitions. Transitions are only visible when in
* DISPLAY_TIME mode, not here in the menu
*/
void set_transitions(volatile state_t *state)
{
	uint16_t count = 0;
	uint8_t toggle = 0;

	display.set = ON;
	display.fade_level[0] = 5;
	display.fade_level[1] = 5;
	display.fade_level[2] = 5;
	display.fade_level[3] = 5;
	display.d1 = BLANK;
	display.d2 = BLANK;
	display.d3 = BLANK;
	timer_leds_set(ENABLE, 100, 10, 10);
	
	/*
	* INFINITE LOOP
	*/
	while(TRUE){

		/*
		* DISPLAY transition
		* The current digit blinks to indicate that it can be changed.
		*/
		if(toggle){
			if(display.mode == DISP_MODE_1) display.d4 = 1;
			else if(display.mode == DISP_MODE_2) display.d4 = 2;
			else if(display.mode == DISP_MODE_3) display.d4 = 3;
			else if(display.mode == DISP_MODE_4) display.d4 = 4;
			else display.d4 = 0;
		} else {
			display.d2 = BLANK;
		}
		
		/* 
		* BUTTONS check: Buttons are detected using an ISR which sets btnXYZ
	    * flags. Once set, the rest of the detection and debounce routine is
	    * handled within buttons_check(), based on the 1ms execution period of
	    * the main infinite loop
	    *
	    * BUTTONS actions:
		* - executed according to the buttons state flags
		*/
		if(btnX.query) buttons_check(&btnX);
	    if(btnY.query) buttons_check(&btnY);
	    if(btnZ.query) buttons_check(&btnZ);
		// If Y pressed, switch to the previous transition animation
		if(btnY.action){
			btnY.action = FALSE;
			count = 0;
			display.mode = change_transition_mode(DOWN);
		}
		// If Z pressed, switch to the next transition animation
		if(btnZ.action){
			btnZ.action = FALSE;
			count = 0;
			display.mode = change_transition_mode(UP);
		}
		// If X pressed, return to the menu
		if((btnX.action) && (btnX.state == BTN_RELEASED) && (!btnX.delay1)){
			btnX.action = FALSE;
			*state = DISPLAY_MENU;
		}
		// If X pressed and hold, return to display the time
		if((btnX.action) && (btnX.delay3)){
			*state = DISPLAY_TIME;
			btnX.action = FALSE;
		}

		/*
		*	GENERAL FUNCTION COUNTER and timeout
		*/
		count++;
		if(!(count % 100)){
			toggle ^= 1;
			if(count >= 30000){
				*state = DISPLAY_MENU;
				count = 0;
			}
		}

		/* 
		* LOOP DELAY AND INTERRUPT ENABLE TIME --------------------------------
		* All interrupts are served within the sei()-cli() block. This is to 
		* avoid the extra care required for arbitrarily triggered ISRs and the 
		* use of atomic operations. "loop" flag is set every 1ms by a timer
		* whose ISR is enabled to produce interrupts every 1ms
		*/
		sei();
		// Wait for the next ms.
		while(!loop);
		loop = FALSE;
		cli();
		// If system state changed, exit fuction.
		if(*state != SET_TRANSITIONS)
			break;

	}	/* INFINITE LOOP */
}

/*-----------------------------------------------------------------------------
-------------------------- L O C A L   F U N C T I O N S ----------------------
-----------------------------------------------------------------------------*/

/*===========================================================================*/
static uint8_t change_transition_mode(uint8_t dir)
{
	uint8_t tmp;

	if(dir == UP){
		if(display.mode == DISP_MODE_1) tmp = DISP_MODE_2;
		else if(display.mode == DISP_MODE_2) tmp = DISP_MODE_3;
		else if(display.mode == DISP_MODE_3) tmp = DISP_MODE_4;
		else if(display.mode == DISP_MODE_4) tmp = DISP_MODE_1;
		else tmp = DISP_MODE_4;	
	} else {
		if(display.mode == DISP_MODE_1) tmp = DISP_MODE_4;
		else if(display.mode == DISP_MODE_2) tmp = DISP_MODE_1;
		else if(display.mode == DISP_MODE_3) tmp = DISP_MODE_2;
		else if(display.mode == DISP_MODE_4) tmp = DISP_MODE_3;
		else tmp = DISP_MODE_4;	
	}

	return tmp;
}
