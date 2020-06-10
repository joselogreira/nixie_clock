/**
 * @file eeprom.c
 * @brief EEPROM manipulation and structure
 *
 * Functions to read/write EEPROM data based on user configuration, device
 * testing and system parameters
 *
 * @author Jose Logreira
 * @date 12.08.2018
 *
 */

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "eeprom.h"

#include <avr/eeprom.h>

/******************************************************************************
***************** E E P R O M   V A R S   D E F I N I T I O N *****************
******************************************************************************/

// EEPROM sections:
uint8_t EEMEM sys_blank;
uint8_t EEMEM test_cnt;				// N times it's been tested
uint8_t EEMEM test_voltages[24];	// Stores voltages check result
uint8_t EEMEM test_rtc[7];			// RTC signal test result
uint8_t EEMEM test_buzzer;
uint8_t EEMEM test_leds[4];

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

/*===========================================================================*/
/*
* Initial, unprogrammed content of ROM is all bytes 0xFF. Thus, it must be
* initialized with some default values
*/
void rom_init(void)
{
	uint8_t val = eeprom_read_byte(&sys_blank);

	if(val != 0xAA){
		// ROM not initialized.
		eeprom_update_byte(&sys_blank, 0xAA);
		// reset test counter
		eeprom_update_byte(&test_cnt, 0);
		
		// reset test buzzer result
		eeprom_update_byte(&test_buzzer, 0);
		
		uint8_t v[24];
		for(uint8_t i = 0; i < 24; i++)
			v[i] = 0;
		// reset test voltages result
		eeprom_update_block((const void *)v, (void *)test_voltages, sizeof(test_voltages));
		// reset test rtc result
		eeprom_update_block((const void *)v, (void *)test_rtc, sizeof(test_rtc));
		// reset test leds result
		eeprom_update_block((const void *)v, (void *)test_leds, sizeof(test_leds));
	} 
}

/*===========================================================================*/
uint8_t rom_increase_test_cnt(void)
{
	uint8_t val = eeprom_read_byte(&test_cnt);
	val++;
	eeprom_update_byte(&test_cnt, val);

	return val;
}

/*===========================================================================*/
uint8_t rom_query_test_cnt(void)
{
	return eeprom_read_byte(&test_cnt);
}

/*===========================================================================*/
/*
* It receives all test results in 3 different vectors. Each vector contains
* 2 sets of tests: the first one was obtained when the boost was off, and 
* the second one obtained when the boost was on. The vectors are organized
* as follows:
* First set: Boost OFF
* 	1. High Voltage boost output voltage, when OFF
*	2. Boost controller internal linear regulator voltage, when OFF
* 	3. Input voltage to the circuit
* 	4. MCU voltage rail
* Second set: Boost ON
* 	5. High Voltage boost output voltage, when ON
*	6. Boost controller internal linear regulator voltage, when ON
* 	7. Input voltage to the circuit
* 	8. MCU voltage rail
*/
void rom_store_voltages_results(uint8_t *t1, uint8_t *t2, uint8_t *t3)
{
	for(uint8_t i = 0; i < 8; i++){
		if(*(t1 + i) == TRUE) eeprom_update_byte((test_voltages + i), PASS);
		else eeprom_update_byte((test_voltages + i), FAIL);		
	}
	for(uint8_t i = 0; i < 8; i++){
		if(*(t2 + i) == TRUE) eeprom_update_byte((test_voltages + 8 + i), PASS);
		else eeprom_update_byte((test_voltages + 8 + i), FAIL);		
	}
	for(uint8_t i = 0; i < 8; i++){
		if(*(t3 + i) == TRUE) eeprom_update_byte((test_voltages + 16 + i), PASS);
		else eeprom_update_byte((test_voltages + 16 + i), FAIL);		
	}
}

/*===========================================================================*/
void rom_store_timing_results(uint8_t clock_ok, uint16_t *times)
{
	if(clock_ok) eeprom_update_byte(test_rtc, PASS);
	else eeprom_update_byte(test_rtc, FAIL);

	eeprom_update_block((const void *)times, (void *)(test_rtc + 1), sizeof(test_rtc) - 1);
}

/*===========================================================================*/
void rom_store_buzzer_results(uint8_t buzzer_ok)
{
	if(buzzer_ok) eeprom_update_byte(&test_buzzer, PASS);
	else  eeprom_update_byte(&test_buzzer, FAIL);
}

/*===========================================================================*/
void rom_store_leds_results(uint8_t *leds_ok)
{
	for(uint8_t i = 0; i < 4; i++){
		if(*(leds_ok + i) == TRUE) eeprom_update_byte((test_leds + i), PASS);
		else eeprom_update_byte((test_leds + i), FAIL);		
	}
}

/*===========================================================================*/
void rom_query_voltages_test(uint8_t *v)
{
	eeprom_read_block((void *)v, test_voltages, sizeof(test_voltages));
}

/*===========================================================================*/
uint8_t rom_query_rtc_ok_test(void)
{
	return eeprom_read_byte(test_rtc);
}

/*===========================================================================*/
void rom_query_rtc_time_test(uint16_t *t)
{
	eeprom_read_block((void *)t, test_rtc + 1, sizeof(test_rtc) - 1);
}

/*===========================================================================*/
uint8_t rom_query_buzzer_ok_test(void)
{
	return eeprom_read_byte(&test_buzzer);
}

/*===========================================================================*/
void rom_query_leds_test(uint8_t *l)
{
	eeprom_read_block((void *)l, test_leds, sizeof(test_leds));
}