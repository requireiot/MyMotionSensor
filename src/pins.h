#ifndef PINS_H_
#define PINS_H_

#include "stdpins.h"

// PIR motion sensor connected to PD2=INT0
#define MOTION          D,2,ACTIVE_HIGH     // HC-SR505 output, HIGH(TRUE) when motion detected

#define LUX_SIGNAL	    C,0,ACTIVE_HIGH     // to phototransistor collector
#define LUX_POWER	    D,5,ACTIVE_HIGH     // to phototransistor via 10k resistor

#endif // PINS_H