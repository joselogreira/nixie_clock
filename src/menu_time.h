/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file menu_time.h
 * @brief Time display and manipulation
 *
 * Utility functions to display the desired time info and messages
 *
 * @author Jose Logreira
 * @date 25.04.2018
 *
 */

#ifndef MENU_TIME_H
#define MENU_TIME_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "buzzer.h"
#include "config.h"
#include "math.h"
#include "timers.h"
#include "uart.h"
#include "util.h"

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

state_t display_time(state_t state);
state_t set_time(state_t state);
state_t set_hour_mode(state_t state);

#endif /* MENU_TIME_H */