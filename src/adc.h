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

#include <stdint.h>

/******************************************************************************
******************* C O N S T A N T   D E F I N I T I O N S *******************
******************************************************************************/

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