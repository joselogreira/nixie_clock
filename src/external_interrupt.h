/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file external_interrupt.h
 * @brief External interrupt config
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

#ifndef EXTERNAL_INTERRUPT
#define EXTERNAL_INTERRUPT

#include <stdint.h>

void pin_change_isr_init(void);

void buttons_set(uint8_t state);


#endif /* EXTERNAL_INTERRUPT */