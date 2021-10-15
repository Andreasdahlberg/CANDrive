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

## Usage
### Debug Console
CANDrive incorporates a simple debug console used for traces and debug commands.
The debug commands are not intended for controlling the motors during normal usage
since they bypass several of the safety features.

The debug console is available at JP1 and disabled by default. It can be enabled by moving jumper J1.

#### Available commands
##### rpm
rpm \[MOTOR_INDEX\] \[RPM\]

Ex.
```
> rpm 0 50
```

##### current
current \[MOTOR_INDEX\] \[CURRENT\]

Ex.
```
> current 1 1500
```

##### run
run \[MOTOR_INDEX\]

Ex.
```
> run 0
```

##### coast
coast \[MOTOR_INDEX\]

Ex.
```
> coast 0
```

##### brake
brake \[MOTOR_INDEX\]

Ex.
```
> brake 0
```

##### reset
reset

Ex.
```
> reset
```

##### level
level \[MODULE\] \[LEVEL\]

Ex.
```
> level App 10
```

## Hardware
[BOM](https://octopart.com/bom-tool/TDnAzRKf)
