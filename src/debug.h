/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file debug.h
 * @brief Test functions
 *
 * Functions that test all system functions
 *
 * @author Jose Logreira
 * @date 08.10.2018
 *
 */

#ifndef DEBUG_H
#define DEBUG_H

#include "config.h"

#include <stdint.h>

state_t usr_test(state_t state);

state_t production_test(state_t state);

#endif /* DEBUG_H */