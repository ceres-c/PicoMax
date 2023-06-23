#ifndef _GLITCH_H
#define _GLITCH_H

#include "pico/stdlib.h"

#include "glitch.pio.h"
#include "pins.h"

typedef struct glitch_s {
	PIO pio;
	uint prog_offs;
	uint sm;
} glitch_t;

// All these function are in RAM to be fast.
// Due to Pi Pico's slow flash, first few executions after cache evict take far longer
void __no_inline_not_in_flash_func(glitch_power_on)();
void __no_inline_not_in_flash_func(glitch_power_off)();
uint8_t __no_inline_not_in_flash_func(do_glitch)(glitch_t *glitch, uint32_t delay, uint32_t pulse_width, bool trig_out);

#endif // _GLITCH_H