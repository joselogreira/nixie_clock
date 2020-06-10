 /******************************************************************************
 * Copyright (C) 2017 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file eeprom.h
 * @brief EEPROM manipulation and structure
 *
 * Functions to read/write EEPROM data based on user configuration, device
 * testing and system parameters
 *
 * @author Jose Logreira
 * @date 12.10.2018
 *
 */

#ifndef EEPROM_H
#define EEPROM_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"

#include <avr/eeprom.h>
#include <stdint.h>

/******************************************************************************
****************** F U N C T I O N   P R O T O T Y P E S **********************
******************************************************************************/

void rom_init(void);

uint8_t rom_increase_test_cnt(void);
uint8_t rom_query_test_cnt(void);

void rom_store_voltages_results(uint8_t *t1, uint8_t *t2, uint8_t *t3);
void rom_store_timing_results(uint8_t clock_ok, uint16_t *times);
void rom_store_buzzer_results(uint8_t buzzer_ok);
void rom_store_leds_results(uint8_t *leds_ok);

void rom_query_voltages_test(uint8_t *v);
uint8_t rom_query_rtc_ok_test(void);
void rom_query_rtc_time_test(uint16_t *t);
uint8_t rom_query_buzzer_ok_test(void);
void rom_query_leds_test(uint8_t *l);

#endif /* EEPROM_H */