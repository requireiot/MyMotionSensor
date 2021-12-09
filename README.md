# MyMotionSensor
MySensors-based motion detector and ambient light sensor
![Overview](/pictures/MyMotionSensor%20Closed.jpg)

This is part of my home automation setup. For details, see my [blog](https://requireiot.com/my-home-automation-story-part-1/).

### Firmware
The software reports the ambient brightness level at regular intervals (30 min), and whenever motion is detected. 

The software reports when the PIR motion sensor becomes active, and when it goes back to inactive, after a delay. The PIR motion sensor is connected to an interrupt input, for immediate response to motion. The PIR logic output is also polled at regular intervals, during sleep, to report when it goes back to inactive.

The software reports its project name and revision number when it starts, and it reports battery voltage at regular intervals. This is the convention I use with my [MySensorsTracker](https://github.com/requireiot/MySensorsTracker) monitoring app.

### Dependencies

In addition to the standard Arduino and MySensors libraries, this project needs
- my [stdpins](https://github.com/requireiot/stdpins) library for AVR pin manipulation and interrupt control
- my [debugstream](https://github.com/requireiot/debugstream) library for sensing debug messages to serial output, only if a terminal or FTDI module is connected
- my [AvrBattery](https://github.com/requireiot/AvrBattery) library for measuring the ATmega supply voltage without external components
- my [MySnooze](https://github.com/requireiot/MySnooze) library, which extends the standard MySensors `sleep()` function with the ability to perform simple actions "during" sleep, at 8s intervals 

### Hardware
Built using an Arduino Pro Mini clone and an ABS project box from Aliexpress, and a piece of perf board:
![Open](pictures/MyMotionSensor%20Open.jpg)

### Battery life
The sensors run well over 18 months on a pair of AA batteries. Here is is a plot of battery voltage over a year, for all MyMotionSensor nodes reporting to my openHAB setup:
![Power consumption](/pictures/VCC.png)

### Power saving tricks

I followed the recommendations for battery powered sensors from the [MySensors](https://www.mysensors.org/build/battery) homepage, including

- use a 3.3V Arduino Pro Mini clone
- remove the power LED and voltage regulator from the module
- power the device directly from 2 AA size batteries

In addition,
- the node sleeps most of the time, only waking up when a pin change interrupt occurs, or every 30min to check if brightness or battery level need to be reported. Simple polling of the motion sensor output is also performed every 8s, "during sleep", using my [MySnooze](https://github.com/requireiot/MySnooze) library
- the processor fuses are programmed for internal oscillator 8 MHz, so firmware downloads can be done quickly, at 57.6 kBaud. However, at startup, the firmware configures the processor for 1 MHz clock, to save power while awake.
- debug messages to the serial port are suppressed when the module is not actually connected to a terminal or FTDI USB-to-serial converter. This is to allow the processor to return to sleep as quickly as possible, not having to wait until a debug message has been sent.
- the brightness sensor is only powered as long as a measurement is ongoing.

### Lessons learned
The PIR sensor appears to be sensitive to power supply fluctuations. In the first prototype, it would often retrigger immediately after its output returned to low (inactive), presumably because sending a message via the NRF24L01+ module would draw currrent from the battery, and lower supply voltage. Therefore, the software ignores transitions of the PIR sensor output for a few seconds after going to sleep.

For more details about this project, see my [blog](https://requireiot.com/mymotionsensor/).