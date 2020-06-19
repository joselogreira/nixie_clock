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

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void usr_test(volatile state_t *state);
void production_test(volatile state_t *state);

#endif /* DEBUG_H */