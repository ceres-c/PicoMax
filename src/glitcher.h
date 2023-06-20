#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "glitch.pio.h"
#include "programmer.pio.h"

#define CMD_DELAY			'D' // Accepts 4 bytes (little endian) of delay value
#define CMD_WIDTH			'W' // Accepts 4 bytes (little endian) of pulse width value
#define CMD_GLITCH_INIT		'g'
#define CMD_GLITCH			'G'
#define CMD_TRIG_OUT_EN		'O'
#define CMD_TRIG_OUT_DIS	'o'
#define CMD_PING			'P'
#define RESP_OK				'k'
#define RESP_KO				'x'
#define RESP_PONG			'p'
#define RESP_GLITCH_FAIL	'.'
#define CMD_POWERON			'+'
#define CMD_POWEROFF		'-'
#define CMD_SENDCMD			'L'

#define PROGRAMMER_CLK		0x12 // pin 24
#define PROGRAMMER_DATA_IN	0x13 // pin 25
#define PROGRAMMER_DATA_OUT 0x14 // pin 26

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

#define GPIO_ATOMIC			((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_OFFSET))
#define SET_GPIO_ATOMIC		((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_SET_OFFSET))
#define CLR_GPIO_ATOMIC		((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_CLR_OFFSET))
#define XOR_GPIO_ATOMIC		((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_XOR_OFFSET))

PIO glitcher_pio = pio0;
PIO programmer_pio = pio1;

typedef enum {
	LoadConfiguration = 0x0,
	IncrementAddres = 0x6,
	ResetAddress = 0x16,
} SendCommand;

#define PROGRAMMER_RECEIVE_CMD	0b1000000	// Set this bit in the command uint32_t to indicate the programmer should switch
											// to receive mode. If the bit is not set, the following 14-bits word will be sent
