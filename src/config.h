/**
 * @file config.h
 * @brief System-related global definitions
 *
 * - MCU core frequency
 * - Generic boolean macros
 * - Pins and ports macros
 * - Display states
 * - System-related macros
 *
 * @author Jose Logreira
 * @date 25.04.2018
 *
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>		/* Standard variable types */
#include <avr/io.h>		/* Device specific ports/peripherals */

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

#define FIRMWARE_DATE 	"15-NOV-2018"

// 16MHz ceramic resonator, prescaled by 8
#define F_CPU			2000000UL
// A non-usual baud rate, but it's needed due to the low operating frequency,
// so that it may work with no errors
#define BAUD 			125000UL
//#define BAUD 			19200UL

// GENERIC BOOLEAN MACROS
#define TRUE		0x01
#define FALSE 		0x00
#define ENABLE		TRUE
#define DISABLE		FALSE
#define ON 			TRUE
#define OFF			FALSE
#define HIGH		TRUE
#define LOW			FALSE
#define UP 			TRUE
#define DOWN 		FALSE
#define PASS 		0xAA
#define FAIL 		0xBB

// DISPLAY MODES
#define DISP_MODE_0		0xE0 	// Normal display
#define DISP_MODE_1		0xE1	// Transition effect 1
#define DISP_MODE_2		0xE2	// Transition effect 2
#define DISP_MODE_3		0xE3	// Transition effect 3
#define DISP_MODE_4		0xE4	// Transition effect 4
#define DISP_MODE_5		0xE5	// 10 mins effect
#define DISP_MODE_6		0xE6	// 1 hour effect
#define DISP_MODE_7		0xE7	// Show-Alarma effect
#define DISP_MODE_8 	0xE8 	// Transition effect as intro for other modes
#define DISP_MODE_9 	0xE9 	// Transition effect as intro for other modes

// TUBES NAMES
#define TUBE_A		0
#define TUBE_B		1
#define TUBE_C		2
#define TUBE_D		3
// When tube is blank
#define BLANK 		0xFF

// PINS AND PORTS MACROS
#define BOOST_SET(x)		( x ? (PORTB &= ~(1<<PORTB2)) : (PORTB |= (1<<PORTB2)))
#define RTC_SIGNAL_TOGGLE()	PINB |= (1<<PINB0)
#define RTC_SIGNAL_SET(x)	( x ? (PORTB |= (1<<PORTB0)) : (PORTB &= ~(1<<PORTB0)))

// TIME STRUCTURE MACROS
#define MODE_12H		0x12
#define MODE_24H		0x24
#define PERIOD_AM		0xAA
#define PERIOD_PM		0xFF

// BUTTON STATE MACROS
#define BTN_IDLE		0xEE
#define BTN_PUSHED		0xDD
#define BTN_RELEASED	0xAA

// Flag: Increment Hr/min/secs count
#define INC_HOUR		0x12
#define INC_MIN			0x13
#define INC_SEC			0x14

// DISPLAY STATES:
// Its advisable to declare them as a limited-range type of variable, to 
// explicitly make sure that all states are different and no unknown 
// states will be selected
typedef enum {
	SYSTEM_SLEEP,
	SYSTEM_INTRO,
	DISPLAY_TIME,
	DISPLAY_MENU,
	SET_TIME,
	SET_ALARM,
	SET_ALARM_ACTIVE,
	SET_HOUR_MODE,
	ALARM_TRIGGERED,
	SET_TRANSITIONS,
	SET_ALARM_THEME,
	USR_TEST,
	PRODUCTION_TEST,
	SYSTEM_RESET
} state_t;

/******************************************************************************
******************** E X T E R N A L   V A R I A B L E S **********************
******************************************************************************/

//extern display_s display;
//extern uint16_t cnt;
//extern volatile uint8_t sleep_mode;
extern uint8_t system_reset;
extern volatile uint8_t loop;

#endif	/* CONFIG_H */