#include "icsp.h"
#include <stdio.h> // For printf TODO remove

void read_prog_mem(icsp_t *icsp, uint32_t addr, uint32_t size, uint8_t *dst) {
	icsp_enter(icsp);

	// sleep_us(1000); // TODO is this needed? In pickle they had it but my PIC seems to work without it
	// uint32_t data = icsp_read(icsp, ICSP_CMD_LOAD_CONFIG);
	// printf("Load config: %x\n", data);

	icsp_imperative(icsp, ICSP_CMD_RESET_ADDR); // Reset to 0

	for (int i = 0; i < addr; i++)
		icsp_imperative(icsp, ICSP_CMD_INCREMENT_ADDR);
	for (int i = 0; i < size; i++) {
		*((icsp_word_t*)(dst + i * ICSP_BYTES_PER_WORD)) = icsp_read(icsp, ICSP_CMD_READ_PROG_MEM);
		// printf("Read: %x\n", *((icsp_word_t*)(dst + i * ICSP_BYTES_PER_WORD)));
		icsp_imperative(icsp, ICSP_CMD_INCREMENT_ADDR);
	}
}

// ICSP command handlers
void icsp_enter(icsp_t* icsp) {
	uint icsp_enter_program_offset = pio_add_program(icsp->pio, &icsp_enter_program);
	uint icsp_enter_sm	= 0;

	pio_sm_drain_tx_fifo(icsp->pio, icsp_enter_sm);
	pio_sm_clear_fifos(icsp->pio, icsp_enter_sm);

	icsp_enter_program_init(icsp->pio, icsp_enter_sm, icsp_enter_program_offset, icsp->clkdiv, ICSPCLK, ICSPDAT);

	pio_sm_put_blocking(icsp->pio, icsp_enter_sm, ICSP_KEY);
	pio_sm_get_blocking(icsp->pio, icsp_enter_sm); // Discard returned value, but make this function blocking

	pio_sm_set_enabled(icsp->pio, icsp_enter_sm, false);
	pio_remove_program(icsp->pio, &icsp_enter_program, icsp_enter_program_offset);

	pio_sm_clear_fifos(icsp->pio, icsp_enter_sm);
}

void icsp_imperative(icsp_t* icsp, uint32_t command) { // TODO change to void
	uint icsp_imperative_program_offset = pio_add_program(icsp->pio, &icsp_imperative_program);
	uint icsp_imperative_sm = 0;

	pio_sm_drain_tx_fifo(icsp->pio, icsp_imperative_sm);
	pio_sm_clear_fifos(icsp->pio, icsp_imperative_sm);

	icsp_imperative_program_init(icsp->pio, icsp_imperative_sm, icsp_imperative_program_offset, icsp->clkdiv, ICSPCLK, ICSPDAT);

	pio_sm_put_blocking(icsp->pio, icsp_imperative_sm, command);
	pio_sm_get_blocking(icsp->pio, icsp_imperative_sm); // Discard returned value, but make this function blocking

	pio_sm_set_enabled(icsp->pio, icsp_imperative_sm, false);
	pio_remove_program(icsp->pio, &icsp_imperative_program, icsp_imperative_program_offset);

	pio_sm_clear_fifos(icsp->pio, icsp_imperative_sm);
}

void icsp_load(icsp_t* icsp, uint8_t command, uint16_t data) {

	uint32_t payload = 0;
	payload |= data << (6 + 1); // 6 bits of command + 1 data start bit (0)
	payload |= command;

	uint icsp_load_program_offset = pio_add_program(icsp->pio, &icsp_load_program);
	uint icsp_load_sm = 0;

	pio_sm_drain_tx_fifo(icsp->pio, icsp_load_sm);
	pio_sm_clear_fifos(icsp->pio, icsp_load_sm);

	icsp_load_program_init(icsp->pio, icsp_load_sm, icsp_load_program_offset, icsp->clkdiv, ICSPCLK, ICSPDAT);

	pio_sm_put_blocking(icsp->pio, icsp_load_sm, payload);
	pio_sm_get_blocking(icsp->pio, icsp_load_sm); // Discard returned value, but make this function blocking

	pio_sm_set_enabled(icsp->pio, icsp_load_sm, false);
	pio_remove_program(icsp->pio, &icsp_load_program, icsp_load_program_offset);

	pio_sm_clear_fifos(icsp->pio, icsp_load_sm);
}

icsp_word_t icsp_read(icsp_t* icsp, uint8_t command) {

	uint icsp_read_program_offset = pio_add_program(icsp->pio, &icsp_read_program);
	uint icsp_read_sm = 0;

	pio_sm_drain_tx_fifo(icsp->pio, icsp_read_sm);
	pio_sm_clear_fifos(icsp->pio, icsp_read_sm);

	icsp_read_program_init(icsp->pio, icsp_read_sm, icsp_read_program_offset, icsp->clkdiv, ICSPCLK, ICSPDAT);

	pio_sm_put_blocking(icsp->pio, icsp_read_sm, command);
	uint32_t ret = pio_sm_get_blocking(icsp->pio, icsp_read_sm);

	pio_sm_set_enabled(icsp->pio, icsp_read_sm, false);
	pio_remove_program(icsp->pio, &icsp_read_program, icsp_read_program_offset);

	pio_sm_clear_fifos(icsp->pio, icsp_read_sm);

	ret >>= 17; // Take upper 16 bits and remove trailing stop bit
	ret &= ICSP_WORD_MASK; // Remove spurious last high bit
	// NOTE: technically stop bit should be a 0, but we are missing some pull downs, I guess

	return (uint16_t)ret;
}
