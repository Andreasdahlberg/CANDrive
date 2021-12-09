[![CANDrive CI](https://github.com/Andreasdahlberg/CANDrive/actions/workflows/github-actions.yml/badge.svg)](https://github.com/Andreasdahlberg/CANDrive/actions/workflows/github-actions.yml)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=Andreasdahlberg_CANDrive&metric=alert_status)](https://sonarcloud.io/dashboard?id=Andreasdahlberg_CANDrive)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=Andreasdahlberg_CANDrive&metric=coverage)](https://sonarcloud.io/dashboard?id=Andreasdahlberg_CANDrive)

# CANDrive
CANDrive is a brushed DC Motor Controller controlled via a [CAN](https://en.wikipedia.org/wiki/CAN_bus) interface.

## Features

* Velocity control
* Torque control
* CAN interface
* Firmware updates over CAN
* Emergency stop
* 5 A output current
* 12-24 V supply voltage

## Getting started

### Getting the Source

This project is [hosted on GitHub](https://github.com/Andreasdahlberg/CANDrive). You can clone this project directly using this command:

```
git clone --recurse-submodules https://github.com/Andreasdahlberg/CANDrive.git
```

### Building

```
scons build-opencm
scons build
```

### Running Tests

Run all tests with valgrind:
```
scons test
```

Run a single test suite:
```
scons app/motor/test
```

Run a single unit test:
```
scons app/motor/test test_Motor_SetSpeed
```

### Tools

#### Monitor
The monitor tool is a serial terminal which parses serial output from the device and formats the
output to make it easier to read.

##### Features
* Host timestamps
* Color coded debug levels
* Translation from addresses to file and line
* Filtering[Not implemented]

![Example of monitor output](doc/images/monitor.png)

##### Usage
The monitor is started by typing *scons monitor*.
Serial port and baudrate can be configured [here](firmware/config.py) or on the command line, e.g.
*scons monitor SERIAL_PORT=/dev/ttyACM0 BAUD_RATE=921600*

#### Debug Console
CANDrive incorporates a simple debug console used for traces and debug commands.
The debug commands are not intended for controlling the motors during normal usage
since they bypass several of the safety features.

The debug console is available at JP1 and disabled by default. It can be enabled by moving jumper J1.
Type *scons serial* to access the console, this is only a shortcut for minicom at the moment and will
be incorporated into the serial monitor at some point.

##### Available commands
###### rpm
rpm \[MOTOR_INDEX\] \[RPM\]

Ex.
```
> rpm 0 50
```

###### current
current \[MOTOR_INDEX\] \[CURRENT\]

Ex.
```
> current 1 1500
```

###### run
run \[MOTOR_INDEX\]

Ex.
```
> run 0
```

###### coast
coast \[MOTOR_INDEX\]

Ex.
```
> coast 0
```

###### brake
brake \[MOTOR_INDEX\]

Ex.
```
> brake 0
```

###### reset
reset

Ex.
```
> reset
```

###### level
level \[MODULE\] \[LEVEL\]

Ex.
```
> level App 10
```

## Hardware
[BOM](https://octopart.com/bom-tool/TDnAzRKf)
