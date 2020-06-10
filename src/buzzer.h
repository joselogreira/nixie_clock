/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file buzzer.h
 * @brief Buzzer-related functions and tones
 *
 * Utility functions to implement the buzzer tones
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

#ifndef BUZZER_H
#define BUZZER_H

#include "config.h"
#include <stdint.h>

// Themes
#define MAJOR_SCALE		10
#define STAR_WARS 		15
#define IMPERIAL_MARCH 	20
#define SUPER_MARIO 	25
#define SIMPLE_ALARM	30
#define DIOMEDES		35
#define USA_ANTHEM		40

// TIMER COUNTER 3 MACROS (notes)
// All notes are divided by 8, since the operation frequency was changed from
// 16MHz to 2MHz
#define N_A6		4545/8	// 1760Hz
#define N_B6		4050/8	// 1975.53Hz
#define N_C7		3822/8	// 2093.00Hz
#define N_Db7		3608/8	// 2217.46Hz
#define N_D7		3405/8	// 2349.32Hz
#define N_E7		3034/8	// 2637.02Hz
#define N_F7		2863/8	// 2793.83Hz
#define N_G7		2551/8	// 3135.96Hz
#define N_Gs7		2408/8	// 3322.44Hz
#define N_A7		2273/8	// 3520Hz
#define N_Bb7		2145/8	// 3729.31Hz
#define N_B7		2025/8	// 3951.07Hz
#define N_C8		1911/8	// 4186.01HzHz
#define N_Cs8		N_Db8
#define N_Db8		1804/8	// 4434.92Hz
#define N_D8		1703/8	// 4698.63Hz
#define N_Ds8		1607/8	// 4978.03Hz
#define N_E8		1517/8	// 5274.04Hz
#define N_F8		1432/8	// 5587.65Hz
#define N_Fs8		N_Gb8
#define N_Gb8		1351/8	// 5919.91Hz
#define N_G8		1276/8	// 6271.93Hz
#define N_A8		1136/8	// 7040.00Hz
#define N_B8		1012/8	// 7902.13Hz
#define N_C9		956/8	// 8372.02Hz
#define N_SIL		0xFFFF	// Silence

// Music notes duration (reference tempo: 100)
#define SIXTEENTH_NOTE	(QUARTER_NOTE / 4)
#define TWELVETH_NOTE	(QUARTER_NOTE / 3)
#define EIGHTH_NOTE		(QUARTER_NOTE / 2)
#define SIXTH_NOTE 		(TWELVETH_NOTE * 2)
#define QUARTER_NOTE	(60000 / 100)
#define HALF_NOTE		(QUARTER_NOTE * 2)
#define WHOLE_NOTE		(QUARTER_NOTE * 4)

void buzzer_beep(void);

uint8_t buzzer_music(uint8_t theme, uint8_t state);

#endif /* BUZZER_H */