/******************************************************************************
 * Copyright (C) 2018 by Nuvitron - Jose Logreira
 *
 * This software is subject to the terms of the GNU General Public License. 
 * Upon copy, modification or redistribution, users are required to maintain 
 * this copyright. See the GNU General Public License for more details.
 *
 *****************************************************************************/
/**
 * @file ADC.h
 * @brief ADC peripheral driver header
 *
 * ADC peripheral settings and related functions
 *
 * @author Jose Logreira
 * @date 25.04.2018
 *
 */

#ifndef ADC_H
#define ADC_H

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "config.h"
#include <stdint.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

#define	ADC_PS_DIV8			(1<<ADPS1 | 1<<ADPS0)
#define	ADC_PS_DIV16		(1<<ADPS2)
#define	ADC_PS_DIV128		(1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0)
#define ADC_MUX_MASK		(1<<MUX4 | 1<<MUX3 | 1<<MUX2 | 1<<MUX1 | 1<<MUX0)

#define ADC_V_HV			0										/* ADC0 */
#define ADC_V_CTL_REG		(1<<MUX0)								/* ADC1 */
#define ADC_V_IN			(1<<MUX1)								/* ADC2 */
#define ADC_V_TST			(1<<MUX1 | 1<<MUX0)						/* ADC3 */
#define ADC_V_REF 			(1<<MUX4 | 1<<MUX3 | 1<<MUX2 | 1<<MUX1)	/* V_REF */

// ADC "oversampling", i.e. # of ADC readings per channel
#define ADC_READ_N		5

#define BOOST_ON 		TRUE
#define BOOST_OFF		FALSE

/******************************************************************************
******************** F U N C T I O N   P R O T O T Y P E S ********************
******************************************************************************/

void adc_init(void);

void adc_set(uint8_t state);

uint16_t adc_read(uint8_t adcx);

uint8_t adc_voltages_test(void);

uint8_t adc_factory_test_check(void);

void adc_factory_voltages_test(uint8_t boost, uint8_t *p);

#endif /* ADC_H */