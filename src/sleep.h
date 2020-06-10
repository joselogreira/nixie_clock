/**
 * @file sleep.h
 * @brief Sleep and wake-up functions
 *
 * Handles the sleep and wake-up sequence of the CPU and peripherals
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

#ifndef SLEEP_H
#define SLEEP_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "adc.h"
#include "buzzer.h"
#include "config.h"
#include "external_interrupt.h"
#include "timers.h"
#include "uart.h"
#include "util.h"

#include <stdint.h>

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

state_t go_to_sleep(state_t state, uint8_t mode);
void peripherals_disable(void);
void peripherals_enable(void);

#endif /* SLEEP_H */