/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file menu_time.c
 * @brief Time display and manipulation
 *
 * Utility functions to display the desired time info and messages
 *
 * @author Jose Logreira
 * @date 25.04.2018
 *
 */

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "menu_time.h"

#include <avr/pgmspace.h>
#include <stdint.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

// LEDs behavior is one of three forms:
// - Breathing: LEDs PWM is varied quadratically with a period of 4 seconds
// - Steady: LEDs PWM do not change.
// - Off: LEDs PWM is disabled
#define LEDS_BREATHE	0xBB
#define LEDS_STEADY		0xEE
#define LEDS_OFF		0xFF

#define LED_RED 	1
#define LED_GREEN 	2
#define LED_BLUE 	3

// Vectors used for the transitions in different display animations
static const uint8_t animation_3d_t1[] PROGMEM = {
	3,8,9,4,0,5,7,2,6,1,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};
static const uint8_t animation_3d_t2[] PROGMEM = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,1,6,2,7,5,0,4,9,8,3
};
static const uint8_t animation_3d[] PROGMEM = {
	3,8,9,4,0,5,7,2,6,1,6,2,7,5,0,4,9,8
};
static const uint8_t positions_3d[] PROGMEM = {
	4,9,7,0,3,5,8,6,1,2,1,6,8,5,3,0,7,9
};

// LEDs PWM values for breathing effect
// 250 values. This seqeuence resembles a quadratic function with a range from
// 0 to 254. Used during display_time() for LEDs effects.
static const uint8_t led_pwm[] PROGMEM = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,2,2,2,2,2,3,3,3,3,4,4,4,5,5,
	5,5,6,6,6,7,7,7,8,8,9,9,9,10,10,11,11,11,12,12,13,13,14,14,15,15,16,16,17,
	17,18,18,19,20,20,21,21,22,23,23,24,24,25,26,26,27,28,28,29,30,31,31,32,33,
	33,34,35,36,36,37,38,39,40,40,41,42,43,44,45,46,46,47,48,49,50,51,52,53,54,
	55,56,57,58,58,59,60,61,62,64,65,66,67,68,69,70,71,72,73,74,75,76,78,79,80,
	81,82,83,84,86,87,88,89,90,92,93,94,95,97,98,99,100,102,103,104,106,107,
	108,110,111,112,114,115,117,118,119,121,122,124,125,126,128,129,131,132,
	134,135,137,138,140,141,143,144,146,147,149,151,152,154,155,157,158,160,
	162,163,165,167,168,170,172,173,175,177,178,180,182,184,185,187,189,191,
	192,194,196,198,200,201,203,205,207,209,211,212,214,216,218,220,222,224,
	226,228,230,232,234,235,237,239,241,243,245,247,249,251,254
};


/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/
/*
* IMPORTANT!: All functions that receive and return a "state_t state" variable 
* are non-blocking, and their structure is rather different than other typical 
* functions: 
* - The actions performed by every fuction are tied (or synced) to the 
*   execution rate of the main loop (1ms), and based on this time, all counters
*   and variables modify the display behavior.
* - The functions are full of "static" type variables, since the contents of 
*   most of the variables must be preserved each time the functions exit and
*   re-enter. 
* - The blocks inside the functions are executed based on the value of counters.
*   According to the value of these counters, the function performs a certain
*   action.
*/

static void increment_time(uint8_t what);
static void change_hour_mode(uint8_t mode);
//static uint8_t led_pwm_value(uint8_t led, uint16_t v, uint8_t day_period);
static uint8_t led_pwm_value(uint8_t led, uint16_t v);

/*===========================================================================*/
/*
* MAIN DISPLAY STATE
* - It mainly displays the time
* - Coordinates animations and display effects
* - Coordinates LEDs colors and sequences
* - Perform certain actions according to the state of buttons
*/
state_t display_time(state_t state)
{
	static uint8_t intro = TRUE;
	// transitions-related variables
	static uint16_t count = 0;
	static uint16_t p = 0;		
	static uint16_t q = 0;
	static uint8_t step = 0;
	static uint8_t transition_triggered = FALSE;
	static uint8_t display_mode = DISP_MODE_8;
	static uint8_t temp[] = {0,0,0,0};		// temporal values
	// leds-related variables
	static uint8_t leds_cnt_up = TRUE;
	static uint16_t leds_count = 0;
	static uint8_t leds_mode = LEDS_BREATHE;
	
	// If system_state changes from other state to DISPLAY_TIME, execute this
	// block the first time this function is entered
	if(intro){
		intro = FALSE;
		step = 0;
		display.set = ON;
		timer_leds_set(ENABLE, 0, 0, 0);
		count = 0;
		p = 0;
		q = 0;
		display_mode = DISP_MODE_8;
		transition_triggered = FALSE;
		leds_count = 0;
		leds_cnt_up = 0;
		display.fade_level[0] = 5;
		display.fade_level[1] = 5;
		display.fade_level[2] = 5;
		display.fade_level[3] = 5;
	}

	/*
	* LEDs SEQUENCES
	* - breathing sequence: leds are synced to the RTC by means of the 
	*   time.update flag.
	* If display.set is ON, enable LEDs; else, disable them
	* If DISP_MODE_7 is selected, it means the clock is displaying the alarm
	* and LEDs show the alarm.day_period color (either green or blue)
	*/
	if(display.set == ON){
		if(display_mode != DISP_MODE_7){
			// LEDs breathing sequence has a period of 4 seconds: 2 seconds
			// increasing intensity and 2 seconds decreasing intensity. leds_count
			// may range from 0 to 1999, but the range is limited from 5 to 1995
			// to account for possible delays in other routines (other routines 
			// may take more than a millisecond to execute)
			if(leds_mode == LEDS_BREATHE){
				if(leds_cnt_up){
					if(leds_count < 1999) leds_count++;
					else leds_count = 1999;
				} else {
					if(leds_count > 0) leds_count--;
					else leds_count = 0;
				}
				
				// sync leds_count with the general counter (T = 1ms)
				if(time.update){
					time.update = FALSE;
					if((leds_cnt_up) && (leds_count > 1000) && (time.sec % 2)){
						leds_cnt_up = FALSE;
						leds_count = 1999;
					} else if((!leds_cnt_up) && (leds_count < 1000) && (time.sec % 2)){
						leds_cnt_up = TRUE;
						leds_count = 0;
					}
				}

				// LEDs update value every 5ms, not every ms (LEDs value does not change every ms)
				if(!(leds_count % 5)){
					uint8_t led_r = 0, led_g = 0, led_b = 0;
					led_r = led_pwm_value(LED_RED, leds_count);
					led_g = led_pwm_value(LED_GREEN, leds_count);
					led_b = led_pwm_value(LED_BLUE, leds_count);
					timer_leds_set(ENABLE, led_r, led_g, led_b);
				}
			} else if(leds_mode == LEDS_STEADY){
				if(time.day_period == PERIOD_AM) 
					timer_leds_set(ENABLE, 50, 30, 0);
				else if(time.day_period == PERIOD_PM)
					timer_leds_set(ENABLE, 20, 20, 65);
			} else if(leds_mode == LEDS_OFF){
				timer_leds_set(DISABLE, 0, 0, 0);
			}
		} else {
			if(alarm.day_period == PERIOD_AM) 
				timer_leds_set(ENABLE, 0, 150, 0);
			else if(alarm.day_period == PERIOD_PM) 
				timer_leds_set(ENABLE, 0, 0, 150);
		}	
	} else {
		timer_leds_set(DISABLE, 0, 0, 0);
	}
	
	/* 
	*	TRANSITION check: 
	*	- if time.min is multiple of 10, trigger 10 mins transition
	* 	- else if time.hour changes, trigger 1 hour transition
	*/
	if((time.sec == 0) && (!transition_triggered)){
		transition_triggered = TRUE;
		if(!(time.min % 10)){
			if(time.min != 0) display_mode = DISP_MODE_5;	// 10 mins transition				
			else display_mode = DISP_MODE_6;				// 1 hour transition				
		} else {				
			display_mode = display.mode;					// 1 minute transition (user selectable)
		}
		p = 0;
		q = 0;
	}

	/* 
	* DISPLAY MODE. Executes a sequence of animations according to the chosen
	* display mode.
	* User configurable DISP_MODE_1, 2, 3, 4
	* DISP_MODE_5, 6: contain the 10 mins and 1 hour animations, respectively
	* DISP_MODE_7: contains the "Show Alarm" animation (when button Y is pressed)
	* DISP_MODE_8, 9: intro animations for DISP_MODE_0, and DISPLAY_MENU.
	* 
	* Dísplay modes are structured as a sequence of steps, each one having its
	* own timing and function. Note that this approach is simpler than the one
	* taken with the Resin Clock code (because the NC3 does NOT have to toggle
	* the display between Hours and Minites, aka, it has 4 tubes), but a major
	* drawback is that repetitive code is often used due to repetitive steps.
	*/
	switch(display_mode){

		// --------------------------------------------------------------------
		case DISP_MODE_0:				
			
			if(time.sec != 0) transition_triggered = FALSE;
			display.d1 = time.h_tens;
			display.d2 = time.h_units;	
			display.d3 = time.m_tens;
			display.d4 = time.m_units;
			break;

		// --------------------------------------------------------------------
		// WATERFALL EFFECT: Random numbers start appearing in each tube, one 
		// by one, using a fade-in effect, and stopping at the current time
		case DISP_MODE_1:

			if(step == 0){
				count = 0;
				// back up current digits
				temp[0] = display.d1;
				temp[1] = display.d2;
				temp[2] = display.d3;
				temp[3] = display.d4;
				display.d1 = BLANK;
				display.d2 = BLANK;
				display.d3 = BLANK;
				display.d4 = BLANK;
				// start all 4 tubes' brightness (fade) level at 1
				display.fade_level[0] = 1;
				display.fade_level[1] = 1;
				display.fade_level[2] = 1;
				display.fade_level[3] = 1;
				step = 1;

			} else if(step == 1){

				// every 50ms, generate a new random number in tube 4
				if(!(count % 50)){
					display.d4 =  random_number(temp[3]);
					temp[3] = display.d4;
				}
				// every 160ms, increase the 4th tube brigthness
				if(!(count % 160) &&  (display.fade_level[TUBE_D] < 5))
					display.fade_level[TUBE_D]++;
				// after 800ms, update tuebe 4 with proper time value and max brightness
				if(count > 800){
					display.fade_level[TUBE_D] = 5;
					display.d4 =  time.m_units;
					count = 0;
					step = 2;
				}

			} else if(step == 2){

				// every 50ms, generate a new random number in tube 3
				if(!(count % 50)){
					display.d3 =  random_number(temp[2]);
					temp[2] = display.d3;
				}
				// every 160ms, increase the 3rd tube brigthness
				if(!(count % 160) &&  (display.fade_level[TUBE_C] < 5))
					display.fade_level[TUBE_C]++;
				// after 800ms, update tuebe 3 with proper time value and max brightness
				if(count > 800){
					display.fade_level[TUBE_C] = 5;
					display.d3 = time.m_tens;
					count = 0;
					step = 3;
				}

			} else if(step == 3){

				// every 50ms, generate a new random number in tube 2
				if(!(count % 50)){
					display.d2 =  random_number(temp[1]);
					temp[1] = display.d2;
				}
				// every 160ms, increase the 2nd tube brigthness
				if(!(count % 160) &&  (display.fade_level[TUBE_B] < 5))
					display.fade_level[TUBE_B]++;
				// after 800ms, update tuebe 2 with proper time value and max brightness
				if(count > 800){
					display.fade_level[TUBE_B] = 5;
					display.d2 = time.h_units;
					count = 0;
					step = 4;
				}

			} else if(step == 4){

				// every 50ms, generate a new random number in tube 1
				if(!(count % 50)){
					display.d1 =  random_number(temp[0]);
					temp[0] = display.d1;
				}
				// every 160ms, increase the 2nd tube brigthness
				if(!(count % 160) &&  (display.fade_level[TUBE_A] < 5))
					display.fade_level[TUBE_A]++;
				// after 800ms, update tuebe 2 with proper time value and max brightness
				if(count > 800){
					display.d1 = time.h_tens;
					count = 0;
					step = 0;
					display_mode = DISP_MODE_0;
				}
			}
			break;
		
		// --------------------------------------------------------------------
		// SLOT MACHINE EFFECT: All 4 tubes display random numbers, and one by
		// one they stop at the proper time digit
		case DISP_MODE_2:

			if(step == 0){

				count = 0;
				// start all 4 tubes' brightness (fade) level at 5
				display.fade_level[0] = 5;
				display.fade_level[1] = 5;
				display.fade_level[2] = 5;
				display.fade_level[3] = 5;
				step = 1;

			} else if(step == 1){

				// Every 50ms, update all 4 tubes with a random number
				if(!(count % 50)){
					display.d1 = random_number(display.d1);
					display.d2 = random_number(display.d2);
					display.d3 = random_number(display.d3);
					display.d4 = random_number(display.d4);
				}
				// after 800ms, fix the 4th tube with the proper digit
				if(count > 800){
					display.d4 = time.m_units;
					count = 0;
					step = 2;
				}

			} else if(step == 2){

				// Every 50ms, update 3 tubes with a random number
				if(!(count % 50)){
					display.d1 = random_number(display.d1);
					display.d2 = random_number(display.d2);
					display.d3 = random_number(display.d3);
				}
				// after 800ms, fix the 3rd tube with the proper digit
				if(count > 800){
					display.d3 = time.m_tens;
					count = 0;
					step = 3;
				}

			} else if(step == 3){

				// Every 50ms, update 2 tubes with a random number
				if(!(count % 50)){
					display.d1 = random_number(display.d1);
					display.d2 = random_number(display.d2);
				}
				// after 800ms, fix the 2nd tube with the proper digit
				if(count > 800){
					display.d2 = time.h_units;
					count = 0;
					step = 4;
				}

			} else if(step == 4){

				// Every 50ms, update the first tube with a random number
				if(!(count % 50))
					display.d1 = random_number(display.d1);
				// after 800ms, fix the 1st tube with the proper digit
				if(count > 800){
					display.d1 = time.h_tens;
					count = 0;
					step = 0;
					display_mode = DISP_MODE_0;
				}
			}
			break;
		
		// --------------------------------------------------------------------
		// WAVE EFFECT: Transition all tubes' filaments in the 3D order 
		// determined by positions_3D[], starting with the current digit that's
		// being displayed, and decreasing progressively the speed.

		case DISP_MODE_3:
			
			if(step == 0){

				// reset counter variables
				count = 0;
				p = 0;
				q = 0;
				// store the current digits, to use them as reference for the transition
				temp[0] = pgm_read_byte(&positions_3d[display.d1]);
				temp[1] = pgm_read_byte(&positions_3d[display.d2]);
				temp[2] = pgm_read_byte(&positions_3d[display.d3]);
				temp[3] = pgm_read_byte(&positions_3d[display.d4]);
				step = 1;

			} else if(step == 1){

				// for each tube 1 to 4, decide the next digit to be displayed
				// according to the order given in positions_3d[]
				if((temp[0] + p) >= sizeof(animation_3d)) 
					display.d1 = pgm_read_byte(&animation_3d[temp[0] + p - sizeof(animation_3d)]);
				else 
					display.d1 = pgm_read_byte(&animation_3d[temp[0] + p]);
	    		if((temp[1] + p) >= sizeof(animation_3d)) 
	    			display.d2 = pgm_read_byte(&animation_3d[temp[1] + p - sizeof(animation_3d)]);
				else 
					display.d2 = pgm_read_byte(&animation_3d[temp[1] + p]);
				if((temp[2] + p) >= sizeof(animation_3d)) 
					display.d3 = pgm_read_byte(&animation_3d[temp[2] + p - sizeof(animation_3d)]);
				else 
					display.d3 = pgm_read_byte(&animation_3d[temp[2] + p]);
				if((temp[3] + p) >= sizeof(animation_3d)) 
					display.d4 = pgm_read_byte(&animation_3d[temp[3] + p - sizeof(animation_3d)]);
				else 
					display.d4 = pgm_read_byte(&animation_3d[temp[3] + p]);
	    		// manage counters that vary the update rate and wave speed
	    		if(count >= (30 + (20 * q))){
					count = 0;
					p++;
					if(p >= sizeof(animation_3d)){
						p = 0;
						q++;
						if(q == 4){
							step = 0;
							display_mode = DISP_MODE_0;
						}
					}
				}		
			}
			break;
		
		// --------------------------------------------------------------------
		// ALL THE ABOVE: every minute, it performs a different animation,
		// among the previous three ones.
		case DISP_MODE_4:
			
			if((time.min == 1) || (time.min == 4) || (time.min == 7))
				display_mode = DISP_MODE_1;
			else if((time.min == 2) || (time.min == 5) || (time.min == 8))
				display_mode = DISP_MODE_2;
			else
				display_mode = DISP_MODE_3;
			break;

		// --------------------------------------------------------------------
		// 10 MINUTES EFFECT: weird variable speed effect showing random
		// numbers, with inverse speeds in adjacent tubes
		case DISP_MODE_5:

			if(step == 0){

				p = 1;
				count = 0;
				// Start all tubes' brightness levels at 5
				display.fade_level[0] = 5;
				display.fade_level[1] = 5;
				display.fade_level[2] = 5;
				display.fade_level[3] = 5;
				step = 1;

			} else if((step == 1) || (step == 2)){

				// counters manage the tube's update speed 
				if(!(count % (100-p))){
					if(step == 1){
						display.d1 = random_number(display.d1);	
						display.d3 = random_number(display.d3);
					} else {
						display.d2 = random_number(display.d2);
						display.d4 = random_number(display.d4);
					}
				} 
				if(!(count % p)){
					if(step == 1){
						display.d2 = random_number(display.d2);	
						display.d4 = random_number(display.d4);
					} else {
						display.d1 = random_number(display.d1);
						display.d3 = random_number(display.d3);
					}
				} 
				// update counters' values
				if(!(count % 50)){
					p++;

					if(p == 99){
						p = 1;
						count = 0;
						if(step == 1){
							step = 2;	
						} else {
							step = 0;
							display_mode = DISP_MODE_0;
						}
					}
				}
			}
			break;

		// --------------------------------------------------------------------
		// 1 HOUR EFFECT: Tubes show the same digit, and the 3D sequence is 
		// performed several times with variable speed: from low to high at
		// first, and then from high to low speed
		case DISP_MODE_6:

			if(step == 0){

				// start all tubes' brightness levels at 5
				display.fade_level[0] = 5;
				display.fade_level[1] = 5;
				display.fade_level[2] = 5;
				display.fade_level[3] = 5;
				// reset counter variables
				count = 0;
				p = 150;
				q = 0;
				step = 1;

			} else if(step == 1){

				// Using counters, increase update rate progressively
				if(count >= p){
					q++;
					if(q >= sizeof(animation_3d)) q = 0;
					display.d1 = pgm_read_byte(&animation_3d[q]);
					display.d2 = display.d1;
					display.d3 = display.d1;
					display.d4 = display.d1;
					count = 0;
					p -= 3;
					if(p <= 15){
						p = 15;
						step = 2;
					}
				}

			} else if(step == 2){

				// Keep update rate constant for 2 seconds
				if(!(count % 15)){
					q++;
					if(q >= sizeof(animation_3d)) q = 0;
					display.d1 = pgm_read_byte(&animation_3d[q]);
					display.d2 = display.d1;
					display.d3 = display.d1;
					display.d4 = display.d1;
					if(count >=  2000) step = 3;
				}

			} else if(step == 3){

				// Using counters, decrease update rate progressively
				if(count >= p){
					q++;
					if(q >= sizeof(animation_3d)) q = 0;
					display.d1 = pgm_read_byte(&animation_3d[q]);
					display.d2 = display.d1;
					display.d3 = display.d1;
					display.d4 = display.d1;
					count = 0;
					p += 3;
					if(p >= 150){
						p = 0;
						step = 0;
						display_mode = DISP_MODE_0;
					}
				}
			}
			break;

		// --------------------------------------------------------------------
		// DISPLAY ALARM: for 3 seconds. Then, return to DISP_MODE_0
		case DISP_MODE_7:

			if(step == 0){
				count = 0;
				display.d1 = alarm.h_tens;
				display.d2 = alarm.h_units;
				display.d3 = alarm.m_tens;
				display.d4 = alarm.m_units;
				step = 1;
			} else if(step == 1){
				if(count > 3000){
					count = 0;
					step = 0;
					display_mode = DISP_MODE_0;
				}
			}
			break;

		// --------------------------------------------------------------------
		// Fade transition: Intro Mode to DISPLAY_MODE_0
		case DISP_MODE_8:

			if(!(count % 20)){
				display.fade_level[step]--;
				if(display.fade_level[step] == 0){
					step++;
					if(step >= 4){
						step = 0;
						count = 0;
						buzzer_beep();
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
			}
			break;

		// --------------------------------------------------------------------
		// Fade transition: intro mode to state DISPLAY_MENU. It's implemented
		// here instead of inside the display_menu() function, since this
		// animation only happens when transitioning from display_time() to
		// display_menu();
		case DISP_MODE_9:

			if(!(count % 20)){
				display.fade_level[step]--;
				if(display.fade_level[step] == 0){
					step++;
					if(step >= 4){
						step = 0;
						count = 0;
						buzzer_beep();
						display.d1 = BLANK;
						display.d2 = BLANK;
						display.d3 = BLANK;
						display.d4 = BLANK;
						for(uint8_t i = 0; i < 4; i++)
							display.fade_level[i] = 5;
						state = DISPLAY_MENU;
						intro = TRUE;					
					}
				}
			}
			break;

		// --------------------------------------------------------------------
		default:
			break;
	}
	
	/* 
	*	BUTTONS actions
	*	- Button actions are executed according to the buttons state flags
	*   - If display.set is OFF, the behavionr of the buttons changes.
	*/
	// If button X pushed for 2 seconds, go to DISPLAY_MENU
	if((btnX.action) && (btnX.delay3) && (!btnY.action) && (!btnZ.action)){
		btnX.action = FALSE;
		if(display.set){
			if(display_mode == DISP_MODE_0){
				display_mode = DISP_MODE_9;
			}
		} else {
			display.set = ON;
		}
	}
	// if X pushed, just go to intro mode
	if((btnX.action) && (btnX.state == BTN_RELEASED) && (!btnX.delay1)){
		btnX.action = FALSE;
		if(display.set) {
			if(display_mode == DISP_MODE_0)
				state = SYSTEM_INTRO;
		} else {
			display.set = ON;
		}
	}
	// if Z pushed, change LEDs behavior
	if((btnZ.action) && (btnZ.state == BTN_RELEASED) && (!btnZ.delay1)){
		btnZ.action = FALSE;
		if(display.set){
			if(leds_mode == LEDS_BREATHE) {
				leds_mode = LEDS_STEADY;
			} else if(leds_mode == LEDS_STEADY) {
				leds_mode = LEDS_OFF;
			} else if(leds_mode == LEDS_OFF) {
				leds_mode = LEDS_BREATHE;
				leds_count = 0;
				leds_cnt_up = TRUE;
			}
		} else {
			display.set = ON;
		}
	}
	// if Y pushed, show alarm
	if((btnY.action) && (btnY.state == BTN_RELEASED) && (!btnY.delay1)){
		btnY.action = FALSE;
		if(display.set) {
			if(display_mode == DISP_MODE_0)
				display_mode = DISP_MODE_7;
		} else {
			display.set = ON;
		}
	}
	// if Y pushed for 2 seconds, display is off
	if((btnY.action) && (btnY.delay3) && (!btnX.action) && (!btnZ.action)){
		btnY.action = FALSE;
		display.set = OFF;
		buzzer_beep();
	}
	// IF ALL THREE BUTTONS PRESSED DURING DELAY3, RESET SYSTEM AND GO TO SLEEP
	if((btnX.action) && (btnX.delay3) && (btnY.action) && (btnY.delay3) && (btnZ.action) && (btnZ.delay3)){
		btnX.action = FALSE;
		btnY.action = FALSE;
		btnZ.action = FALSE;
		display.set = OFF;
		system_reset = TRUE;
		state = SYSTEM_RESET;
		intro = TRUE;
	}
	/* 
	* 	GENERAL FUNCTION COUNTER
	*/
	count++;

	return state;
}

/*===========================================================================*/
/*
* TIME SETTING STATE
* - User performs hours and minutes adjustments using Z button
* - Toggles selection between hours and minutes using Y button
* - Fixed LEDs color.
*/
state_t set_time(state_t state)
{
	static uint8_t intro = TRUE;
	static uint16_t count = 0;
	static uint8_t toggle = 0;
	static uint8_t selection = 1;
	static uint8_t display_mode = DISP_MODE_0;

	if(intro){
		intro = FALSE;
		display.set = ON;
		display_mode = DISP_MODE_0;
		count = 0;
		selection = 1;
		if(time.day_period == PERIOD_AM) timer_leds_set(ENABLE, 50, 30, 0);
		else if(time.day_period == PERIOD_PM) timer_leds_set(ENABLE, 20, 20, 65);
	}

	/*
	*	DISPLAY TRANSITIONS: toggle
	*	The animation simply consist of blinking the digits as if there were
	*	a cursor, to indicate which quantity can be changed. If the button
	* 	is kept pushed, blinking stops and fast increment occurs
	*/
	switch(display_mode){

		case DISP_MODE_0:
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
					display_mode = DISP_MODE_1;
				}
			}
			break;

		case DISP_MODE_1:
			if((toggle) || (btnZ.state == BTN_PUSHED)){
				display.d1 = time.h_tens;
				display.d2 = time.h_units;
				display.d3 = time.m_tens;
				display.d4 = time.m_units;
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

		case DISP_MODE_2:
			if((toggle) || (btnZ.state == BTN_PUSHED)){
				display.d1 = time.m_tens;
				display.d2 = time.m_units;
				display.d3 = time.s_tens;
				display.d4 = time.s_units;
			} else {
				if(selection) {
					display.d1 = BLANK;
					display.d2 = BLANK;
				} else {
					display.d3 = BLANK;
					display.d4 = BLANK;
				}
			}
			break;
	}	

	/*
	*	BUTTONS ACTIONS
	*/
	// If X is pressed, return to the menu
	if((btnX.action) && (btnX.state == BTN_RELEASED) && (!btnX.delay1)){
		btnX.action = FALSE;
		state = DISPLAY_MENU;
		intro = TRUE;
	}
	// If X is pressed for delay3 ms, return to display the time
	if((btnX.action) && (btnX.delay3)){
		state = DISPLAY_TIME;
		btnX.action = FALSE;
		intro = TRUE;
	}
	// If Y is pressed, toggle hours/minutes selection
	if((btnY.action) && (btnY.state == BTN_RELEASED) && (!btnY.delay1)){
		btnY.action = FALSE;
		selection ^= 1;
		count = 0;
	}
	// If Y button pressed for delay3 ms, show minutes and seconds, not hours.
	// This would be a "hidden" feature, used for calibration purposes only
	if((btnY.action) && (btnY.delay3)){
		btnY.action = FALSE;
		count = 0;
		selection ^= 1;
		if(display_mode == DISP_MODE_1)
			display_mode = DISP_MODE_2;
		else
			display_mode = DISP_MODE_1;
	}
	// If Z is pressed, increment the selected quantity
	if(btnZ.action){
		if(display_mode == DISP_MODE_1){
			if(btnZ.state == BTN_RELEASED){
				if(selection) increment_time(INC_HOUR);
				else increment_time(INC_MIN);
				update_time_variables();
				btnZ.action = FALSE;
			} else if((btnZ.delay1) && (btnZ.delay2)){
				btnZ.delay2 = FALSE;
				if(selection) increment_time(INC_HOUR);
				else increment_time(INC_MIN);
				update_time_variables();
			}
		} else if(display_mode == DISP_MODE_2){
			if(btnZ.state == BTN_RELEASED){
				if(selection) increment_time(INC_MIN);
				else increment_time(INC_SEC);
				update_time_variables();
				btnZ.action = FALSE;
			} else if((btnZ.delay1) && (btnZ.delay2)){
				btnZ.delay2 = FALSE;
				if(selection) increment_time(INC_MIN);
				else increment_time(INC_SEC);
				update_time_variables();
			}
		}
		count = 0;
	}

	/*
	* 	GENERAL FUNCTION COUNTER and timeout
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
* HOUR MODE SETTING STATE
* - User chooses between 12h or 24h mode
* - Fixed LEDs color.
*/
state_t set_hour_mode(state_t state)
{
	static uint8_t intro = TRUE;
	static uint16_t count = 0;
	static uint8_t toggle = 0;

	if(intro){
		intro = FALSE;
		count = 0;
		display.d1 = BLANK;
		display.d2 = BLANK;
		timer_leds_set(ENABLE, 10, 10, 100);
	}

	/*
	*	DISPLAY message
	* 	The animation simply consist of blinking the hour mode as if there were
	*	a cursor, to indicate which quantity can be changed. 
	*/
	if(toggle){
		display.set = ON;
		if(time.hour_mode == MODE_12H){
			display.d3 = 1;
			display.d4 = 2;
		} else if(time.hour_mode == MODE_24H){
			display.d3 = 2;
			display.d4 = 4;
		}
	} else {
		display.set = OFF;
	}

	/*
	* 	BUTTONS
	*/
	// If either Y or Z are pressed, change the hour mode
	if((btnY.action) || (btnZ.action)){
		btnY.action = FALSE;
		btnZ.action = FALSE;
		if(time.hour_mode == MODE_12H) change_hour_mode(MODE_24H);
		else if(time.hour_mode == MODE_24H) change_hour_mode(MODE_12H);
		update_time_variables();
		count = 0;
	}
	// If X pressed shortly, return to the menu
	if((btnX.action) && (btnX.state == BTN_RELEASED) && (!btnX.delay1)){
		btnX.action = FALSE;
		state = DISPLAY_MENU;
		intro = TRUE;
	}
	// If X pressed and hold for delay3 ms, return to display the time
	if((btnX.action) && (btnX.delay3)){
		state = DISPLAY_TIME;
		btnX.action = FALSE;
		intro = TRUE;
	}

	/*
	*	GENERAL FUNCTION COUNTER and timeout
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


/*-----------------------------------------------------------------------------
-------------------------- L O C A L   F U N C T I O N S ----------------------
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*
* Increment the stated quantity, taking into account the boundary values:
* 	Hours: 12h or 24h mode? AM or PM?
*   Minutes: minute 59?
*/
static void increment_time(uint8_t what)
{
	if(what == INC_HOUR){
		if(time.hour_mode == MODE_12H){
			if(time.hour == 11){
				time.hour++;
				if(time.day_period == PERIOD_AM) time.day_period = PERIOD_PM;
				else time.day_period = PERIOD_AM;
			} else if(time.hour == 12){
				time.hour = 1;
			} else {
				time.hour++;
			}
		} else if(time.hour_mode == MODE_24H){
			if(time.hour == 11){
				time.hour++;
				time.day_period = PERIOD_PM;
			} else if(time.hour == 23){
				time.hour = 0;
				time.day_period = PERIOD_AM;
			} else {
				time.hour++;
			}
		}
	} else if(what == INC_MIN){
		if(time.min == 59) time.min = 0;
		else time.min++;
	} else if(what == INC_SEC){
		if(time.sec == 59) time.sec = 0;
		else time.sec++;
	}

	// LEDs update
	if(time.day_period == PERIOD_AM) timer_leds_set(ENABLE, 50, 30, 0);
	else if(time.day_period == PERIOD_PM) timer_leds_set(ENABLE, 20, 20, 65);
}

/*===========================================================================*/
/*
* Changes Hour Mode
* In changing the hour mode, the time hour value and alarm hour value must also
* be evaluated and updated.
*/
static void change_hour_mode(uint8_t mode)
{
	// change Time and Alarm hour mode accordingly
	if(mode == MODE_12H){
		time.hour_mode = MODE_12H;
		alarm.hour_mode = MODE_12H;
		// time correction
		if(time.hour == 0) time.hour = 12;
		else if(time.hour >=13) time.hour -= 12; 
		// alarm correction
		if(alarm.hour == 0) alarm.hour = 12;
		else if(alarm.hour >=13) alarm.hour -= 12; 
	} else if(mode == MODE_24H){
		time.hour_mode = MODE_24H;
		alarm.hour_mode = MODE_24H;
		// time correction
		if((time.day_period == PERIOD_AM) && (time.hour == 12))	time.hour = 0;	
		else if((time.day_period == PERIOD_PM) && (time.hour != 12)) time.hour += 12;
		// alarm correction
		if((alarm.day_period == PERIOD_AM) && (alarm.hour == 12))	alarm.hour = 0;	
		else if((alarm.day_period == PERIOD_PM) && (alarm.hour != 12)) alarm.hour += 12;
	}
}

/*===========================================================================*/
/*
* Used for breathing effect. Returns a PWM value from a vector stored in FLASH
* That vector is computed as a quadratic function. The quadratic function was 
* chosen so that v = 0 gives a value of PWM = 1, and v = 1999 gives a value of 
* PWM = 250 (provided a scale factor of 1). Different scale factors are 
* provided for different color intensities.
* This function was previously implemented using floating-point operations, but
* it was very time-consuming, so this strategy (query a stored value) is much
* faster.
*/
static uint8_t led_pwm_value(uint8_t led, uint16_t v)
{
	uint8_t x = (uint8_t)(v / 8);
	uint8_t pwm = pgm_read_byte(&led_pwm[x]);
	float scale;
	uint8_t out;

	if (led == LED_RED) {
		if (time.day_period == PERIOD_AM) scale = 0.2;
		else scale = 0.08;
	} else if (led == LED_GREEN) {
		if (time.day_period == PERIOD_AM) scale = 0.12;
		else scale = 0.08;
	} else if (led == LED_BLUE) {
		if (time.day_period == PERIOD_AM) scale = 0.0;
		else scale = 0.26;
	}

	out = (uint8_t)(scale * (float)pwm);
	out += 1;
	
	return out;
}