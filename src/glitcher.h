#ifndef _GLITCHER_H
#define _GLITCHER_H
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "glitch.pio.h"

#include "pic_programmer/icsp.h"

// #include "icsp_enter.pio.h"
#include "icsp_imperative.pio.h" // TODO remove all of these
#include "icsp_load.pio.h"
#include "icsp_read.pio.h"

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

#define ICSPCLK				0x12 // pin 24
#define ICSPDAT				0x13 // pin 25
#define nMCLR				0x14 // pin 26
#define nMCLR_MASK			(1 << nMCLR)
#define ICSP_WORD_SIZE		14
#define ICSP_WORD_MASK		((1 << ICSP_WORD_SIZE) - 1)

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

const PIO glitcher_pio = pio0;
const PIO icsp_pio = pio1;

#define PROGRAMMER_CMD_LOAD_CONFIG		0x00
#define PROGRAMMER_CMD_LOAD_DATA_MEM	0X03
#define PROGRAMMER_CMD_READ_PROGMEM		0x04
#define PROGRAMMER_CMD_INCREMENT_ADDR	0x06
#define PROGRAMMER_CMD_RESET_ADDR		0x16

#endif // _GLITCHER_H
