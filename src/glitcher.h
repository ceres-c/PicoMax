#ifndef _GLITCHER_H
#define _GLITCHER_H
#include <stdio.h>
#include <stdlib.h> // For malloc
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
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
#define CMD_POWERON			'+'
#define CMD_POWEROFF		'-'
#define CMD_READ_DATA		'R'
#define CMD_READ_PROG		'r'
#define CMD_WRITE_DATA		'W'
#define CMD_WRITE_PROG		'w'
#define CMD_ERASE_BULK		'E'

#define RESP_OK				'k'
#define RESP_KO				'x'
#define RESP_PONG			'p'
#define RESP_GLITCH_FAIL	'.'

const PIO glitcher_pio = pio0;
const PIO icsp_pio = pio1;

#endif // _GLITCHER_H
