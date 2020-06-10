/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file menu_alarm.h
 * @brief Alarm-related functions
 *
 * Utility functions to change the alarm options
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

#ifndef MENU_ALARM_H
#define MENU_ALARM_H

#include "config.h"

state_t set_alarm_active(state_t state);

state_t set_alarm(state_t state);

state_t alarm_triggered(state_t state);

state_t set_alarm_theme(state_t state);

#endif /* MENU_ALARM_H */