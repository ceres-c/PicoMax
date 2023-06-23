#include "glitcher.h"
#include <stdint.h> // TODO remove if not needed

uint glitcher_prog_offst = 0;

const uint glitcher_sm = 0;

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

void __no_inline_not_in_flash_func(power_off)() {
	*SET_GPIO_ATOMIC = (MAX_EN_MASK | MAX_SEL_MASK); // Ensure MAX4619 is disabled and highest voltage is selected
	// Right now we have:
	// EN=high, SEL=high
	// (output disabled, highest voltage selected)
	sleep_ms(50); // TODO maybe reduce this?
}

void __no_inline_not_in_flash_func(power_on)() {
	*SET_GPIO_ATOMIC = (MAX_EN_MASK | MAX_SEL_MASK); // Ensure MAX4619 is disabled and highest voltage is selected
	*CLR_GPIO_ATOMIC = MAX_EN_MASK; // Then enable MAX4619
	// Right now we have:
	// EN=low, SEL=high
	// (output enabled, highest voltage selected)
}

// This function must be in RAM to have consistent timing. Due to Pi Pico's slow flash, first few
// executions were taking far longer
uint8_t __no_inline_not_in_flash_func(glitch)(uint32_t delay, uint32_t pulse_width, bool trig_out) {
	// TODO maybe add a timeout with an external watchdog timer?
	uint sm = 0; // TODO remove and change to glitcher_sm variable
	glitch_trigger_program_init(glitcher_pio, sm, glitcher_prog_offst, trig_out, MAX_SEL_PIN, PIC_OUT_PIN, TRIG_OUT_PIN);

	pio_sm_put_blocking(glitcher_pio, sm, delay);
	pio_sm_put_blocking(glitcher_pio, sm, pulse_width);
	uint32_t ret = pio_sm_get_blocking(glitcher_pio, sm);

	gpio_set_function(MAX_SEL_PIN, GPIO_FUNC_SIO); // Return MAX_SEL_PIN to SIO after PIO

	return (uint8_t)ret;
}

int main() {
	stdio_init_all();
	stdio_set_translate_crlf(&stdio_usb, false); // TODO remove if not needed
	init_pins();
	glitcher_prog_offst = pio_add_program(glitcher_pio, &glitch_trigger_program); // TODO make local

	icsp_t icsp = {
		.pio = icsp_pio,
		.sm = 0,
		.prog_offs = pio_add_program(icsp.pio, &icsp_program),
		// The standard-mandated 100ns half-period seems to be too short for this setup
		// to work reliably. 200ns looks good
		.clkdiv = (clock_get_hz(clk_sys) / 1e7f),
	};

	// uint32_t delay = 0;
	uint32_t delay = 50; // TODO remove and uncomment above
	uint32_t pulse_width = 0;
	// bool trig_in = false;
	bool trig_out = false;
	bool powered_on = false;
	bool init = false;

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
			fread(&delay, 1, 4, stdin);
			putchar(RESP_OK);
			break;
		case CMD_WIDTH:
			fread(&pulse_width, 1, 4, stdin);
			putchar(RESP_OK);
			break;
		case CMD_TRIG_OUT_EN:
			trig_out = true;
			putchar(RESP_OK);
			break;
		case CMD_TRIG_OUT_DIS:
			trig_out = false;
			putchar(RESP_OK);
			break;
		case CMD_GLITCH_INIT: // TODO is this actually needed?
			power_off();
			power_on();
			init = true;
			putchar(RESP_OK);
			break;
		case CMD_GLITCH:
			if (!init) {
				putchar(RESP_KO);
				break;
			}
			uint8_t ret = glitch(delay, pulse_width, trig_out);
			switch(ret) {
			case 0b000:
				// Both PIC outputs are low, the chip died
				putchar(RESP_KO);
				break;
			case 0b010:
				// PIC is alive, but glitching failed
				putchar(RESP_GLITCH_FAIL);
				break;
			case 0b100:
				// GLITCH!
				putchar(RESP_OK);
				break;
			}
			break;
		case CMD_POWERON:
			if (powered_on) {
				putchar(RESP_KO);
				break;
			}
			powered_on = true;
			*SET_GPIO_ATOMIC = MAX_SEL_MASK;
			*CLR_GPIO_ATOMIC = MAX_EN_MASK;
			putchar(RESP_OK);
			break;
		case CMD_POWEROFF:
			if (!powered_on) {
				putchar(RESP_KO);
				break;
			}
			powered_on = false;
			*SET_GPIO_ATOMIC = MAX_EN_MASK;
			*CLR_GPIO_ATOMIC = MAX_SEL_MASK;
			putchar(RESP_OK);
			break;
		}
	}

	return 0;
}
