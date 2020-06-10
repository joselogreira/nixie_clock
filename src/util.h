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

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "buzzer.h"
#include "config.h"
#include "external_interrupt.h"
#include "timers.h"
#include "uart.h"
#include <stdint.h>

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void set_tube(uint8_t k);
void set_digit(uint8_t n);
void set_digit_t(uint8_t n);

void buttons_check(btn_s *btn);
void bin_to_ascii(char *p, uint8_t bin);
void led_blink(uint8_t n, uint8_t time);
uint8_t check_alarm(void);
void update_time_variables(void);
uint8_t random_number(uint8_t seed);

#endif	/* UTIL_H */