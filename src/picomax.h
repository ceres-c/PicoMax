#ifndef _PICOMAX_H
#define _PICOMAX_H
#include <stdio.h>
#include <stdlib.h> // For malloc
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/pio.h"
#include "hardware/structs/xip_ctrl.h" // To disable cache
#include "hardware/i2c.h"

#include "icsp/icsp.h"
#include "glitch.h"
#include "pins.h"

#define CMD_DELAY			'D' // Accepts 4 bytes (little endian) of delay value
#define CMD_WIDTH			'W' // Accepts 4 bytes (little endian) of pulse width value
#define CMD_GLITCH			'G'
#define CMD_GLITCH_LOOP		'L'	// TODO remove
#define CMD_TRIG_OUT_EN		'O'
#define CMD_TRIG_OUT_DIS	'o'
#define CMD_TRIG_IN_RISING	'I'
#define CMD_TRIG_IN_FALLING 'i'
#define CMD_PING			'P'
#define CMD_POWERON			'+'
#define CMD_POWEROFF		'-'
#define CMD_READ_DATA		'R'
#define CMD_READ_PROG		'r'
#define CMD_WRITE_DATA		'W'
#define CMD_WRITE_PROG		'w'
#define CMD_ERASE_BULK_DATA	'E'
#define CMD_ERASE_BULK_PROG	'e'
#define CMD_ERASE_ROW_PROG	'@'

#define CMD_READ_ATOMIC		'A'

#define RESP_OK				'k'
#define RESP_KO				'x'
#define RESP_PONG			'p'
#define RESP_GLITCH_FAIL	'.'
#define RESP_GLITCH_WEIRD	'y' // Used when deviceID is wrong but nonzero

#define picomax_i2c			i2c0

#define PIC_SLAVE_ADDR		0x8

#endif // _PICOMAX_H
