/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file buzzer.c
 * @brief Buzzer-related functions and tones
 *
 * Utility functions to implement the buzzer tones
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "buzzer.h"
#include "config.h"
#include "timers.h"
#include "uart.h"

#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdint.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

/*
* Melodies are stored as an array of note-duration structures. "note" field 
* determines the musical note (the PWM frequency of the buzzer), and "duration"
* determines for how long the note is to be played.
*/
typedef struct {
	uint16_t note;
	uint16_t duration;
} note_s;

static const note_s star_wars_theme[] PROGMEM = {
	{N_SIL, QUARTER_NOTE},
	{N_C7, TWELVETH_NOTE},
	{N_C7, TWELVETH_NOTE},
	{N_C7, TWELVETH_NOTE},
	{N_F7, HALF_NOTE},
	{N_C8, HALF_NOTE},
	{N_B7, TWELVETH_NOTE},
	{N_A7, TWELVETH_NOTE},
	{N_G7, TWELVETH_NOTE},
	{N_F8, HALF_NOTE},
	{N_C8, QUARTER_NOTE},
	{N_B7, TWELVETH_NOTE},
	{N_A7, TWELVETH_NOTE},
	{N_G7, TWELVETH_NOTE},
	{N_F8, HALF_NOTE},
	{N_C8, QUARTER_NOTE},
	{N_B7, TWELVETH_NOTE},
	{N_A7, TWELVETH_NOTE},
	{N_B7, TWELVETH_NOTE},
	{N_G7, HALF_NOTE},
};

static const note_s imperial_march_theme[] PROGMEM = {
	{N_SIL, QUARTER_NOTE},
	{N_F7, QUARTER_NOTE},
	{N_F7, QUARTER_NOTE},
	{N_F7, QUARTER_NOTE},
	{N_Db7, EIGHTH_NOTE + SIXTEENTH_NOTE},
	{N_A7, SIXTEENTH_NOTE},
	{N_F7, QUARTER_NOTE},
	{N_Db7, EIGHTH_NOTE + SIXTEENTH_NOTE},
	{N_A7, SIXTEENTH_NOTE},
	{N_F7, HALF_NOTE},
	{N_C8, QUARTER_NOTE},
	{N_C8, QUARTER_NOTE},
	{N_C8, QUARTER_NOTE},
	{N_Db8, EIGHTH_NOTE + SIXTEENTH_NOTE},
	{N_A7, SIXTEENTH_NOTE},
	{N_E7, QUARTER_NOTE},
	{N_C7, EIGHTH_NOTE + SIXTEENTH_NOTE},
	{N_A7, SIXTEENTH_NOTE},
	{N_F7, HALF_NOTE},
};

static const note_s major_scale[] PROGMEM = {
	{N_SIL, SIXTEENTH_NOTE},
	{N_A6, SIXTEENTH_NOTE},
	{N_B6, SIXTEENTH_NOTE},
	{N_C7, SIXTEENTH_NOTE},
	{N_D7, SIXTEENTH_NOTE},
	{N_E7, SIXTEENTH_NOTE},
	{N_F7, SIXTEENTH_NOTE},
	{N_G7, SIXTEENTH_NOTE},
	{N_A7, SIXTEENTH_NOTE},
	{N_B7, SIXTEENTH_NOTE},
	{N_C8, SIXTEENTH_NOTE},
	{N_D8, SIXTEENTH_NOTE},
	{N_E8, SIXTEENTH_NOTE},
	{N_F8, SIXTEENTH_NOTE},
	{N_G8, SIXTEENTH_NOTE},
};

static const note_s super_mario_theme[] PROGMEM = {
	{N_SIL, QUARTER_NOTE},
	// compass 1
	{N_E8, EIGHTH_NOTE},
	{N_E8, EIGHTH_NOTE},
	{N_SIL, EIGHTH_NOTE},
	{N_E8, EIGHTH_NOTE},
	{N_SIL, EIGHTH_NOTE},
	{N_C8, EIGHTH_NOTE},
	{N_E8, QUARTER_NOTE},
	// compass 2
	{N_G8, QUARTER_NOTE},
	{N_SIL, QUARTER_NOTE},
	{N_G7, QUARTER_NOTE},
	{N_SIL, QUARTER_NOTE},
	// compass 3
	{N_C8, QUARTER_NOTE + EIGHTH_NOTE},
	{N_G7, EIGHTH_NOTE},
	{N_SIL, QUARTER_NOTE},
	{N_E7, QUARTER_NOTE + EIGHTH_NOTE},
	{N_A7, QUARTER_NOTE},
	{N_B7, QUARTER_NOTE},
	{N_Bb7, EIGHTH_NOTE},
	{N_A7, QUARTER_NOTE},
	// compass 5
	{N_G7, SIXTH_NOTE},
	{N_E8, SIXTH_NOTE},
	{N_G8, SIXTH_NOTE},
	{N_A8, QUARTER_NOTE},
	{N_F8, EIGHTH_NOTE},
	{N_G8, EIGHTH_NOTE},
	// compass 6
	{N_SIL, EIGHTH_NOTE},
	{N_E8, QUARTER_NOTE},
	{N_C8, EIGHTH_NOTE},
	{N_D8, EIGHTH_NOTE},
	{N_B7, QUARTER_NOTE},
	{N_SIL, EIGHTH_NOTE},
	// compass 7
	{N_SIL, QUARTER_NOTE},
	{N_G8, EIGHTH_NOTE},
	{N_Gb8, EIGHTH_NOTE},
	{N_F8, EIGHTH_NOTE},
	{N_Ds8, QUARTER_NOTE},
	{N_E8, EIGHTH_NOTE},
	// compass 8
	{N_SIL, EIGHTH_NOTE},
	{N_Gs7, EIGHTH_NOTE},
	{N_A7, EIGHTH_NOTE},
	{N_C8, EIGHTH_NOTE},
	{N_SIL, EIGHTH_NOTE},
	{N_A7, EIGHTH_NOTE},
	{N_C8, EIGHTH_NOTE},
	{N_D8, EIGHTH_NOTE},
	// compass 9
	{N_SIL, QUARTER_NOTE},
	{N_G8, EIGHTH_NOTE},
	{N_Gb8, EIGHTH_NOTE},
	{N_F8, EIGHTH_NOTE},
	{N_Ds8, QUARTER_NOTE},
	{N_E8, EIGHTH_NOTE},
	// compass 10
	{N_SIL, EIGHTH_NOTE},
	{N_C9, EIGHTH_NOTE},
	{N_SIL, EIGHTH_NOTE},
	{N_C9, EIGHTH_NOTE},
	{N_C9, HALF_NOTE},
	// compass 11
	{N_SIL, QUARTER_NOTE},
	{N_G8, EIGHTH_NOTE},
	{N_Gb8, EIGHTH_NOTE},
	{N_F8, EIGHTH_NOTE},
	{N_Ds8, QUARTER_NOTE},
	{N_E8, EIGHTH_NOTE},
	// compass 12
	{N_SIL, EIGHTH_NOTE},
	{N_Gs7, EIGHTH_NOTE},
	{N_A7, EIGHTH_NOTE},
	{N_C8, EIGHTH_NOTE},
	{N_SIL, EIGHTH_NOTE},
	{N_A7, EIGHTH_NOTE},
	{N_C8, EIGHTH_NOTE},
	{N_D8, EIGHTH_NOTE},
	// compass 13
	{N_SIL, QUARTER_NOTE},
	{N_Ds8, QUARTER_NOTE},
	{N_SIL, EIGHTH_NOTE},
	{N_D8, QUARTER_NOTE},
	{N_SIL, EIGHTH_NOTE},
	// compass
	{N_C8, HALF_NOTE},
};

static const note_s simple_alarm[] PROGMEM = {
	{N_SIL, QUARTER_NOTE},
	{N_E8, QUARTER_NOTE},
	{N_C8, QUARTER_NOTE},
	{N_E8, QUARTER_NOTE},
	{N_C8, QUARTER_NOTE},
	{N_E8, QUARTER_NOTE},
	{N_C8, HALF_NOTE},
};

static const note_s diomedes[] PROGMEM = {
	{N_SIL, QUARTER_NOTE},
	// compass
	{N_B8, HALF_NOTE},
	{N_SIL, QUARTER_NOTE},
	{N_A8, EIGHTH_NOTE},
	{N_A8, QUARTER_NOTE},
	{N_G8, EIGHTH_NOTE},
	{N_G8, EIGHTH_NOTE},
	{N_Fs8, EIGHTH_NOTE},
	{N_G8, HALF_NOTE + EIGHTH_NOTE},
	{N_E8, QUARTER_NOTE},
	{N_Fs8, EIGHTH_NOTE},
	{N_G8, QUARTER_NOTE},
	{N_A8, EIGHTH_NOTE},
	{N_A8, QUARTER_NOTE},
	{N_Fs8, HALF_NOTE},
	{N_SIL, QUARTER_NOTE + EIGHTH_NOTE},
	
	{N_G8, HALF_NOTE},
	{N_SIL, QUARTER_NOTE},
	{N_Fs8, EIGHTH_NOTE},
	{N_Fs8, QUARTER_NOTE},
	{N_E8, EIGHTH_NOTE},
	{N_E8, EIGHTH_NOTE},
	{N_D8, EIGHTH_NOTE},
	{N_E8, HALF_NOTE + EIGHTH_NOTE},
	{N_Cs8, QUARTER_NOTE},
	{N_D8, EIGHTH_NOTE},
	{N_E8, QUARTER_NOTE},
	{N_Fs8, EIGHTH_NOTE},
	{N_Fs8, QUARTER_NOTE + EIGHTH_NOTE},
	{N_D8, HALF_NOTE + QUARTER_NOTE},
	
	{N_SIL, HALF_NOTE + QUARTER_NOTE},
	{N_A7, EIGHTH_NOTE},
	{N_A8, QUARTER_NOTE},
	{N_A8, QUARTER_NOTE},
	{N_A8, EIGHTH_NOTE},
	{N_A8, QUARTER_NOTE},
	{N_G8, EIGHTH_NOTE},
	{N_G8, QUARTER_NOTE},
	{N_Fs8, QUARTER_NOTE},
	{N_G8, EIGHTH_NOTE},
	{N_G8, QUARTER_NOTE},
	{N_Fs8, HALF_NOTE},
	{N_A7, QUARTER_NOTE},
	{N_B7, QUARTER_NOTE},
	{N_Cs8, EIGHTH_NOTE},
	{N_B7, QUARTER_NOTE},

	{N_Cs8, QUARTER_NOTE + EIGHTH_NOTE},
	{N_SIL, QUARTER_NOTE},
	{N_A7, EIGHTH_NOTE},
	{N_E8, QUARTER_NOTE},
	{N_E8, QUARTER_NOTE},
	{N_E8, EIGHTH_NOTE},
	{N_E8, QUARTER_NOTE},
	{N_D8, EIGHTH_NOTE},
	{N_D8, QUARTER_NOTE},
	{N_Cs8, QUARTER_NOTE},
	{N_B8, EIGHTH_NOTE},
	{N_D8, QUARTER_NOTE},
	{N_Cs8, HALF_NOTE},
	{N_G7, QUARTER_NOTE},
	{N_A7, QUARTER_NOTE},
	{N_B7, EIGHTH_NOTE},
	{N_A7, QUARTER_NOTE},
	{N_F7, QUARTER_NOTE + EIGHTH_NOTE},
};

static const note_s usa_anthem[] PROGMEM = {
	{N_SIL, QUARTER_NOTE},
	// compass
	{N_D8, EIGHTH_NOTE + SIXTEENTH_NOTE},
	{N_B7, SIXTEENTH_NOTE},
	{N_G7, QUARTER_NOTE},
	{N_B7, QUARTER_NOTE},
	{N_D8, QUARTER_NOTE},
	{N_G8, HALF_NOTE},
	{N_B8, EIGHTH_NOTE + SIXTEENTH_NOTE},
	{N_A8, SIXTEENTH_NOTE},
	{N_G8, QUARTER_NOTE},
	{N_B7, QUARTER_NOTE},
	{N_Cs8, QUARTER_NOTE},
	{N_D8, HALF_NOTE},
	{N_D8, EIGHTH_NOTE},
	{N_D8, EIGHTH_NOTE},
	{N_B8, QUARTER_NOTE + EIGHTH_NOTE},
	{N_A8, EIGHTH_NOTE},
	{N_G8, QUARTER_NOTE},
	{N_Fs8, HALF_NOTE},
	{N_E8, EIGHTH_NOTE},
	{N_Fs8, EIGHTH_NOTE},
	{N_G8, QUARTER_NOTE},
	{N_G8, QUARTER_NOTE},
	{N_D8, QUARTER_NOTE},
	{N_B7, QUARTER_NOTE},
	{N_G7, QUARTER_NOTE},
	{N_D8, EIGHTH_NOTE + SIXTEENTH_NOTE},
	{N_B7, SIXTEENTH_NOTE},
	{N_G7, QUARTER_NOTE},
	{N_B7, QUARTER_NOTE},
	{N_D8, QUARTER_NOTE},
	{N_G8, HALF_NOTE},
	{N_B8, EIGHTH_NOTE + SIXTEENTH_NOTE},
	{N_A8, SIXTEENTH_NOTE},
	{N_G8, QUARTER_NOTE},
	{N_B7, QUARTER_NOTE},
	{N_Cs8, QUARTER_NOTE},
	{N_D8, HALF_NOTE},
	{N_D8, EIGHTH_NOTE},
	{N_D8, EIGHTH_NOTE},
	{N_B8, QUARTER_NOTE + EIGHTH_NOTE},
	{N_A8, EIGHTH_NOTE},
	{N_G8, QUARTER_NOTE},
	{N_Fs8, HALF_NOTE},
	{N_E8, EIGHTH_NOTE},
	{N_Fs8, EIGHTH_NOTE},
	{N_G8, QUARTER_NOTE},
	{N_G8, QUARTER_NOTE},
	{N_D8, QUARTER_NOTE},
	{N_B7, QUARTER_NOTE},
	{N_G7, QUARTER_NOTE},	
};
/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

// prototype functions with local scope
static uint16_t note_duration(uint16_t counts, uint8_t tempo);

/*
* Buzzer beeps for 30ms. This is a blocking function!: the function does not
* return until the delay elapses
*/
void buzzer_beep(void){

	buzzer_set(ENABLE, N_C8);
	_delay_ms(30);
	buzzer_set(DISABLE, N_C8);
}

/*
* BUZZER MUSIC
* This function is non-blocking, meaning that the melodies' notes aren't played
* sequentially, but rather the notes' timing is controlled similarly as it's
* done with the DISPLAY_TIME transitions (animations): 
* - A counter keeps track of the elapsed time
* - the counter is compared to the duration of each note
* - if the note duration is reached, the PWM frequency is modified for the next
*   note, and the counter is reset.
* - This fuction is executed once per millisecond, thus, 1ms is the time base
*/
uint8_t buzzer_music(uint8_t theme, uint8_t state){


	static uint8_t intro = TRUE;
	static uint16_t count = 0;
	static uint8_t n = 0;
	static uint8_t out = FALSE;
	static uint16_t count_vect[100];
	static uint8_t size;
	static const note_s *p; 
	
	/* 
	* Different melodies have different tempo. Thus, the time duration of each
	* melody's notes must be calculated based on a reference tempo (100). 
	* This calculation is performed only once when initializing this function.
	*/
	if(state){
		if(intro){
			intro = FALSE;
			count = 0;
			n = 0;
			out = FALSE;
			uint8_t tempo = 0;
			// point to the proper melody and calculate its size
			switch(theme){
				case MAJOR_SCALE: 	 
					p = major_scale;
					tempo = 200;
					size = sizeof(major_scale)/sizeof(note_s);
					break;
				case STAR_WARS:
					p = star_wars_theme;
					tempo = 108;
					size = sizeof(star_wars_theme)/sizeof(note_s);	
					break;
				case IMPERIAL_MARCH:
					p = imperial_march_theme;
					tempo = 108;
					size = sizeof(imperial_march_theme)/sizeof(note_s);
					break;
				case SUPER_MARIO:
					p = super_mario_theme;
					tempo = 200;
					size = sizeof(super_mario_theme)/sizeof(note_s);
					break;
				case SIMPLE_ALARM:
					p = simple_alarm;
					tempo = 200;
					size = sizeof(simple_alarm)/sizeof(note_s);
					break;
				case DIOMEDES:
					p = diomedes;
					tempo = 150;
					size = sizeof(diomedes)/sizeof(note_s);
					break;
				case USA_ANTHEM:
					p = usa_anthem;
					tempo = 90;
					size = sizeof(usa_anthem)/sizeof(note_s);
				default: break;
			}
			// calculate proper notes' duration and store the result in vector
			for(uint8_t i = 0; i < size; i++)
				count_vect[i] = note_duration(pgm_read_word(&p[i].duration), tempo);
		}

		// notes' transitions: based on "count" counter and the values stored in 
		// count_vect[] vector. If a note finishes playing, jump to the next one.
		if(count >= count_vect[n]){
			n++;
			if(pgm_read_word(&p[n].note) != N_SIL)
				buzzer_set(ENABLE, pgm_read_word(&p[n].note));
			else
				buzzer_set(DISABLE, pgm_read_word(&p[n].note));
			count = 0;
		}
		// If melody finishes playing, output TRUE
		if(n >= size){
			buzzer_set(DISABLE, N_C8);
			intro = TRUE;
			out = TRUE;
		}
		
		count++;	
	} else {
		// If "state" flag is disabled, stop playing melody and disable buzzer
		intro = TRUE;
		buzzer_set(DISABLE, N_SIL);	
	}	
	
	return out;
}

/*-----------------------------------------------------------------------------
-------------------------- L O C A L   F U N C T I O N S ----------------------
-----------------------------------------------------------------------------*/

static uint16_t note_duration(uint16_t counts, uint8_t tempo){
/*
*	Base tempo is 100 (100 bits per minute). 
*/

	float f_tempo = (float) tempo;
	float f_counts = (float) counts;
	float x = (100.0 / f_tempo) * f_counts;

	return (uint16_t) x;
}