/**
 * @file init.h
 * @brief Initial boot sequence
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

#ifndef INIT_H
#define INIT_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "adc.h"
#include "buzzer.h"
#include "config.h"
#include "eeprom.h"
#include "external_interrupt.h"
#include "menu_alarm.h"
#include "menu_time.h"
#include "timers.h"
#include "uart.h"

#include <stdint.h>

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void boot(void);

#endif	/* INIT_H */