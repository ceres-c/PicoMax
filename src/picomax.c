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
		case CMD_READ_DATA:
			uint32_t read_data_addr = 0, read_data_size = 0;
			fread(&read_data_addr, 1, 4, stdin); // Words in this case are bytes because the upper 6 bits from the PIC are zeros
			fread(&read_data_size, 1, 4, stdin);
			if (read_data_size > 0xFFFF) {
				// Trying to allocate more than 65k. Honestly didn't try if it could work, but let's just not
				putchar(RESP_KO);
				break;
			}
			uint8_t *data_data = calloc(read_data_size, 1);
			putchar(RESP_OK);

			icsp_power_on();
			icsp_enter(&icsp);
			read_data_mem(&icsp, read_data_addr, read_data_size, data_data);
			icsp_power_off();

			putchar(RESP_OK);

			fwrite(data_data, 1, read_data_size, stdout);
			fflush(stdout);
			free(data_data);
			break;
		case CMD_READ_PROG:
			uint32_t read_prog_addr = 0, read_prog_size = 0;
			fread(&read_prog_addr, 1, 4, stdin); // In words
			fread(&read_prog_size, 1, 4, stdin); // In words, again
			if ((read_prog_size * ICSP_BYTES_PER_WORD) > 0xFFFF) {
				// Trying to allocate more than 65k. Honestly didn't try if it could work, but let's just not
				putchar(RESP_KO);
				break;
			}
			uint8_t *prog_data = calloc(read_prog_size, ICSP_BYTES_PER_WORD);
			putchar(RESP_OK);

			icsp_power_on();
			icsp_enter(&icsp);
			read_prog_mem(&icsp, read_prog_addr, read_prog_size, prog_data);
			icsp_power_off();

			putchar(RESP_OK);

			fwrite(prog_data, ICSP_BYTES_PER_WORD, read_prog_size, stdout);
			fflush(stdout);
			free(prog_data);
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
			icsp_enter(&icsp);
			write_prog_mem(&icsp, write_addr, new_data);
			icsp_power_off();

			putchar(RESP_OK);
			break;
		case CMD_ERASE_BULK_DATA:
			icsp_power_on();
			icsp_enter(&icsp);
			bulk_erase_data(&icsp);
			icsp_power_off();

			putchar(RESP_OK);
			break;
		case CMD_ERASE_BULK_PROG:
			icsp_power_on();
			icsp_enter(&icsp);
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
			// Power off
			*SET_GPIO_ATOMIC = MAX_EN_MASK; // Disable MAX4619
			sleep_ms(5); // Randomly chosen

			// Power on
			gpio_disable_pulls(nMCLR);
			*SET_GPIO_ATOMIC = (MAX_EN_MASK | MAX_SEL_MASK | nMCLR_MASK); // Ensure MAX4619 is disabled and highest voltage is selected
			*CLR_GPIO_ATOMIC = MAX_EN_MASK;	// Then enable MAX4619
			sleep_us(ICSP_TENTS);

			// Enable programming mode
			*CLR_GPIO_ATOMIC = nMCLR_MASK;
			// The glitcher PIO program will trigger here and start counting for delay
			sleep_us(ICSP_TENTH);

			uint16_t data;
			// // Get DeviceID
			// icsp_enter(&icsp);
			// read_prog_mem(&icsp, 0x8006, 0x01, (uint8_t*)&data);

			// if (data == 0) {
			// 	putchar(RESP_KO);
			// 	break;
			// } else if ((data & DEVICEID_MASK) != PIC16LF1936_DEVICEID) {
			// 	putchar(RESP_GLITCH_WEIRD);
			// 	break;
			// }

			prepare_glitch(&glitch);
			icsp_enter(&icsp);

			icsp_imperative(&icsp, ICSP_CMD_RESET_ADDR); // Reset to 0

			// for (pc; pc < addr; pc++)
			// 	icsp_imperative(icsp, ICSP_CMD_INCREMENT_ADDR);
			data = icsp_read(&icsp, ICSP_CMD_READ_PROG_MEM);

			if (data != 0) {
				putchar(RESP_OK);
				fwrite(&data, ICSP_BYTES_PER_WORD, 1, stdout);
				fflush(stdout);
			} else {
				putchar(RESP_GLITCH_FAIL);
			}

			break;
		case CMD_POWERON:
			glitch_power_on(false);
			putchar(RESP_OK);
			break;
		case CMD_POWEROFF:
			glitch_power_off();
			putchar(RESP_OK);
			break;
		default:
			putchar(RESP_KO);
			break;
		}
	}

	return 0;
}
