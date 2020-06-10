/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file timers.h
 * @brief Timer counter configurations
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

#ifndef TIMERS_H
#define TIMERS_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "buzzer.h"
#include "config.h"

#include <stdint.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

// LED Register Counters 
#define R_LED			OCR0A
#define G_LED			OCR0B
#define B_LED			OCR1A

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void timer_rtc_set(uint8_t state);

void timer_base_init(void);
void timer_buzzer_init(void);
void timer_leds_init(void);

void timer_base_set(uint8_t state);
void timer_buzzer_set(uint8_t state, uint16_t note);
void timer_leds_set(uint8_t state, uint8_t r, uint8_t g, uint8_t b);

#endif	/* TIMERS_H */