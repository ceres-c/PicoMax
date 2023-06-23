#include "icsp.h"
#include <stdio.h> // For printf TODO remove

void read_prog_mem(icsp_t *icsp, uint32_t addr, uint32_t size, uint8_t *dst) {
	icsp_enter(icsp);

	// sleep_us(1000); // TODO is this needed? In pickle they had it but my PIC seems to work without it

	icsp_imperative(icsp, ICSP_CMD_RESET_ADDR); // Reset to 0

	for (int i = 0; i < addr; i++)
		icsp_imperative(icsp, ICSP_CMD_INCREMENT_ADDR);
	for (int i = 0; i < size; i++) {
		*((icsp_word_t*)(dst + i * ICSP_BYTES_PER_WORD)) = icsp_read(icsp, ICSP_CMD_READ_PROG_MEM);
		icsp_imperative(icsp, ICSP_CMD_INCREMENT_ADDR);
	}
}

void write_prog_mem(icsp_t *icsp, uint32_t addr, icsp_word_t src) {
	icsp_enter(icsp); // TODO move all of these to caller

	// sleep_us(1000); // TODO is this needed? In pickle they had it but my PIC seems to work without it
	// uint32_t data = icsp_read(icsp, ICSP_CMD_LOAD_CONFIG);
	// printf("Load config: %x\n", data);

	icsp_imperative(icsp, ICSP_CMD_RESET_ADDR); // Reset to 0 // TODO try with this

	for (int i = 0; i < addr; i++)
		icsp_imperative(icsp, ICSP_CMD_INCREMENT_ADDR);
	printf("Writing %x to %x\n", src, addr); // TODO remove
	icsp_load(icsp, ICSP_CMD_LOAD_PROG_MEM, src);

	// icsp_imperative(icsp, ICSP_CMD_BEGIN_INT_TIMED);
	// sleep_us(ICSP_TDIS_TYP);
	icsp_imperative(icsp, ICSP_CMD_BEGIN_EXT_TIMED);
	sleep_us(ICSP_TPEXT_MIN);
	icsp_imperative(icsp, ICSP_CMD_END_EXT_TIMED);

}

void row_erase_bulk_prog(icsp_t *icsp, bool erase_user_ids) {
	icsp_enter(icsp);

	if (erase_user_ids) // According to DS41397B, to erase user IDs the current address must be 0x8000 <= addr <= 0x8008
		for (int i = 0; i < ICSP_BULK_ERASE_PROG_MEM_UISERID_TARGET; i++)
			icsp_imperative(icsp, ICSP_CMD_INCREMENT_ADDR);

	icsp_imperative(icsp, ICSP_CMD_BULK_ERASE_PROG);
	sleep_us(ICSP_TERAB_MAX);
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

void icsp_load(icsp_t* icsp, uint8_t command, uint16_t data) { // TODO change data to icsp_word_t

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
