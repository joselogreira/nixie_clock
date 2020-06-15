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

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "buzzer.h"
#include "config.h"
#include "menu_time.h"
#include "timers.h"
#include "uart.h"
#include "util.h"

/******************************************************************************
***************** S T R U C T U R E   D E C L A R A T I O N S *****************
******************************************************************************/

typedef struct {
	uint8_t	sec;			// seconds
	uint8_t min;			// minutes
	uint8_t hour;			// hours
	uint8_t s_units;		// BCD seconds' units
	uint8_t s_tens;			// BCD seconds' tens
	uint8_t m_units;		// BCD minutes' units
	uint8_t m_tens;			// BCD minutes' tens
	uint8_t h_units;		// BCD hours' units
	uint8_t h_tens;			// BCD hours' tens
	uint8_t hour_mode;		// 12/24h 
	uint8_t day_period;		// AM/PM
	uint8_t active;			// flag. Alarm ON?
	volatile uint8_t triggered;	// flag. Alarm MATCH?
	uint8_t theme;			// Alarm melody
} alarm_s;

extern alarm_s alarm;

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void alarm_init(void);
void set_alarm_active(volatile state_t *state);
void set_alarm(volatile state_t *state);
void alarm_triggered(volatile state_t *state);
void set_alarm_theme(volatile state_t *state);

#endif /* MENU_ALARM_H */