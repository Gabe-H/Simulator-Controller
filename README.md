# Simulator Controller

For the BHS Flight and Racing Simulator

# Hardware

- Arduino Mega 2560
- At least buttons (stop and start)
- At least 1 emergency stop button
- Indicator light
- 3 ODrives connected to 3 UARTs

## Wiring

Specific pins are defined in `src/pins.h`

### ODrives

- Arduino TX1 -> ODrive 1 RX (GPIO2)
- ~~Arduino RX1 -> ODrive 1 TX (GPIO1)~~ (not yet implemented)
- Arduino TX2 -> ODrive 2 RX (GPIO2)
- ~~Arduino RX1 -> ODrive 1 TX (GPIO1)~~ (not yet implemented)
- Arduino TX3 -> ODrive 3 RX (GPIO2)
- ~~Arduino RX1 -> ODrive 1 TX (GPIO1)~~ (not yet implemented)
- Arduino GND -> All ODrives GND

### Emergency Stop Button(s) (normally open)

- Arduino RST and GND

### Start Button(s) (normally open)

- Arduino START_BUTTON and GND

### Stop Button(s) (normally open)

- Arduino STOP_BUTTON and GND

### Status Light - Use an NPN transistor or relay for 12v light

- Arduino STATUS_PIN and GND

# Software

- VSCode with PlatformIO extension
- [FlyPT Mover](https://www.flyptmover.com/home)
- [PyFly homing script](https://github.com/Gabe-H/PyFly)

## Configurations

### FlyPT Mover

Output: Serial

- Type: Binary
- Baudrate: 115200
- Data bits: 8
- Stop bits: 1
- Parity: None
- Interval loops: 5 (10ms)
- Start string: `<255><255>S`
- Output string: `<255><255>F1<Axis1a>2<Axis2a>3<Axis3a>4<Axis4a>5<Axis5a>6<Axis6a>!`
- Stop string: `<255><255>E`

Rig: Linear Hexapod

- Position bit output: 16 (0 to 65535 in 2 bytes)
- Actuator filerting: `CROP(VALUE; -200, 200)`

### PyFly

- Configure serial numbers in `config.json`. Serial numbers can be
  found when running `odrivetool` and plugging in each ODrive.
