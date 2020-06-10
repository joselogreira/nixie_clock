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

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"

#include <stdint.h>

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void pin_change_isr_init(void);
void buttons_set(uint8_t state);

uint8_t btn_check_x(void);
uint8_t btn_check_y(void);
uint8_t btn_check_z(void);

#endif /* EXTERNAL_INTERRUPT */