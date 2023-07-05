#ifndef _PINS_H
#define _PINS_H

// If you need to rewire the board, change here

#define UART_TX_PIN				0x00 // Rpi pico pin 1
#define UART_RX_PIN				0x01 // pin 2

// Glitcher-specific
#define MAX_EN_PIN				0x02 // pin 4
#define MAX_EN_MASK				(1 << MAX_EN_PIN)
#define MAX_SEL_PIN				0x03 // pin 5
#define MAX_SEL_MASK			(1 << MAX_SEL_PIN)
#define TRIG_IN_PIN				0x04 // pin 6
#define TRIG_IN_MASK			(1 << TRIG_IN_PIN)
#define TRIG_OUT_PIN			0x05 // pin 7
#define TRIG_OUT_MASK			(1 << TRIG_OUT_PIN)

// TODO remove this
#define ICPS_READ_CMD_TRIGGER_PIN	0x0A
#define ICPS_READ_CMD_TRIGGER_PIN_MASK (1 << ICPS_READ_CMD_TRIGGER_PIN)

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
