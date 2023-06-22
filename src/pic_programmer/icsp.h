#ifndef _ICSP_H
#define _ICSP_H
#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "icsp_enter.pio.h"
#include "icsp_imperative.pio.h"
#include "icsp_load.pio.h"
#include "icsp_read.pio.h"

// PIO init inlines
static inline void icsp_enter_program_init(PIO pio, uint sm, uint prog_offs, float clkdiv, uint pin_clock, uint pin_data) {
	// NOTE: Setting pin directions and values first to avoid sending garbage to the target device
	// Set pin directions (pin_clock and pin_data are always outputs)
	pio_sm_set_pindirs_with_mask(
		pio, sm,
		(1u << pin_clock) | (1u << pin_data),
		(1u << pin_clock) | (1u << pin_data));

	// Set default pin values (everything is low)
	pio_sm_set_pins_with_mask(
		pio, sm,
		0,
		(1u << pin_clock) | (1u << pin_data));

	// Claim pins for PIO
	pio_gpio_init(pio, pin_clock);
	pio_gpio_init(pio, pin_data);

	pio_sm_config c = icsp_enter_program_get_default_config(prog_offs);

	sm_config_set_out_pins(&c, pin_data, 1); 		// Set base pin and number of pins for OUT operand
	sm_config_set_sideset_pins(&c, pin_clock);		// Set sideset base pin

	sm_config_set_clkdiv(&c, clkdiv);

	// hw_set_bits(&pio->input_sync_bypass, 1u << pin_miso); // TODO this disables flip-flops on the input pins for faster inputs, useful?

	// Initialize the state machine
	pio_sm_init(pio, sm, prog_offs, &c);
	pio_sm_set_enabled(pio, sm, true);
}

static inline void icsp_imperative_program_init(PIO pio, uint sm, uint prog_offs, float clkdiv, uint pin_clock, uint pin_data) {
	// NOTE: Setting pin directions and values first to avoid sending garbage to the target device
	// Set pin directions (pin_clock is always an output and pin_data is initially an outputs)
	pio_sm_set_pindirs_with_mask(
		pio, sm,
		(1u << pin_clock) | (1u << pin_data),
		(1u << pin_clock) | (1u << pin_data));

	// Set default pin values (everything is low)
	pio_sm_set_pins_with_mask(
		pio, sm,
		0,
		(1u << pin_clock) | (1u << pin_data));

	// Claim pins for PIO
	pio_gpio_init(pio, pin_clock);
	pio_gpio_init(pio, pin_data);

	pio_sm_config c = icsp_imperative_program_get_default_config(prog_offs);

	sm_config_set_set_pins(&c, pin_data, 1); 		// Used to set the pindir in the PIO asm code
	sm_config_set_out_pins(&c, pin_data, 1); 		// Set base pin and number of pins for OUT operand
	sm_config_set_in_pins(&c, pin_data); 			// Set input base pin // TODO no ins
	sm_config_set_sideset_pins(&c, pin_clock);		// Set sideset base pin

	sm_config_set_clkdiv(&c, clkdiv);

	// hw_set_bits(&pio->input_sync_bypass, 1u << pin_miso); // TODO this disables flip-flops on the input pins for faster inputs, useful?

	// Initialize the state machine
	pio_sm_init(pio, sm, prog_offs, &c);
	pio_sm_set_enabled(pio, sm, true);
}

static inline void icsp_load_program_init(PIO pio, uint sm, uint prog_offs, float clkdiv, uint pin_clock, uint pin_data) {
	// NOTE: Setting pin directions and values first to avoid sending garbage to the target device
	// Set pin directions (pin_clock is always an output and pin_data is initially an outputs)
	pio_sm_set_pindirs_with_mask(
		pio, sm,
		(1u << pin_clock) | (1u << pin_data),
		(1u << pin_clock) | (1u << pin_data));

	// Set default pin values (everything is low)
	pio_sm_set_pins_with_mask(
		pio, sm,
		0,
		(1u << pin_clock) | (1u << pin_data));

	// Claim pins for PIO
	pio_gpio_init(pio, pin_clock);
	pio_gpio_init(pio, pin_data);

	pio_sm_config c = icsp_load_program_get_default_config(prog_offs);

	sm_config_set_set_pins(&c, pin_data + 1, 1); 		// TODO remove
	sm_config_set_out_pins(&c, pin_data, 1); 		// Set base pin and number of pins for OUT operand
	sm_config_set_sideset_pins(&c, pin_clock);		// Set sideset base pin

	sm_config_set_clkdiv(&c, clkdiv);

	// hw_set_bits(&pio->input_sync_bypass, 1u << pin_miso); // TODO this disables flip-flops on the input pins for faster inputs, useful?

	// Initialize the state machine
	pio_sm_init(pio, sm, prog_offs, &c);
	pio_sm_set_enabled(pio, sm, true);
}

static inline void icsp_read_program_init(PIO pio, uint sm, uint prog_offs, float clkdiv, uint pin_clock, uint pin_data) {
	// NOTE: Setting pin directions and values first to avoid sending garbage to the target device
	// Set pin directions (pin_clock is always an output and pin_data is initially an outputs)
	pio_sm_set_pindirs_with_mask(
		pio, sm,
		(1u << pin_clock) | (1u << pin_data),
		(1u << pin_clock) | (1u << pin_data));

	// Set default pin values (everything is low)
	pio_sm_set_pins_with_mask(
		pio, sm,
		0,
		(1u << pin_clock) | (1u << pin_data));

	// Claim pins for PIO
	pio_gpio_init(pio, pin_clock);
	pio_gpio_init(pio, pin_data);

	pio_sm_config c = icsp_read_program_get_default_config(prog_offs);

	sm_config_set_set_pins(&c, pin_data, 1); 		// Used to set the pindir in the PIO asm code
	sm_config_set_out_pins(&c, pin_data, 1); 		// Set base pin and number of pins for OUT operand
	sm_config_set_in_pins(&c, pin_data); 			// Set input base pin
	sm_config_set_sideset_pins(&c, pin_clock);		// Set sideset base pin

	sm_config_set_clkdiv(&c, clkdiv);

	// hw_set_bits(&pio->input_sync_bypass, 1u << pin_miso); // TODO this disables flip-flops on the input pins for faster inputs, useful?

	// Initialize the state machine
	pio_sm_init(pio, sm, prog_offs, &c);
	pio_sm_set_enabled(pio, sm, true);
}

#endif // _ICSP_H
