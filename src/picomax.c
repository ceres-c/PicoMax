#include "picomax.h"

static void init_pins() {
	gpio_set_function(MAX_EN_PIN, GPIO_FUNC_SIO);
	gpio_set_function(MAX_SEL_PIN, GPIO_FUNC_SIO); // This will be handed over to PIO when we start glitching
	gpio_set_function(nMCLR, GPIO_FUNC_SIO);

	gpio_set_dir(MAX_EN_PIN, GPIO_OUT);
	gpio_set_dir(MAX_SEL_PIN, GPIO_OUT);
	gpio_set_dir(nMCLR, GPIO_OUT);

	gpio_pull_up(MAX_EN_PIN);	// MAX4619 EN is active low
	gpio_pull_up(MAX_SEL_PIN);	// Default selected voltage to the highest of the two
	gpio_pull_down(nMCLR);

	gpio_set_slew_rate(MAX_EN_PIN, GPIO_SLEW_RATE_FAST); // SPEED
	gpio_set_slew_rate(MAX_SEL_PIN, GPIO_SLEW_RATE_FAST);

	*SET_GPIO_ATOMIC = (MAX_EN_MASK | MAX_SEL_MASK | nMCLR_MASK); // MAX4619 enable is active low, so disable it
}

int main() {
	bool powered_on = false;

	stdio_init_all();
	stdio_set_translate_crlf(&stdio_usb, false);
	init_pins();

	glitch_t glitch;
	if (!glitch_init(glitcher_pio, &glitch)) {
		printf("Failed to initialize glitcher\n");
		return 1;
	}

	icsp_t icsp;
	if (!icsp_init(&icsp, icsp_pio)) {
		printf("Failed to initialize ICSP\n");
		return 1;
	}

	while (true) {
		uint8_t cmd = getchar();
		switch(cmd) {
		// TODO CMD_READ_DATA

		case 'N': // NEW ICSP test
			uint16_t data_out;

			icsp_power_on();
			icsp_enter(&icsp);

			data_out = icsp_read(&icsp, ICSP_CMD_READ_PROG_MEM);
			printf("0x%x\n", data_out);

			icsp_load(&icsp, ICSP_CMD_LOAD_PROG_MEM, 0x1111);
			icsp_imperative(&icsp, ICSP_CMD_BEGIN_EXT_TIMED);
			sleep_us(ICSP_TPEXT_MIN);
			icsp_imperative(&icsp, ICSP_CMD_END_EXT_TIMED);
			sleep_us(ICSP_TDIS_MIN);

			// icsp_imperative(&icsp, ICSP_CMD_INCREMENT_ADDR);
			data_out = icsp_read(&icsp, ICSP_CMD_READ_PROG_MEM);
			printf("0x%x\n", data_out);
			pio_remove_program(icsp.pio, &icsp_program, icsp.prog_offs);
			icsp_power_off();

			putchar(RESP_OK);
			break;
		case CMD_READ_PROG:
			uint32_t read_addr = 0, size = 0;
			fread(&read_addr, 1, 4, stdin); // In words
			fread(&size, 1, 4, stdin); // In words, again
			if ((size * ICSP_BYTES_PER_WORD) > 0xFFFF) {
				// Trying to allocate more than 65k. Honestly didn't try if it could work, but let's just not
				putchar(RESP_KO);
				break;
			}
			uint8_t *data = calloc(size, ICSP_BYTES_PER_WORD);
			putchar(RESP_OK);

			icsp_power_on();
			read_prog_mem(&icsp, read_addr, size, data);
			icsp_power_off();

			putchar(RESP_OK);

			fwrite(data, ICSP_BYTES_PER_WORD, size, stdout);
			fflush(stdout);
			free(data);
			break;
		// TODO CMD_WRITE_DATA
		case CMD_WRITE_PROG:
			uint32_t write_addr = 0;
			fread(&write_addr, 1, 4, stdin); // In words
			uint16_t new_data = 0;
			fread(&new_data, 1, 2, stdin);
			new_data &= ICSP_WORD_MASK; // Only 14 bits are allowed

			// TODO check for max address?

			icsp_power_on();
			write_prog_mem(&icsp, write_addr, new_data);
			icsp_power_off();

			putchar(RESP_OK);
			break;
		case CMD_ERASE_BULK:
			icsp_power_on();
			bulk_erase_prog(&icsp, false); // Do not erase user IDs
			icsp_power_off();

			putchar(RESP_OK);
			break;
		case CMD_PING:
			putchar(RESP_PONG);
			break;
		case CMD_DELAY:
			fread(&glitch.delay, 1, 4, stdin);
			putchar(RESP_OK);
			break;
		case CMD_WIDTH:
			fread(&glitch.pulse_width, 1, 4, stdin);
			putchar(RESP_OK);
			break;
		case CMD_TRIG_OUT_EN:
			glitch.trig_out = true;
			putchar(RESP_OK);
			break;
		case CMD_TRIG_OUT_DIS:
			glitch.trig_out = false;
			putchar(RESP_OK);
			break;
		case CMD_TRIG_IN_RISING:
			glitch.on_rising = true;
			putchar(RESP_OK);
			break;
		case CMD_TRIG_IN_FALLING:
			glitch.on_rising = false;
			putchar(RESP_OK);
			break;
		case CMD_GLITCH:
			// enable_prog_mode(); // TODO decomment when glitching code readout
			target_glitch(&glitch);
			target_wait();
			if (!target_alive()) {
				putchar(RESP_KO);
				break;
			}
			if (target_glitched()) {
				putchar(RESP_OK);
				break;
			}
			putchar(RESP_GLITCH_FAIL);
			break;
		case CMD_POWERON:
			glitch_power_on(false);
			putchar(RESP_OK);
			break;
		case CMD_POWEROFF:
			glitch_power_off();
			putchar(RESP_OK);
			break;
		}
	}

	return 0;
}
