#ifndef _PINS_H
#define _PINS_H

// If you need to rewire the board, change here

// Glitcher-specific
#define MAX_EN_PIN				0x02 // pin 4
#define MAX_EN_MASK				(1 << MAX_EN_PIN)
#define MAX_SEL_PIN				0x03 // pin 5
#define MAX_SEL_MASK			(1 << MAX_SEL_PIN)
#define TRIG_IN_PIN				0x04 // pin 6
#define TRIG_IN_MASK			(1 << TRIG_IN_PIN)
#define TRIG_OUT_PIN			0x05 // pin 7
#define TRIG_OUT_MASK			(1 << TRIG_OUT_PIN)

// PIC firmware-related
#define PIC_START_READ_PIN		0x06 // pin 9
#define START_READ_MASK			(1 << PIC_START_READ_PIN)
#define I2C_SDA_PIN				0x10 // pin 21 (opposite of 1)
#define I2C_SCL_PIN				0x11 // pin 22

// PIC ICSP programming-related
#define ICSPCLK					0x12 // pin 24
#define ICSPDAT					0x13 // pin 25
#define nMCLR					0x14 // pin 26
#define nMCLR_MASK				(1 << nMCLR)

// Registers for the SIO
#define GPIO_ATOMIC				((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_OFFSET))
#define SET_GPIO_ATOMIC			((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_SET_OFFSET))
#define CLR_GPIO_ATOMIC			((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_CLR_OFFSET))
#define XOR_GPIO_ATOMIC			((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_XOR_OFFSET))

#endif // _PINS_H
