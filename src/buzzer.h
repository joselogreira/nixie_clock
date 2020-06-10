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

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"
#include "timers.h"
#include "uart.h"

#include <stdint.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

// Alarm Themes
#define MAJOR_SCALE		10
#define STAR_WARS 		15
#define IMPERIAL_MARCH 	20
#define SUPER_MARIO 	25
#define SIMPLE_ALARM	30
#define DIOMEDES		35
#define USA_ANTHEM		40

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void buzzer_beep(void);
void buzzer_set(uint8_t state);
uint8_t buzzer_music(uint8_t theme, uint8_t state);

#endif /* BUZZER_H */