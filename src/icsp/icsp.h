#ifndef _ICSP_H
#define _ICSP_H
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "../pins.h"

#include "icsp.pio.h"

// All the characteristics (word size), commands and timings are relative to the PIC16F1936 chip
// and taken from the Memory Programming Specification DS41397B
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
#define ICSP_CMD_BEGIN_INT_TIMED	0x08
#define ICSP_CMD_BEGIN_EXT_TIMED	0x18
#define ICSP_CMD_END_EXT_TIMED		0x0A
#define ICSP_CMD_BULK_ERASE_PROG	0x09
#define ICSP_CMD_BULK_ERASE_DATA	0x0B
#define ICSP_CMD_ROW_ERASE_PROG		0x11

#define ICSP_CONFIG_MEM_ADDR		0x8000

// All times are in microseconds
#define ICSP_TENTS					1 // This should be 100 nanoseconds, but this is easier to use and won't harm (TM)
#define ICSP_TENTH					250
#define ICSP_TPEXT_MIN				1000	// 1ms
#define ICSP_TDIS_MIN				300
#define ICSP_TDIS_TYP				10000	// 10ms (got from pickle code)
#define ICSP_TERAB_MAX				5000	// 5ms

typedef struct icsp_s {
	PIO pio;
	uint prog_offs;
	uint sm;
	uint16_t clkdiv;
} icsp_t;

// Wrappers around common functionality
void read_prog_mem(icsp_t *icsp, uint32_t addr, uint32_t size, uint8_t *dst);
void write_prog_mem(icsp_t *icsp, uint32_t addr, icsp_word_t src);
void bulk_erase_prog(icsp_t *icsp, bool erase_user_ids);
// This function should not be used when glitching because it will reset the PIC
inline void icsp_power_on() {
	*SET_GPIO_ATOMIC = (MAX_EN_MASK | MAX_SEL_MASK | nMCLR_MASK);
	*CLR_GPIO_ATOMIC = MAX_EN_MASK;
	sleep_us(ICSP_TENTS);
	*CLR_GPIO_ATOMIC = nMCLR_MASK; // Clear MCLR to enable programming
	sleep_us(ICSP_TENTH);
}
// This function should not be used when glitching because it will reset the PIC
inline void icsp_power_off() {
	*CLR_GPIO_ATOMIC = nMCLR_MASK;
	*SET_GPIO_ATOMIC = MAX_EN_MASK;
}


// Bare operations
void icsp_enter(icsp_t* icsp);
void icsp_imperative(icsp_t* icsp, uint8_t command);
void icsp_load(icsp_t* icsp, uint8_t command, uint16_t data);
uint16_t icsp_read(icsp_t* icsp, uint8_t command);

// PIO init
// TODO pass icsp_t as argument
// static inline void icsp_program_init(PIO pio, uint sm, uint prog_offs, float clkdiv, uint pin_clock, uint pin_data) {
static inline void icsp_program_init(icsp_t *icsp, uint pin_clock, uint pin_data) {
	// NOTE: Setting pin directions and values first to avoid sending garbage to the target device
	// Set pin directions (pin_clock is always an output and pin_data is initially an outputs)
	pio_sm_set_pindirs_with_mask(
		icsp->pio, icsp->sm,
		(1u << pin_clock) | (1u << pin_data),
		(1u << pin_clock) | (1u << pin_data));

	// Set default pin values (everything is low)
	pio_sm_set_pins_with_mask(
		icsp->pio, icsp->sm,
		0,
		(1u << pin_clock) | (1u << pin_data));

	// Claim pins for PIO
	pio_gpio_init(icsp->pio, pin_clock);
	pio_gpio_init(icsp->pio, pin_data);

	pio_sm_config c = icsp_program_get_default_config(icsp->prog_offs);

	sm_config_set_set_pins(&c, pin_data, 1); 		// Used to set the pindir in the PIO asm code
	sm_config_set_out_pins(&c, pin_data, 1); 		// Set base pin and number of pins for OUT operand
	sm_config_set_in_pins(&c, pin_data); 			// Set input base pin
	sm_config_set_sideset_pins(&c, pin_clock);		// Set sideset base pin

	sm_config_set_clkdiv(&c, icsp->clkdiv);

	// Initialize the state machine
	pio_sm_init(icsp->pio, icsp->sm, icsp->prog_offs, &c);
	pio_sm_set_enabled(icsp->pio, icsp->sm, true);
}

bool icsp_init(icsp_t *icsp, PIO pio);

#endif // _ICSP_H
