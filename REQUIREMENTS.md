__MyMotionSensor__
---

Notation:
- [x] requirement is implemented and tested. 
- [ ] requirement is not implemented or tested, ideas for future versions.

## Objectives

* simple home automation node for detecting motion in a hallway or room, to control lighting

## Requirements
### System requirements
- [x] `R01.01` Lights turn on automatically, e.g. in a hallway, when someone walks through the hallway
- [x] `R01.02` Lights turn off automatically after a configurable time, e.g. 1 min.
- [x] `R01.03` Lights turn on automatically only if the location is in the dark, i.e. at night or on very overcast days

### Node requirements, nonfunctional
- [x] `R02.01` Low cost, <10€ per node
- [x] `R02.02` unobtrusive appearance
- [x] `R02.03` node is battery powered, requires battery change less than once a year 
- [x] `R02.04` node integrates with existing home automation infrastructure (MySensors 2.4 GHz RF gateway, openHAB controller)

### Node requirements, functional
- [x] `R03.01` when the PIR sensor detects motion, then the node shall measure and report ambient brightness, in percent of full range
- [x] `R03.02` when the PIR sensor detects motion, then the node shall report that to the HA controller
- [x] `R03.03` when the PIR sensor becomes inactive again, the node shall report that
- [x] `R03.04` every 30min, the node shall measure and report ambient brightness in percent of full scale
- [x] `R03.05` every 12 hours, the node shall measure battery voltage, and report in milllivolts, and in percent of usable battery voltage range
- [x] `R03.06` when the node is powered on, it shall report project name and revision number, as expected by [MySensorsTracker](https://github.com/requireiot/MySensorsTracker)