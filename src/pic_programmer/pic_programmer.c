PIO icsp_pio = pio;
uint programmer_sm = sm;
programmer_clk_pin = clk_pin;
programmer_data_pin = data_pin;

void programmer_init(PIO pio, uint sm, uint clk_pin, uint data_pin) {
	icsp_pio = pio;
	programmer_sm = sm;
	programmer_clk_pin = clk_pin;
	programmer_data_pin = data_pin;

	programmer_program_offset = pio_add_program(icsp_pio, &programmer_program);
	programmer_program_init(icsp_pio, programmer_sm, programmer_program_offset, PROGRAMMER_CLKDIV, programmer_clk_pin, programmer_data_pin);

	return;
}

void send_key() {
	uint pic_key_program_offset = pio_add_program(icsp_pio, &pic_key_program);
	uint pic_key_sm	= 0;

	float clkdiv = (clock_get_hz(clk_sys) / 1e7f) / 2.0f; // 100 ns (half period) / 2
	pic_key_program_init(icsp_pio, pic_key_sm, pic_key_program_offset, clkdiv, PROGRAMMER_CLK, PROGRAMMER_DATA);

	pio_sm_put_blocking(icsp_pio, pic_key_sm, 0b01001101010000110100100001010000); // "MCHP" taken from DS41397B-page 18
	pio_sm_get_blocking(icsp_pio, pic_key_sm); // Discard returned value

	pio_remove_program(icsp_pio, &pic_key_program, pic_key_program_offset);

	return;
}
