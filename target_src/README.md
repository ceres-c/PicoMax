This folder contains a Microchip MPLAB project for the PIC16F1936 target. It
essentially runs a for loop inside an endless loop. In the for, the counter
is incremented by one until it reaches `0x10`, and another target variable
starting at `0x10` is decremented by one until it reaches `0`.
At the start of the for a pin is raised, then after the for, the pin is set to
low. Additionally, the code will check if the two variables reached their
expected values, and raise two different pins if they did or did not.

## Building
Just download and install MPLAB GUI plus the XC8 compiler. I have no clue how
to build this from the command line, nor I care to learn tbh.

The prebuilt .hex files flashable with pickle or any other PIC programmer can
be found in
`target_src/pic16_c_template_1.X/dist/HT_PIC16F1936/production/`

## CPU Configuration
- Brownout reset: enabled
- Clock speed: 16MHz

## PIC Port configuration
```
RA0: High during loop execution
RA1: High after loop if glitch occurred
RA2: High after loop if no glitch
```
![](../img/MPLAB_PIC_pin_config.png)
