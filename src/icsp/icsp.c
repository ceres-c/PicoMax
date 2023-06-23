#include "icsp.h"

void read_prog_mem(icsp_t *icsp, uint32_t addr, uint32_t size, uint8_t *dst) {
	icsp_enter(icsp);

	int pc = 0;

	icsp_imperative(icsp, ICSP_CMD_RESET_ADDR); // Reset to 0
	if (addr >= ICSP_CONFIG_MEM_ADDR) {
		icsp_load(icsp, ICSP_CMD_LOAD_CONFIG, 0); // Idk which word to load here, pickle does 0, let's go with that
		pc =+ ICSP_CONFIG_MEM_ADDR;
	}

	for (pc; pc < addr; pc++)
		icsp_imperative(icsp, ICSP_CMD_INCREMENT_ADDR);
	for (int j = 0; j < size; j++) {
		*((icsp_word_t*)(dst + j * ICSP_BYTES_PER_WORD)) = icsp_read(icsp, ICSP_CMD_READ_PROG_MEM);
		icsp_imperative(icsp, ICSP_CMD_INCREMENT_ADDR);
	}
}

void write_prog_mem(icsp_t *icsp, uint32_t addr, icsp_word_t src) {
	icsp_enter(icsp);

	int pc = 0;

	icsp_imperative(icsp, ICSP_CMD_RESET_ADDR); // Reset to 0
	if (addr >= ICSP_CONFIG_MEM_ADDR) {
		icsp_load(icsp, ICSP_CMD_LOAD_CONFIG, 0); // Idk which word to load here, pickle does 0, let's go with that
		pc =+ ICSP_CONFIG_MEM_ADDR;
	}

	for (pc; pc < addr; pc++)
		icsp_imperative(icsp, ICSP_CMD_INCREMENT_ADDR);
	icsp_load(icsp, ICSP_CMD_LOAD_PROG_MEM, src);

	// icsp_imperative(icsp, ICSP_CMD_BEGIN_INT_TIMED); // This could also work, I guess
	// sleep_us(ICSP_TDIS_TYP);
	icsp_imperative(icsp, ICSP_CMD_BEGIN_EXT_TIMED);
	sleep_us(ICSP_TPEXT_MIN);
	icsp_imperative(icsp, ICSP_CMD_END_EXT_TIMED);

}

void bulk_erase_prog(icsp_t *icsp, bool erase_user_ids) {
	icsp_enter(icsp);

	if (erase_user_ids) // According to DS41397B, to erase user IDs the current address must be 0x8000 <= addr <= 0x8008
		for (int i = 0; i < ICSP_CONFIG_MEM_ADDR; i++)
			icsp_imperative(icsp, ICSP_CMD_INCREMENT_ADDR);

	icsp_imperative(icsp, ICSP_CMD_BULK_ERASE_PROG);
	sleep_us(ICSP_TERAB_MAX);
}

void icsp_enter(icsp_t* icsp) {
	icsp_program_init(icsp->pio, icsp->sm, icsp->prog_offs, icsp->clkdiv, ICSPCLK, ICSPDAT);

	pio_sm_put_blocking(icsp->pio, icsp->sm, 32);
	pio_sm_put_blocking(icsp->pio, icsp->sm, 0);
	pio_sm_put_blocking(icsp->pio, icsp->sm, ICSP_KEY);
	pio_sm_put_blocking(icsp->pio, icsp->sm, 0);

	pio_sm_get_blocking(icsp->pio, icsp->sm); // Discard returned value, but make this function blocking

	pio_sm_set_enabled(icsp->pio, icsp->sm, false);

	// sleep_us(1000); // Is this needed? In pickle they had it but my PIC seems to work without it
}

void icsp_imperative(icsp_t* icsp, uint8_t command) {
	icsp_program_init(icsp->pio, icsp->sm, icsp->prog_offs, icsp->clkdiv, ICSPCLK, ICSPDAT);

	pio_sm_put_blocking(icsp->pio, icsp->sm, 5);
	pio_sm_put_blocking(icsp->pio, icsp->sm, 0);
	pio_sm_put_blocking(icsp->pio, icsp->sm, command);
	pio_sm_put_blocking(icsp->pio, icsp->sm, 0);

	pio_sm_get_blocking(icsp->pio, icsp->sm); // Discard returned value, but make this function blocking

	pio_sm_set_enabled(icsp->pio, icsp->sm, false);

	pio_sm_clear_fifos(icsp->pio, icsp->sm);
}


void icsp_load(icsp_t* icsp, uint8_t command, uint16_t data) {
	icsp_program_init(icsp->pio, icsp->sm, icsp->prog_offs, icsp->clkdiv, ICSPCLK, ICSPDAT);

	pio_sm_put_blocking(icsp->pio, icsp->sm, 5);
	pio_sm_put_blocking(icsp->pio, icsp->sm, 15); // 14 bits data from the PIC + 2 star/stop bits - 1
	pio_sm_put_blocking(icsp->pio, icsp->sm, command);
	pio_sm_put_blocking(icsp->pio, icsp->sm, (data << 2) | 1); // Add start bit and mark as send command

	pio_sm_get_blocking(icsp->pio, icsp->sm); // Discard returned value, but make this function blocking

	pio_sm_set_enabled(icsp->pio, icsp->sm, false);
}

uint16_t icsp_read(icsp_t* icsp, uint8_t command) {
	icsp_program_init(icsp->pio, icsp->sm, icsp->prog_offs, icsp->clkdiv, ICSPCLK, ICSPDAT);

	pio_sm_put_blocking(icsp->pio, icsp->sm, 5);
	pio_sm_put_blocking(icsp->pio, icsp->sm, 15); // 14 bits data from the PIC + 2 star/stop bits - 1
	pio_sm_put_blocking(icsp->pio, icsp->sm, command);
	pio_sm_put_blocking(icsp->pio, icsp->sm, 0);

	uint32_t ret = pio_sm_get_blocking(icsp->pio, icsp->sm);

	pio_sm_set_enabled(icsp->pio, icsp->sm, false);

	ret >>= 17; // Take upper 16 bits and remove trailing stop bit
	ret &= ICSP_WORD_MASK; // Remove spurious last high bit
	// NOTE: technically stop bit should be a 0, but we are missing some pull downs, I guess

	return (uint16_t)ret;
}

bool icsp_init(PIO pio, icsp_t *icsp) {
	if (!pio_can_add_program(pio, &icsp_program)) {
		return false;
	}
	icsp->pio = pio;
	icsp->sm = 0;
	icsp->prog_offs = pio_add_program(pio, &icsp_program);
	// The standard-mandated 100ns half-period seems to be too short for this setup to work reliably. 200ns looks good
	icsp->clkdiv = (clock_get_hz(clk_sys) / 1e7f);
}
