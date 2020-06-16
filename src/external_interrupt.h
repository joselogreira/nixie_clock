/**
 * @file external_interrupt.h
 * @brief External interrupt config
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

#ifndef EXTERNAL_INTERRUPT
#define EXTERNAL_INTERRUPT

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"

#include <stdint.h>

/******************************************************************************
***************** S T R U C T U R E   D E C L A R A T I O N S *****************
******************************************************************************/

typedef struct {
	volatile uint8_t query;	// flag; query button state
	uint8_t action;			// flag; button action activated
	uint8_t lock;			// flag; button locked
	uint8_t state;			// button state: IDLE, PUSHED, RELEASED
	uint16_t count;			// time counter
	uint8_t delay1;			// flag; delay 1 elapsed
	uint8_t delay2;			// flag; delay 2 elapsed
	uint8_t delay3;			// flag; delay 3 elapsed
	uint8_t (*check)();		// function pointer to a state check handler
} btn_s;

extern btn_s btnX, btnY, btnZ;

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

// External Power detection pin
#define EXT_PWR			(PINC & (1<<PINC2))

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void buttons_init(void);
void pin_change_isr_init(void);
void buttons_set(uint8_t state);

uint8_t btn_check_x(void);
uint8_t btn_check_y(void);
uint8_t btn_check_z(void);

#endif /* EXTERNAL_INTERRUPT */