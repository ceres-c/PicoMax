#ifndef _GLITCH_H
#define _GLITCH_H

#include "pico/stdlib.h"
#include "hardware/pio.h"

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

static inline void glitch_trigger_program_init(PIO pio, uint sm, uint prog_offs,
	bool trig_out_enabled, uint pin_max_sel, uint pin_trig_in, uint pin_trig_out) {
	// TODO allow for glitch inversion with mux

	// NOTE: Setting pin directions and values first to avoid cutting power to the target
	// device by switching the MAX4619 selector

	// Set pin directions (pin_trigger_out and pin_max_sel are outputs, pin_trigger_in is an input)
	pio_sm_set_pindirs_with_mask(
		pio, sm,
		(trig_out_enabled << pin_trig_out) | (1u << pin_max_sel),
		(trig_out_enabled << pin_trig_out) | (1u << pin_max_sel) | (1u << pin_trig_in));

	// Set default pin values (pin_trigger_out is low, pin_max_sel is high)
	pio_sm_set_pins_with_mask(
		pio, sm, 
		(1u << pin_max_sel),
		(1u << pin_trig_out) | (1u << pin_max_sel));

	// Claim pins for PIO
	pio_gpio_init(pio, pin_max_sel);
	pio_gpio_init(pio, pin_trig_in);
	pio_gpio_init(pio, pin_trig_in + 1);
	pio_gpio_init(pio, pin_trig_in + 2);
	pio_gpio_init(pio, pin_trig_out);

	pio_sm_config c = glitch_trigger_program_get_default_config(prog_offs);

	sm_config_set_set_pins(&c, pin_max_sel, 1); 	// Set base pin and number of pins for SET operand
	sm_config_set_in_pins(&c, pin_trig_in); 		// Set input base pin
	sm_config_set_sideset_pins(&c, pin_trig_out);	// Set sideset base pin

	// sm_config_set_clkdiv(&c, clkdiv); // TODO is this useful in any way? float clkdiv

	// Use GPIO pin muxes to invert the output. It's easier to think of the glitch as 0->1->0 rather than 1->0->1.
	// gpio_set_outover(pin_max_sel, GPIO_OVERRIDE_INVERT); // NOTE: this MUST be after the inits

	// hw_set_bits(&pio->input_sync_bypass, 1u << pin_miso); // TODO this disables flip-flops on the input pins for faster inputs, useful?

	// Initialize the state machine
	pio_sm_init(pio, sm, prog_offs, &c);
	pio_sm_set_enabled(pio, sm, true);
}

bool glitch_init(PIO pio, glitch_t *glitch);

#endif // _GLITCH_H