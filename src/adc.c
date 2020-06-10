/**
 * @file adc.c
 * @brief ADC peripheral driver
 *
 * ADC peripheral settings and related functions
 *
 * @author Jose Logreira
 * @date 25.04.2018
 *
 */

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "adc.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

#define	ADC_PS_DIV8		(1<<ADPS1 | 1<<ADPS0)
#define	ADC_PS_DIV16	(1<<ADPS2)
#define	ADC_PS_DIV128	(1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0)
#define ADC_MUX_MASK	(1<<MUX4 | 1<<MUX3 | 1<<MUX2 | 1<<MUX1 | 1<<MUX0)

#define ADC_V_HV		0										/* ADC0 */
#define ADC_V_CTL_REG	(1<<MUX0)								/* ADC1 */
#define ADC_V_IN		(1<<MUX1)								/* ADC2 */
#define ADC_V_TST		(1<<MUX1 | 1<<MUX0)						/* ADC3 */
#define ADC_V_REF 		(1<<MUX4 | 1<<MUX3 | 1<<MUX2 | 1<<MUX1)	/* V_REF */

/* 
* ADC nominal voltages:
*/
#define V_HV  			((float) 3.794)	// nominal 160V
#define V_HV_OFF		((float) 0.273)	// nominal 11.5V
#define V_CTL_REG 		((float) 2.12)	// nominal 8V
#define V_CTL_REG_OFF 	((float) 0.0)	// nominal 0V
#define V_IN 			((float) 3.01)	// nominal 11.4V
#define V_REF 			((float) 1.1)	// nominal 1.1V
#define V_DD 			((float) 4.3)	// nominal MCU voltage v_dd

// ADC "oversampling", i.e. # of ADC readings per channel
#define ADC_READ_N		5

// Structure for the ADC voltage measurements
typedef struct {
	uint16_t val;		// raw measured value
	uint16_t val_nom;	// nominal (ideal) value
	uint16_t val_min;	// minimum accpeted value
	uint16_t val_max;	// maximum accepted value
	uint8_t good;		// flag, value measured out of range
} voltage_s;

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static void convert_and_send(uint16_t v, float c, float what, float v_dd);

/*===========================================================================*/
/* 
* ADC conversions are not enabled here. Just the necessary prescaler and
* multiplexer mask cleared
*/
void adc_init(void)
{
	ADMUX &= ~(ADC_MUX_MASK);		// Clear ADC mux selection
	ADMUX |= (1<<REFS0);			// Choose AVcc as ADC reference
	ADCSRA |= ADC_PS_DIV16;			// Set prescaler
}

/*===========================================================================*/
void adc_set(uint8_t state){

	if(state)
		ADCSRA |= (1<<ADEN);		// Enable ADC conversions
	else
		ADCSRA &= ~(1<<ADEN);		// Disable ADC conversions
}

/*===========================================================================*/
uint16_t adc_read(uint8_t adcx)
{	
	uint8_t i;
	uint16_t avg = 0;

	ADMUX &= ~(ADC_MUX_MASK);		// Clear ADC mux selection
	ADMUX |= adcx;					// Select proper ADC channel
	// if I measure internal V_BG (1.1V band gap voltage), it takes some time
	// to stabilize voltage
	if(adcx == ADC_V_REF) _delay_us(200);
	else _delay_us(20);
	ADCSRA |= (1<<ADSC);			// This starts the conversion
	
	// perform n reads and accumulate them in a temporary variable
	for (i = 0; i < ADC_READ_N; i++){
		while ((ADCSRA & (1<<ADSC)));	// Wait 'til conversion completed
		avg += ADC;
	}
	// average the ADC reads
	avg = avg / ((uint16_t)ADC_READ_N);	// average value	
	
	return avg;
}

/*===========================================================================*/
/*
* Check System Voltages
* The MCU is NOT directly fed by the 5V rail, but it's placed after a diode.
* Thus, we rely on the value of the internal reference voltage to deduce the
* voltage reference for the ADC (the MCU rail voltage)
*/
uint8_t adc_voltages_test(void)
{
	uint16_t v_ref_raw;
	float v_dd;
	voltage_s v_hv, v_ctl_reg, v_in;

	// read internal reference
	v_ref_raw = adc_read(ADC_V_REF);
	// based on internal reference value, deduce the ADC reference voltage
	v_dd = V_REF * 1023.0 / ((float)(v_ref_raw));
	
	// compute the raw (integer) limit values for each measure
	v_hv.val_nom = (uint16_t)((V_HV / v_dd) * 1023.0);
	v_hv.val_min = (uint16_t)(((float)v_hv.val_nom) * 0.85);
	v_hv.val_max = (uint16_t)(((float)v_hv.val_nom) * 1.15);

	v_ctl_reg.val_nom = (uint16_t)((V_CTL_REG / v_dd) * 1023.0);
	v_ctl_reg.val_min = (uint16_t)(((float)v_ctl_reg.val_nom) * 0.85);
	v_ctl_reg.val_max = (uint16_t)(((float)v_ctl_reg.val_nom) * 1.15);

	v_in.val_nom = (uint16_t)((V_IN / v_dd) * 1023.0);
	v_in.val_min = (uint16_t)(((float)v_in.val_nom) * 0.8);
	v_in.val_max = (uint16_t)(((float)v_in.val_nom) * 1.2);

	// measure each voltage individually
	v_hv.val = adc_read(ADC_V_HV);
	v_ctl_reg.val = adc_read(ADC_V_CTL_REG);
	v_in.val = adc_read(ADC_V_IN);

	// compare measured value with the calculated voltage limits
	if((v_hv.val >= v_hv.val_min) && (v_hv.val <= v_hv.val_max)) v_hv.good = TRUE;
	else v_hv.good = FALSE;

	if((v_ctl_reg.val >= v_ctl_reg.val_min) && (v_ctl_reg.val <= v_ctl_reg.val_max)) v_ctl_reg.good = TRUE;
	else v_ctl_reg.good = FALSE;
	
	if((v_in.val >= v_in.val_min) && (v_in.val <= v_in.val_max)) v_in.good = TRUE;
	else v_in.good = FALSE;
	
	// Report voltages through serial port
	uart_send_string_p(PSTR("\n\rSYSTEM VOLTAGES:"));
	convert_and_send(v_hv.val, 42.2, V_HV, v_dd); 			// HIGH VOLTAGE (BOOST CONVERTER OUTPUT)
	convert_and_send(v_ctl_reg.val, 3.78, V_CTL_REG, v_dd);	// BOOST CONTROLLER INTERNAL REGULATOR
	convert_and_send(v_in.val, 3.78, V_IN, v_dd);			// INPUT ADAPTER VOLTAGE
	convert_and_send(1023, 1.0, V_DD, v_dd);				// CALCULATED VOLTAGE RAIL (4.3V ideallly)

	// If some voltage is out of range, warn about it
	if(!v_hv.good) uart_send_string_p(PSTR("\n\rWARING! - Boost voltage out of range"));
	if(!v_ctl_reg.good) uart_send_string_p(PSTR("\n\rWARING! - Boost controller regulator voltage out of range"));
	if(!v_in.good) uart_send_string_p(PSTR("\n\rWARNING! - Input voltage out of range"));
	
	// If some voltage is out of range, return false.
	return ((v_hv.good && v_ctl_reg.good && v_in.good));
}

/*===========================================================================*/
/*
* Checks V_TST pin 3 times. If any of the 3 measurements gives a voltage
* greater than (approximately) 0.5V, it returns FALSE.
* In other words, all 3 measurements should be below 0.5V to return TRUE
*/
uint8_t adc_factory_test_check(void)
{
	uint8_t t = TRUE;

	for(uint8_t i = 0; i < 3; i++){
		if(adc_read(ADC_V_TST) > 100){
			t = FALSE;
			break;
		}
	}

	return t;
}

/*===========================================================================*/
/*
* Performs the same voltage reading that adc_voltages_test() does, but it
* takes into account the current state of the boost controller.
* At the end of the test, it stores the reslults using the vector pointer
* that was passed to the function.
*/
void adc_factory_voltages_test(uint8_t boost, uint8_t *p)
{
	uint16_t v_ref_raw;
	float v_dd;
	uint8_t v_dd_good;
	voltage_s v_hv, v_ctl_reg, v_in;

	// read internal reference
	v_ref_raw = adc_read(ADC_V_REF);
	// based on internal reference value, deduce the ADC reference voltage
	v_dd = V_REF * 1023.0 / ((float)(v_ref_raw));	// (float value)
	
	// compute the raw (integer) limit values for each measure
	if(boost == BOOST_ON) v_hv.val_nom = (uint16_t)((V_HV / v_dd) * 1023.0);
	else v_hv.val_nom = (uint16_t)((V_HV_OFF / v_dd) * 1023.0);
	v_hv.val_min = (uint16_t)(((float)v_hv.val_nom) * 0.85);
	v_hv.val_max = (uint16_t)(((float)v_hv.val_nom) * 1.15);

	if(boost == BOOST_ON){
		v_ctl_reg.val_nom = (uint16_t)((V_CTL_REG / v_dd) * 1023.0);
		v_ctl_reg.val_min = (uint16_t)(((float)v_ctl_reg.val_nom) * 0.85);
		v_ctl_reg.val_max = (uint16_t)(((float)v_ctl_reg.val_nom) * 1.15);
	} else {
		v_ctl_reg.val_nom = 0;
		v_ctl_reg.val_min = 0;
		v_ctl_reg.val_max = 25;		// 0.1V
	}

	v_in.val_nom = (uint16_t)((V_IN / v_dd) * 1023.0);
	v_in.val_min = (uint16_t)(((float)v_in.val_nom) * 0.8);
	v_in.val_max = (uint16_t)(((float)v_in.val_nom) * 1.2);

	// measure each voltage individually
	v_hv.val = adc_read(ADC_V_HV);
	v_ctl_reg.val = adc_read(ADC_V_CTL_REG);
	v_in.val = adc_read(ADC_V_IN);

	// compare measured value with the calculated voltage limits
	if((v_hv.val >= v_hv.val_min) && (v_hv.val <= v_hv.val_max)) v_hv.good = TRUE;
	else v_hv.good = FALSE;

	if((v_ctl_reg.val >= v_ctl_reg.val_min) && (v_ctl_reg.val <= v_ctl_reg.val_max)) v_ctl_reg.good = TRUE;
	else v_ctl_reg.good = FALSE;
	
	if((v_in.val >= v_in.val_min) && (v_in.val <= v_in.val_max)) v_in.good = TRUE;
	else v_in.good = FALSE;

	if((v_dd < 5.0) && (v_dd > 4.0)) v_dd_good = TRUE;
	else v_dd_good = FALSE;

	// Report voltages through serial port
	convert_and_send(v_hv.val, 42.2, V_HV, v_dd); 			// HIGH VOLTAGE (BOOST CONVERTER OUTPUT)
	if(v_hv.good) uart_send_string_p(PSTR(" - PASS"));
	else uart_send_string_p(PSTR(" - FAIL!!!"));
	convert_and_send(v_ctl_reg.val, 3.78, V_CTL_REG, v_dd);	// BOOST CONTROLLER INTERNAL REGULATOR
	if(v_ctl_reg.good) uart_send_string_p(PSTR(" - PASS"));
	else uart_send_string_p(PSTR(" - FAIL!!!"));
	convert_and_send(v_in.val, 3.78, V_IN, v_dd);			// INPUT ADAPTER VOLTAGE
	if(v_in.good) uart_send_string_p(PSTR(" - PASS"));
	else uart_send_string_p(PSTR(" - FAIL!!!"));
	convert_and_send(1023, 1.0, V_DD, v_dd);				// CALCULATED VOLTAGE RAIL (4.3V ideallly)
	if(v_dd_good) uart_send_string_p(PSTR(" - PASS"));
	else uart_send_string_p(PSTR(" - FAIL!!!"));

	// store pass/fail results in ROM.
	// The order matters!. It should always be in the foolowing order>
	// 	1. High Voltage boost output voltage
	//	2. Boost controller internal linear regulator voltage
	//	3. Input voltage to the circuit
	//	4. MCU voltage rail
	*p = v_hv.good;
	*(p + 1) = v_ctl_reg.good;
	*(p + 2) = v_in.good;
	*(p + 3) = v_dd_good;
}

/*-----------------------------------------------------------------------------
--------------------- I N T E R N A L   F U N C T I O N S ---------------------
-----------------------------------------------------------------------------*/

/*===========================================================================*/
static void convert_and_send(uint16_t v, float c, float what, float v_dd)
{
	uint8_t integer, decimal;
	char integer_str[5], decimal_str[5];
	float fVoltage, float_part;

	fVoltage = (float) v;
	fVoltage = (fVoltage * v_dd * c) / 1023.0;	// "c" accounts for the resistor divider factor

	integer = (uint8_t)fVoltage;			// Extract integer part
	float_part = fVoltage - (float)integer;	// Extract floating part
	decimal = (uint8_t)(float_part * 100.0);// two decimal digits

	itoa(integer, integer_str, 10);			// convert integer part to string
	itoa(decimal, decimal_str, 10);			// convert decimal part to string

	if(what == V_HV)  uart_send_string_p(PSTR("\r\n- High Voltage: "));
	else if(what == V_CTL_REG) uart_send_string_p(PSTR("\r\n- Boost controller regulator voltage: "));
	else if(what == V_IN) uart_send_string_p(PSTR("\r\n- Input voltage: "));
	else if(what == V_DD) uart_send_string_p(PSTR("\r\n- VDD rail voltage: "));
	uart_send_string(integer_str);
	uart_send_string_p(PSTR(","));
	if(decimal < 10)						// significant digit correction
		uart_send_string_p(PSTR("0"));
	uart_send_string(decimal_str);
}