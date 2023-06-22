#ifndef _PINS_H
#define _PINS_H

#define ICSPCLK				0x12 // pin 24
#define ICSPDAT				0x13 // pin 25
#define nMCLR				0x14 // pin 26
#define nMCLR_MASK			(1 << nMCLR)

#define MAX_EN_PIN			0x02 // Rpi pico pin 4
#define MAX_EN_MASK			(1 << MAX_EN_PIN)
#define MAX_SEL_PIN			0x03 // pin 5
#define MAX_SEL_MASK		(1 << MAX_SEL_PIN)
#define TRIG_IN_PIN			0x04 // pin 6
#define TRIG_IN_MASK		(1 << TRIG_IN_PIN)
#define TRIG_OUT_PIN		0x05 // pin 7
#define TRIG_OUT_MASK		(1 << TRIG_OUT_PIN)

#define PIC_OUT_PIN			0x07 // pin 10 // TODO maybe this can be 0x06?
#define PIC_OUT_MASK		(1 << PIC_OUT_PIN)
#define PIC_GLITCH_SUCC_PIN	0x08 // pin 11 - These MUST be consecutive for the PIO to work
#define PIC_GLITCH_SUCC_MASK	(1 << PIC_GLITCH_SUCC_PIN)
#define PIC_GLITCH_FAIL_PIN	0x09 // pin 12 - These MUST be consecutive for the PIO to work
#define PIC_GLITCH_FAIL_MASK	(1 << PIC_GLITCH_FAIL_PIN)

#endif // _PINS_H
