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

#include "config.h"

#include <stdint.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

// RTC SLEEP MODES:
// If enabled: RTC will keep running during sleep mode
// If disabled: RTC will stop during sleep mode
#define RTC_DISABLE 	FALSE
#define RTC_ENABLE	 	TRUE

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void go_to_sleep(volatile state_t *state, volatile uint8_t mode);
void peripherals_disable(volatile uint8_t mode);
void peripherals_enable(void);

#endif /* SLEEP_H */