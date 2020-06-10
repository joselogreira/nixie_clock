/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file util.h
 * @brief Miscelaneous functions
 *
 * @author Jose Logreira
 * @date 07.05.2018
 *
 */
#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

void set_tube(uint8_t k);

void set_digit(uint8_t n);

void set_digit_t(uint8_t n);

void buttons_check(void);

void bin_to_ascii(char *p, uint8_t bin);

void led_blink(uint8_t n, uint8_t time);

uint8_t check_alarm(void);

void update_time_variables(void);

uint8_t random_number(uint8_t seed);

#endif	/* UTIL_H */