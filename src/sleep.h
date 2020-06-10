/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file sleep.h
 * @brief Sleep and wake-up functions
 *
 * Handles the sleep and wake-up sequence of the CPU and peripherals
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

#ifndef SLEEP_H
#define SLEEP_H

#include "config.h"
#include <stdint.h>

state_t go_to_sleep(state_t state, uint8_t mode);

void peripherals_disable(void);

void peripherals_enable(void);

#endif /* SLEEP_H */