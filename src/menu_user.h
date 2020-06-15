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

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "buzzer.h"
#include "config.h"
#include "timers.h"
#include "uart.h"
#include "util.h"

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void intro(volatile state_t *state);
void display_menu(volatile state_t *state);
void set_transitions(volatile state_t *state);
void usr_test_tubes(volatile state_t *state);

#endif /* MENU_USER_H */