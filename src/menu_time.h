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

#include "config.h"

state_t display_time(state_t state);

state_t set_time(state_t state);

state_t set_hour_mode(state_t state);

#endif /* MENU_TIME_H */