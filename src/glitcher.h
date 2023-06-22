#ifndef _GLITCHER_H
#define _GLITCHER_H
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"

#include "glitch.pio.h"
#include "icsp/icsp.h"
#include "pins.h"

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

#define GPIO_ATOMIC			((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_OFFSET))
#define SET_GPIO_ATOMIC		((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_SET_OFFSET))
#define CLR_GPIO_ATOMIC		((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_CLR_OFFSET))
#define XOR_GPIO_ATOMIC		((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_XOR_OFFSET))

const PIO glitcher_pio = pio0;
const PIO icsp_pio = pio1;

#endif // _GLITCHER_H
