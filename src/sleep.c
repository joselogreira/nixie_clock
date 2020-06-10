/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file sleep.c
 * @brief Sleep and wake-up functions
 *
 * Handles the sleep and wake-up sequence of the CPU and peripherals
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "sleep.h"

#include <stdint.h>
#include <avr/sleep.h>		/* Macros for handling spleep routines */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

enum {
	DISABLE_PERIPHERALS,
	SLEEP_CPU,
	WAKEUP_CPU,
	ENABLE_SYSTEM
};

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static void ports_power_save(uint8_t state);

/*===========================================================================*/
/* 	
* When this function is entered, MCU execution is kept within this function 
* until 12V adapter is connected (EXT_PWR is true).
* - Wake up sources: 
*	- EXT_PWR pin change interrupt: wakes up and exit
*	- TIMER2 1Hz compare match: only in POWER SAVE, not in POWER DOWM
* The steps to sleep and wake up are implemented within a switch() statement to
* be able to jump back when needed
*/
state_t go_to_sleep(state_t state, uint8_t mode)
{
	uint8_t step = DISABLE_PERIPHERALS;
	uint8_t sys = FALSE;

	while(TRUE){
		switch(step){

			case DISABLE_PERIPHERALS:
				// disable all system and external peripheral
				if(sleep_mode == RTC_ENABLE)
					uart_send_string("\n\rGood Bye   ");
				peripherals_disable();
				step = SLEEP_CPU;
				break;

			case SLEEP_CPU:
				/* 
				* POWER_SAVE/POWER_DOWN SLEEP MODES
				* Once sei() is executed, NO interrupt request should be triggered, since
				* ISR execution may break the timed shut down CPU sequence and may not go
				* to sleep
				*/
				if(sleep_mode == RTC_DISABLE)
					set_sleep_mode(SLEEP_MODE_PWR_DOWN);	// Select POWER_DOWN sleep
				else if(sleep_mode == RTC_ENABLE)
					set_sleep_mode(SLEEP_MODE_PWR_SAVE);	// Select POWER_SAVE sleep
				sleep_enable();						// Enable Sleep Mode 
				sleep_bod_disable();				// Disable BOD when sleep 
				sei();
				sleep_cpu();						// GO TO SLEEP 
				/*
				*	WAKE UP starts here
				*	MCU executes the ISR that triggered the WakeUp
				*	and resumes execution HERE 
				*/
				sleep_disable();	// Disable Sleep Mode to avoid unwanted sleep re-entry 
				cli();
				step = WAKEUP_CPU;
				break;

			case WAKEUP_CPU:
				// Look for external power (12V adapter). If present, enable full system
				// if not present, go to sleep again 
				if(EXT_PWR) {
					step = ENABLE_SYSTEM;
					sleep_mode = RTC_ENABLE;
				} else {
					step = SLEEP_CPU;
				}
				break;

			case ENABLE_SYSTEM:
				// enable all system and external peripherals
				peripherals_enable();
				uart_send_string("\n\rWhat's Up!");
				// check system voltages
				//_delay_ms(1500);
				_delay_ms(2000);
				if(!adc_voltages_test()){
				    system_reset = TRUE;
				    state = SYSTEM_RESET;				    
				} else {
					if(alarm.triggered) state = ALARM_TRIGGERED;
					else state = SYSTEM_INTRO;
				}
				sys = TRUE;				
				break;

			default:
				break;
		}
		if(sys) break;
	}

	return state;
}

/*===========================================================================*/
/*
* When going to sleep: All peripherals should be disabled to
* save the most power possible. The following peripherals are
* disabled:
* - ADC
* - USART
* - Timers
* - Buttons' external interrupt (External power ISR is still active)
* - RTC only if entering PWR_DOWN sleep mode. Otherwise, keep running
*
* * Boost is not explicitly disabled since the absence of power adapter
* 	immediately disables the boost controller. What must be done is to
*	disable the HIGH level of BOOST_EN pin, since it includes a pull 
*	down resistor that consumes power
* * Buttons' pull-ups also disabled to avoid power consumption when
*   pressed
*/
void peripherals_disable(void)
{
	adc_set(DISABLE);
	uart_set(DISABLE);
	buzzer_set(DISABLE);
	timer_leds_set(DISABLE, 0, 0, 0);
	timer_base_set(DISABLE);
	buttons_set(DISABLE);
	// Disable RTC only if entering POWER DOWN
	if(sleep_mode == RTC_DISABLE)
		timer_rtc_set(DISABLE);
	ports_power_save(DISABLE);
}

/*===========================================================================*/
/*
* When waking up: All peripherals should be enabled to use them when
* power adapter is available. The following peripherals are enabled:
* - ADC
* - USART
* - Timers
* - Buttons' external interrupt (External power ISR is still active)
* - RTC always enabled when waking up
* * Buttons' pull-ups also enabled
*/
void peripherals_enable(void)
{
	ports_power_save(ENABLE);
	adc_set(ENABLE);
	uart_set(ENABLE);
	timer_base_set(ENABLE);
	buttons_set(ENABLE);
	timer_rtc_set(ENABLE);	// Always enable RTC
	BOOST_SET(ENABLE);
	// missing I2C
}

/*-----------------------------------------------------------------------------
-------------------------- L O C A L   F U N C T I O N S ----------------------
-----------------------------------------------------------------------------*/

/*===========================================================================*/
static void ports_power_save(uint8_t state)
{
	if(state){
		// Set BOOST_EN pin HIGH (Boost disabled)
		PORTB |= (1<<PORTB0);
		// Change buttons' pins direction
		DDRB &= ~((1<<DDB5) | (1<<DDB6) | (1<<DDB7));
		// Enable buttons' pull-ups
		PORTB |= (1<<PORTB5) | (1<<PORTB6) | (1<<PORTB7);
	} else {
		// Set BOOST_EN pin LOW to avoid current consumption of the pull-down
		// resistor within the boost regulator. In absence of 12V adapter, the
		// boost regulator won't turn on
		PORTB &= ~(1<<PORTB0);
		// Disable buttons' pull-ups to avoid current consumption when buttons
		// are accidentally pressed
		PORTB &= ~((1<<PORTB5) | (1<<PORTB6) | (1<<PORTB7));
		// Change buttons' pins direction
		DDRB |= (1<<DDB5) | (1<<DDB6) | (1<<DDB7);
		// Clear IN-12 driver
		set_tube(DISABLE);
		set_digit(BLANK);
	}
}