/**
 * @file 		  Basics.cpp
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
 * @brief Basic initializaton tasks that need to be done in most sensor projects.
 * 
 */


#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/boot.h>
#include "stdpins.h"
#include "debugstream.h"


#if     ((F_CPU /  2L) < 200000uL)
 #define ADC_PRESCALER 1
#elif ((F_CPU /  4L) < 200000uL)
 #define ADC_PRESCALER 2
#elif ((F_CPU /  8L) < 200000uL)
 #define ADC_PRESCALER 3
#elif ((F_CPU / 16L) < 200000uL)
 #define ADC_PRESCALER 4
#elif ((F_CPU / 32L) < 200000uL)
 #define ADC_PRESCALER 5
#elif ((F_CPU / 64L) < 200000uL)
 #define ADC_PRESCALER 6
#else 
 #define ADC_PRESCALER 7
#endif

/**
 * @brief Basic initialization of peripherals, for minimum power consumption.
 * Call this from setup(), or from preHwInit(), which is called from MySensors framework
 * 
 */
void basicHwInit()
{
#ifdef SOFT_1MHZ
  clock_prescale_set(clock_div_8);
#endif

	// first disable ADC, then turn off in PRR
	ADCSRA 	= (ADC_PRESCALER << ADPS0)	// prescaler
			| (0 << ADIE)	// no interrupts
			| (0 << ADATE)	// no auto trigger enable
			| (0 << ADEN)	// disable ADC for now
			;
	PRR = _BV(PRADC) | _BV(PRTWI) | _BV(PRTIM1) | _BV(PRTIM2);

	// disable analog comparator
	ACSR |= _BV(ACD);

	// set direction and pull-ups, this is specific for ATmega328
	DDRB &= ~0b11000001;        // port B: PB1-5 used by SPI; PB6,7 are Xtal
	PORTB |= 0b00000001;        // enable pull-up

	DDRC = 0;                   // port C all input (default after reset anyway)
	PORTC = 0xFF;               // enable pull-up on all bits to save power ...
#ifdef REPORT_CLIMATE
    PULLUP_DISABLE(_I2C_SCL);   // ... except no pull-up on SDA,SCL
    PULLUP_DISABLE(_I2C_SDA);	
#endif

	DDRD = 0;                   // port D all input (default after reset anyway)
	PORTD = 0xFF;               // enable pull-up on all bits to save power ...
    PULLUP_DISABLE(_UART_RX);   // ... except no pull-up on RXD, TXD
    PULLUP_DISABLE(_UART_TX);   

#ifdef SOFT_1MHZ
	/*
	  Fuses are set for internal 8 MHz RC oscillator, no divide-by-8
	  bootloader operates at 8 MHz, 57600 Baud
	  F_CPU is set to 1'000'000 
      at start of application, enable divide-by-8
	*/
	clock_prescale_set(clock_div_8);
#endif
}


/**
 * @brief Basic things to do in setup().
 * 
 */
void basicSetup()
{
	#ifdef SOFT_1MHZ
	  DEBUG_PRINT("* Soft 1 MHz\r\n");
	#endif

	DEBUG_PRINTF("* Fuses: L=%02X H=%02X E=%02X\r\n",
		boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS),
		boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS),
		boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS)
	);
}
