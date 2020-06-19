/**
 * @file init.c
 * @brief Initial boot sequence
 *
 * @author Jose Logreira
 * @date 15.05.2018
 *
 */

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "init.h"
#include "adc.h"
#include "buzzer.h"
#include "config.h"
#include "eeprom.h"
#include "external_interrupt.h"
#include "menu_alarm.h"
#include "menu_time.h"
#include "timers.h"
#include "uart.h"

#include <avr/io.h>
#include <stdint.h>

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static void ports_init(void);
static void system_defaults(void);

/*===========================================================================*/
/* 
* Initialize pin ports and default values of variables
*/
void boot(void)
{
	ports_init();
    system_defaults();
	rom_init();

    /*
    * Peripherals initialization.
    * This just sets the right values for each peripheral, but does not
    * enable them.
    * - leds (Timers 0 & 1)
    * - base timer (Timer 3)
    * - buzzer (Timer 4)
    * - ADC
    * - uart (used for debug)
    * - external interrupts (pin changes)
    * RTC is NOT initialized!!!
    */
    timer_leds_init();        // timers
    timer_base_init();  // timer
    timer_buzzer_init();      // timer
    adc_init();
    uart_init();
    pin_change_isr_init();
}

/*-----------------------------------------------------------------------------
-------------------------- L O C A L   F U N C T I O N S ----------------------
-----------------------------------------------------------------------------*/

/*===========================================================================*/
static void ports_init(void)
{
	// ADC inputs
	DDRA &= ~(1<<DDA0);		// V_HV
	DDRA &= ~(1<<DDA1);		// V_CTL_REG
	DDRA &= ~(1<<DDA2);		// V_IN
	DDRA &= ~(1<<DDA3);		// V_TST
	
	// IN-12 Anodes
	DDRD |= (1<<DDD1);		// Tube D
	DDRD |= (1<<DDD0);		// Tube C
	DDRE |= (1<<DDE5);		// Tube B
	DDRA |= (1<<DDA4);		// Tube A
	PORTD &= ~(1<<PORTD1);
	PORTD &= ~(1<<PORTD0);
	PORTE &= ~(1<<PORTE5);
	PORTA &= ~(1<<PORTA4);

	// IN-12 Cathodes
	DDRD |= (1<<DDD2);		// Number 0
	DDRC |= (1<<DDC3);		// Number 1
	DDRC |= (1<<DDC5);		// Number 2
	DDRA |= (1<<DDA7);		// Number 3
	DDRA |= (1<<DDA6);		// Number 4
	DDRA |= (1<<DDA5);		// Number 5
	DDRD |= (1<<DDD3);		// Number 6
	DDRD |= (1<<DDD4);		// Number 7
	DDRD |= (1<<DDD7);		// Number 8
	DDRD |= (1<<DDD6);		// Number 9
	PORTD &= ~(1<<PORTD2);
	PORTC &= ~(1<<PORTC3);
	PORTC &= ~(1<<PORTC5);
	PORTA &= ~(1<<PORTA7);
	PORTA &= ~(1<<PORTA6);
	PORTA &= ~(1<<PORTA5);
	PORTD &= ~(1<<PORTD3);
	PORTD &= ~(1<<PORTD4);
	PORTD &= ~(1<<PORTD7);
	PORTD &= ~(1<<PORTD6);

	// Boost ENable
	DDRB |= (1<<DDB2);
	PORTB |= (1<<PORTB2);

	// Buzzer
	DDRC |= (1<<DDC4);
	PORTC &= ~(1<<PORTC4);

	// RGB LEDs
	DDRB |= (1<<DDB3);		// Red
	DDRB |= (1<<DDB4);		// Green
	DDRD |= (1<<DDD5);		// Blue
	PORTB &= ~(1<<PORTB3);
	PORTB &= ~(1<<PORTB4);
	PORTD &= ~(1<<PORTD5);

	// Input buttons
	DDRB &= ~(1<<DDB5);		// Button Z
	DDRB &= ~(1<<DDB6);		// Button Y
	DDRB &= ~(1<<DDB7);		// Button X
	PORTB |= (1<<PORTB5);	// Pull-up enabled
	PORTB |= (1<<PORTB6);	// Pull-up enabled
	PORTB |= (1<<PORTB7);	// Pull-up enabled

	// External power input
	DDRC &= ~(1<<DDC2);

	// Debug I2C interface
	DDRC |= (1<<DDC0);		// SCL
	DDRC |= (1<<DDC1);		// SDA
	PORTC |= (1<<PORTC0);
	PORTC |= (1<<PORTC1);

	// Debug UART interface
	DDRE &= ~(1<<DDE2);		// RX, external Pull-down required
	DDRE |= (1<<DDE3);		// TX	
	PORTE |= (1<<PORTE3);

	// Debug RTC LED signal
	DDRB |= (1<<DDB0);
	PORTB &= ~(1<<PORTB0);

	// Port not used
	DDRB |= (1<<DDB1);
	PORTB &= ~(1<<PORTB1);
}

/*===========================================================================*/
/*
* Initializes all global system structures to default values
*/
static void system_defaults(void)
{
	time_init();
    alarm_init();
    buttons_init();
    display_init();	
}