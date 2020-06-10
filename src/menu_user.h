/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file menu_user.c
 * @brief Various user configurations
 *
 * Utility functions to configure some user options
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

#ifndef MENU_USER_H
#define MENU_USER_H

#include "config.h"
#include "util.h"

state_t intro(state_t state);

state_t display_menu(state_t state);

state_t set_transitions(state_t state);

state_t usr_test_tubes(state_t state);

#endif /* MENU_USER_H */