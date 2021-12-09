/**
 * @file          MyMotionSensor.cpp
 *
 * @author		  Bernd Waldmann
 * Created		: 15-Apr-2017
 * Tabsize		: 4
 * 
 * This Revision: $Id: MyMotionSensor.cpp 1304 2021-12-09 08:46:20Z  $
 */

/*
   Copyright (C) 2017,2021 Bernd Waldmann

   This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
   If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/

   SPDX-License-Identifier: MPL-2.0
*/

/*
 * Relies on MySensors, Created by Henrik Ekblad <henrik.ekblad@mysensors.org>.
 * Note that the MySensors library is under GPL, so 
 * - if you want to combine this source code file with the MySensors library 
 *   and redistribute it, then read the GPL to find out what is allowed.
 * - if you want to combine this file with the MySensors library and only 
 *   use it at home, then read the GPL to find out what is allowed.
 * - if you want to take just this source code, learn from it, modify it, and 
 *   redistribute it, independent from MySensors, then you have to abide by 
 *   my license rules (MPL 2.0), which imho are less demanding than the GPL.
 */

// standard headers
#include <avr/boot.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>

// Arduino libraries
#include <Arduino.h>
#include "mysensors_conf.h"
#include <MySensors.h>

// my libraries
#include <MySnooze.h>
#include <AvrBattery.h>
#define IMPLEMENT_DEBUGSTREAM_ARDUINO
#include <debugstream.h>

// project-specific headers
#include "pins.h"
#include "LuxMeter.h"
#include "Basics.h"

//=============================================================================
#pragma region Constants

//----- timing ----------------------------------------------------------------

#define SECONDS		* 1000uL
#define MINUTES 	* 60uL SECONDS
#define HOURS 		* 60uL MINUTES
#define DAYS		* 24uL HOURS

// time between battery status reports every 1h
const unsigned long BATTERY_REPORT_INTERVAL = 12 HOURS;			// report battery level every 12h
const unsigned long LIGHT_REPORT_INTERVAL   = 30 MINUTES; 		// report light level every 30min
const unsigned long SLEEP_DUR_SHORT 		= 1 MINUTES;		// sleep during motion: 1 min
const unsigned long SLEEP_DUR_LONG			= 30 MINUTES;		// sleep when no motion: 30min

//----- IDs and Messages ------------------------------------------------------

#define SENSOR_ID_MOTION 	21   	// motion sensor

#define TRIPPED 1
#define UNTRIPPED 0

// tick() is called every TICK_PERIOD seconds or less
const uint8_t TICK_PERIOD = 8;

//-----------------------------------------------------------------------------
#pragma endregion
//=============================================================================
#pragma region global variables 

MyMessage msgMotion( SENSOR_ID_MOTION, V_TRIPPED );

/// last reported state of motion detector
bool reportedTripped = false;

/// last known state of motion detector
volatile bool nowTripped = false;

/// count how often tick() is called during sleep (only for debugging)
uint16_t nTicks=0;

/// after start of sleep, ignore input pin changes for this many seconds
uint8_t ignore_nsecs=0;

//-----------------------------------------------------------------------------
#pragma endregion
//=============================================================================
#pragma region Local functions

/*
	For a few seconds after a message has been sent, detection of motion detector 
	signal transition is blocked ("blackout period"), because the VCC fluctuation 
	causwed by RF send may trigger a false alarm.

	Normally, a transition to "motion detector active" will be caught by the 
	pin change interrupt here. If the transition occurs in the blackout period,
	then it will be detected later, in the every-8-seconds tick() routine
*/

PCI_ISR(MOTION)
{
	// do nothing if we are still ignoring pin changes
	if (ignore_nsecs==0) {
		// if motion started, we need to end sleep now!
		nowTripped = IS_TRUE(MOTION);
		if (!reportedTripped && nowTripped) {
			wokeUpWhy |= 1;
		}
	}
}

//----------------------------------------------------------------------------

/**
 * @brief called by MySnooze::snooze() every 8 seconds or so
 * 
 * @return int8_t  != 0 if sleep should end now
 */
int8_t tick()
{
	nTicks++;

	if (ignore_nsecs) {
		if (ignore_nsecs < TICK_PERIOD)
			ignore_nsecs = 0;
		else
			ignore_nsecs -= TICK_PERIOD;
		return 0;
	}

	nowTripped = IS_TRUE(MOTION);
	if (!reportedTripped && nowTripped) {
		return 2;	// if motion started, we need to end sleep now!
	} else if (reportedTripped && !nowTripped) {
		return 4;
	} else {
		return 0;
	}
}

//-----------------------------------------------------------------------------
#pragma endregion
//===========================================================================
#pragma region ----- battery stuff

#define SENSOR_ID_VCC 		99 		// battery voltage

MyMessage msgVCC( SENSOR_ID_VCC, V_VOLTAGE );


static inline
void presentBattery()
{
	present(SENSOR_ID_VCC, S_MULTIMETER, "VCC [mV]");
}


/**
 * @brief Send MySensors messages with battery level [%] and batery voltage [mV]
 * 
 */
void reportBatteryVoltage()
{
	uint16_t batteryVoltage = AvrBattery::measureVCC();
	send(msgVCC.set(batteryVoltage));
	uint8_t percent = AvrBattery::calcVCC_Percent(batteryVoltage);
	DEBUG_PRINTF("Bat: %u mV = %d%%\r\n", batteryVoltage, percent);
	sendBatteryLevel(percent);
}

//-----------------------------------------------------------------------------
#pragma endregion
//===========================================================================
#pragma region ----- light sensor

#define SENSOR_ID_LIGHT	61		// light sensor in %

MyMessage msgLux( SENSOR_ID_LIGHT, V_LIGHT_LEVEL );


static inline
void presentLux()
{
	// Register sensors to gw
	//                                    	 1...5...10...15...20...25 max payload
	//                                    	 |   |    |    |    |    |
	present(SENSOR_ID_LIGHT, S_LIGHT_LEVEL, "Light [%]");
}


static
void reportLux()
{
    uint16_t u = measureLux();
    send(msgLux.set(u));

}

//---------------------------------------------------------------------------
#pragma endregion
//=============================================================================
#pragma region ----- MySensor framework functions

void presentation()
{
    static char rev[] = "$Rev: 1304 $";
    char* p = strchr(rev+6,'$');
    if (p) *p=0;

	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("MyMotionSensor", rev+6);

	// Register all sensors to gw (they will be created as child devices)
	present(SENSOR_ID_MOTION, S_MOTION, "Motion");
    presentBattery();
    presentLux();
}

//----------------------------------------------------------------------------

/**
 * @brief Initialize hardware, called early by MySensors framework
 * 
 */
void preHwInit()
{
    basicHwInit();
    initLux();

	AS_INPUT_FLOAT(MOTION);
	PCI_ENABLE(MOTION);
	PCIEx_DISABLE(MOTION);
}

//-----------------------------------------------------------------------------
#pragma endregion
//=============================================================================
#pragma region -----Arduino framework functions

void setup()
{
    basicSetup();
	reportBatteryVoltage();
    reportLux();
}

//---------------------------------------------------------------------------

/*
 at the end of sleep, the VCC fluctuation is enough to trigger the motion sensor.
 Therefore, the last sampled state of the input immediately before returning
 from sleep is used

 The sequence of events should be as follows:
 1. motion happens, PIR sets MOTION=H
 2. PCI-ISR fires, detects nowTripped==true, signals "end sleep!"
 3. sleep ends, loop() continues, sends RF message TRIPPED
 4. start new sleep for 1 minute
 5. all pin changes are ignored (they may be triggered by VCC fluctuation)
    ... so if MOTION return to L in first 8 seconds of sleep, it is ignored
 6. if MOTION returns to L after more than 8 seconds, PCI-ISR fires, updates nowTripped=false
 7. at end of 1 min sleep, loop() resumes, sends RF message NOT_TRIPPED
 8. start new sleep for 30 minutes, ignore all PCI during first 8 seconds
*/

void loop()
{
	static unsigned long t_startWake=0, t_endWake=0;
	static unsigned long sleep_dur = SLEEP_DUR_SHORT;
	//static bool sent_message = true;

	unsigned long wake_dur;
	int8_t why;

	Serial.flush();
	// prepare for sleep
	nTicks = 0;
	ignore_nsecs = 30;	// ignore 1st n sec

	// calculate how long we were awake this time, report after next sleep cycle
	t_endWake = millis();
	wake_dur = t_endWake - t_startWake;

	// remember, then clear old interrupts
	PCIEx_DISABLE(MOTION);
	wokeUpWhy = 0;
	PCIFx_CLEAR(MOTION);
	PCIEx_ENABLE(MOTION);

	why = snooze(sleep_dur);	// Zzzzzzzzzzzzzzzzzzzzzzzzzzz

	PCIEx_DISABLE(MOTION);

	t_startWake = millis();

	DEBUG_PRINTF("* %5ld ms, %3u t, why:%d Tr:%d\r\n",wake_dur, nTicks, why, nowTripped ? 1 : 0 );

	unsigned long t_now = millis();

	// every 30min and when triggered, report light
	static unsigned long t_last_light_report=0;
	if (
		(nowTripped && !reportedTripped) ||
		((unsigned long)(t_now - t_last_light_report) >= LIGHT_REPORT_INTERVAL)
		) {
		t_last_light_report = t_now;
        reportLux();
	}

	if (
		(why==MY_WAKE_UP_BY_TIMER)
		|| (nowTripped && !reportedTripped)
		|| (!nowTripped && reportedTripped)
		) {
		send(msgMotion.set(nowTripped ? TRIPPED : UNTRIPPED));  // Send tripped value to gw
		reportedTripped = nowTripped;
		sleep_dur = reportedTripped ? SLEEP_DUR_SHORT : SLEEP_DUR_LONG;
	}

	// once a day or so, report battery status
	static unsigned long t_last_battery_report=0;
	if ((unsigned long)(t_now - t_last_battery_report) >= BATTERY_REPORT_INTERVAL) {
			t_last_battery_report = t_now;
			reportBatteryVoltage();
	}
}

//-----------------------------------------------------------------------------
#pragma endregion
