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

#include "config.h"

#include <stdint.h>

/******************************************************************************
***************** S T R U C T U R E   D E C L A R A T I O N S *****************
******************************************************************************/

typedef volatile struct {
	uint8_t	sec;			// seconds
	uint8_t min;			// minutes
	uint8_t hour;			// hours
	uint8_t s_units;		// BCD seconds' units
	uint8_t s_tens;			// BCD seconds' tens
	uint8_t m_units;		// BCD minutes' units
	uint8_t m_tens;			// BCD minutes' tens
	uint8_t h_units;		// BCD hours' units
	uint8_t h_tens;			// BCD hours' tens
	uint8_t update;			// flag. 1Hz update?
	uint8_t hour_mode;		// 12/24h 
	uint8_t day_period;		// AM/PM
} time_s;

extern time_s time;

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void time_init(void);
void display_time(volatile state_t *state);
void set_time(volatile state_t *state);
void set_hour_mode(volatile state_t *state);

#endif /* MENU_TIME_H */