#include "icsp.h"

void enter_icsp(icsp_t* icsp) {
	uint icsp_enter_program_offset = pio_add_program(icsp->pio, &icsp_enter_program);
	uint icsp_enter_sm	= 0;

	pio_sm_drain_tx_fifo(icsp->pio, icsp_enter_sm);
	pio_sm_clear_fifos(icsp->pio, icsp_enter_sm);

	// float icsp->clkdiv = (clock_get_hz(clk_sys) / 1e6f) / 8.0f; // 100 ns (half period) / 2 // TODO remove
	icsp_enter_program_init(icsp->pio, icsp_enter_sm, icsp_enter_program_offset, icsp->clkdiv, ICSPCLK, ICSPDAT);

	pio_sm_put_blocking(icsp->pio, icsp_enter_sm, ICSP_KEY);
	pio_sm_get_blocking(icsp->pio, icsp_enter_sm); // Discard returned value

	pio_sm_set_enabled(icsp->pio, icsp_enter_sm, false);
	pio_remove_program(icsp->pio, &icsp_enter_program, icsp_enter_program_offset);

	pio_sm_clear_fifos(icsp->pio, icsp_enter_sm);

	return;
}

uint32_t send_command_6bits(icsp_t* icsp, uint32_t command) { // TODO change to void
	uint icsp_imperative_program_offset = pio_add_program(icsp->pio, &icsp_imperative_program);
	uint icsp_imperative_sm = 0;

	pio_sm_drain_tx_fifo(icsp->pio, icsp_imperative_sm);
	pio_sm_clear_fifos(icsp->pio, icsp_imperative_sm);

	// float clkdiv = (clock_get_hz(clk_sys) / 1e6f) / 8.0f; // 100 ns (half period) / 2 // TODO remove
	icsp_imperative_program_init(icsp->pio, icsp_imperative_sm, icsp_imperative_program_offset, icsp->clkdiv, ICSPCLK, ICSPDAT);

	pio_sm_put_blocking(icsp->pio, icsp_imperative_sm, command);
	uint32_t ret = pio_sm_get_blocking(icsp->pio, icsp_imperative_sm);

	pio_sm_set_enabled(icsp->pio, icsp_imperative_sm, false);
	pio_remove_program(icsp->pio, &icsp_imperative_program, icsp_imperative_program_offset);

	pio_sm_clear_fifos(icsp->pio, icsp_imperative_sm);

	return ret;
}

uint32_t icsp_load(icsp_t* icsp, uint8_t command, uint16_t data) {

	uint32_t payload = 0;
	payload |= data << (6 + 1); // 6 bits of command + 1 data start bit (0)
	payload |= command;

	uint icsp_load_program_offset = pio_add_program(icsp->pio, &icsp_load_program);
	uint icsp_load_sm = 0;

	pio_sm_drain_tx_fifo(icsp->pio, icsp_load_sm);
	pio_sm_clear_fifos(icsp->pio, icsp_load_sm);

	// float clkdiv = (clock_get_hz(clk_sys) / 1e6f) / 8.0f; // 100 ns (half period) / 2 // TODO all these comments are wrong, timing is much more than 100ns now
	icsp_load_program_init(icsp->pio, icsp_load_sm, icsp_load_program_offset, icsp->clkdiv, ICSPCLK, ICSPDAT);

	pio_sm_put_blocking(icsp->pio, icsp_load_sm, payload);
	uint32_t ret = pio_sm_get_blocking(icsp->pio, icsp_load_sm);

	pio_sm_set_enabled(icsp->pio, icsp_load_sm, false);
	pio_remove_program(icsp->pio, &icsp_load_program, icsp_load_program_offset);

	pio_sm_clear_fifos(icsp->pio, icsp_load_sm);

	return ret;
}

uint32_t icsp_read(icsp_t* icsp, uint8_t command) {

	uint icsp_read_program_offset = pio_add_program(icsp->pio, &icsp_read_program);
	uint icsp_read_sm = 0;

	pio_sm_drain_tx_fifo(icsp->pio, icsp_read_sm);
	pio_sm_clear_fifos(icsp->pio, icsp_read_sm);

	// float icsp->clkdiv = (clock_get_hz(clk_sys) / 1e6f) / 8.0f; // 100 ns (half period) / 2 // TODO remove
	icsp_read_program_init(icsp->pio, icsp_read_sm, icsp_read_program_offset, icsp->clkdiv, ICSPCLK, ICSPDAT);

	pio_sm_put_blocking(icsp->pio, icsp_read_sm, command);
	uint32_t ret = pio_sm_get_blocking(icsp->pio, icsp_read_sm);

	pio_sm_set_enabled(icsp->pio, icsp_read_sm, false);
	pio_remove_program(icsp->pio, &icsp_read_program, icsp_read_program_offset);

	pio_sm_clear_fifos(icsp->pio, icsp_read_sm);

	ret >>= 17; // Take upper 16 bits and remove trailing stop bit
	ret &= ICSP_WORD_MASK; // Remove spurious last high bit (technically stop bit should be a 0, but we are missing some pull downs, I guess)

	return ret;
}
