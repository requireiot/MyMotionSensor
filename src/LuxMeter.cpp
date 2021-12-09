/**
 * @file 		  LuxMeter.cpp
 *
 * Project		: Home automation
 * Author		: Bernd Waldmann
 * Created		: 09-Jun-2021
 * Tabsize		: 4
 * 
 * This Revision: $Id: $
 */

/*
   Copyright (C) 2021 Bernd Waldmann

   This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
   If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/

   SPDX-License-Identifier: MPL-2.0
*/

/**
 * @brief Light level measurement via ADC, and a BPW40 photo transistor.
 * 
 * Connect BPW40 between ADC input and GND, and 10k reistor between same ADC
 * input and a digital output
 * 
 */

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include "stdpins.h"

#include "pins.h"


/**
 * @brief Initialize hardware for light level measurement. 
 * Call this from setup().
 * 
 */
void initLux()
{
	DIDR0 |= BV(LUX_SIGNAL);
	AS_OUTPUT(LUX_POWER);
	NEGATE(LUX_POWER);
}


/**
 * @brief Measure light level
 * For low power use, this will 1. enable ADC, 2. make a measurement, 3. disable ADC
 * 
 * @return uint16_t  Light level in %, 0=dark 100=bright.
 */
uint16_t measureLux()
{
	uint16_t result, resV;

	ASSERT(LUX_POWER);	// power to phototransistor on
	PRR &= ~_BV(PRADC);
	ADCSRA |= _BV(ADEN);

	uint8_t channel = portBIT(LUX_SIGNAL);
    // Measure Vin against AVCC
    ADMUX 	= (1 << REFS0) 	// ref 1 = AVCC
            | (channel << MUX0)	// channel 0 = ADC0
            ;
    // Vref settle
    //_delay_us(50);

	// Do conversion and ignore
	ADCSRA |= _BV(ADSC);
	while (ADCSRA & _BV(ADSC)) {};
	result = ADC;

	// Do conversion
	ADCSRA |= _BV(ADSC);
	while (ADCSRA & _BV(ADSC)) {};

	// return Vcc in %
	result = (ADC * 100L) / 1023L; // in % of VCC
	NEGATE(LUX_POWER);	// power to phototransistor off

	ADCSRA = 0;
	PRR |= _BV(PRADC);

	return 100-result; // 0%==dark, 100%==bright
}
