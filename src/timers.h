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

#include <stdint.h>

// LED Register Counters 
#define R_LED			OCR0A
#define G_LED			OCR0B
#define B_LED			OCR1A

void rtc_set(uint8_t state);

void base_timer_init(void);

void buzzer_init(void);

void leds_init(void);

void base_timer_set(uint8_t state);

void buzzer_set(uint8_t state, uint16_t note);

void leds_set(uint8_t state, uint8_t r, uint8_t g, uint8_t b);

#endif	/* TIMERS_H */