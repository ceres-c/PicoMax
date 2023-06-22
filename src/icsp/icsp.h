#ifndef _ICSP_H
#define _ICSP_H
#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "../pins.h"

#include "icsp_enter.pio.h"
#include "icsp_imperative.pio.h"
#include "icsp_load.pio.h"
#include "icsp_read.pio.h"

#define ICSP_WORD_SIZE				14
#define ICSP_WORD_MASK				((1 << ICSP_WORD_SIZE) - 1)
#define ICSP_BYTES_PER_WORD			2 // ceil(ICSP_WORD_SIZE / 8)
typedef uint16_t icsp_word_t;

#define ICSP_KEY					0b01001101010000110100100001010000 // "MCHP" taken from DS41397B-page 18

#define ICSP_CMD_LOAD_CONFIG		0x00
#define ICSP_CMD_LOAD_PROG_MEM		0x02
#define ICSP_CMD_LOAD_DATA_MEM		0X03
#define ICSP_CMD_READ_PROG_MEM		0x04
#define ICSP_CMD_READ_DATA_MEM		0x05
#define ICSP_CMD_INCREMENT_ADDR		0x06
#define ICSP_CMD_RESET_ADDR			0x16
#define ICSP_CMD_BEGIN_INT_TMR		0x08
#define ICSP_CMD_BEGIN_EXT_TMR		0x18
#define ICSP_CMD_END_EXT_TMR		0x0A
#define ICSP_CMD_BULK_ERASE_PROG	0x09
#define ICSP_CMD_BULK_ERASE_DATA	0x0B
#define ICSP_CMD_ROW_ERASE_PROG		0x11

typedef struct icsp_s {
	PIO pio;
	uint16_t clkdiv;
} icsp_t;

void read_prog_mem(icsp_t *icsp, uint32_t addr, uint32_t size, uint8_t *dst);

void icsp_enter(icsp_t* icsp);
void icsp_imperative(icsp_t* icsp, uint32_t command);
void icsp_load(icsp_t* icsp, uint8_t command, uint16_t data);
uint16_t icsp_read(icsp_t* icsp, uint8_t command);

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
