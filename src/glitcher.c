#include "glitcher.h"
#include <stdint.h>

uint glitcher_prog_offst = 0;
uint programmer_program_offset = 0;

const uint glitcher_sm = 0;
const uint programmer_sm = 0;

static void init_pins() {
	gpio_set_function(MAX_EN_PIN, GPIO_FUNC_SIO);
	gpio_set_function(MAX_SEL_PIN, GPIO_FUNC_SIO); // This will be handed over to PIO when we start glitching

	gpio_set_dir(MAX_EN_PIN, GPIO_OUT);
	gpio_set_dir(MAX_SEL_PIN, GPIO_OUT);

	gpio_pull_up(MAX_EN_PIN); // MAX4619 EN is active low
	gpio_pull_up(MAX_SEL_PIN); // Default selected voltage to the highest of the two

	gpio_set_slew_rate(MAX_EN_PIN, GPIO_SLEW_RATE_FAST); // SPEED
	gpio_set_slew_rate(MAX_SEL_PIN, GPIO_SLEW_RATE_FAST);

	*SET_GPIO_ATOMIC = (MAX_EN_MASK | MAX_SEL_MASK); // MAX4619 enable is active low, so disable it
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


void __no_inline_not_in_flash_func(send_command)(uint32_t command) {
	float clkdiv = (clock_get_hz(clk_sys) / 1e7f) / 2.0f; // 100 ns (half period) / 2
	programmer_program_init(programmer_pio, programmer_sm, programmer_program_offset, clkdiv, PROGRAMMER_CLK, PROGRAMMER_DATA);

	putchar('a');
	pio_sm_put_blocking(programmer_pio, programmer_sm, command);
	putchar('b');
	uint8_t ret = pio_sm_get_blocking(programmer_pio, programmer_sm); // Discard returned value for the time being, then switch based on the command
	putchar('c');
	printf("Command returned %d\n", ret);

	return;
}

int main() {
	stdio_init_all();
	init_pins();
	glitcher_prog_offst = pio_add_program(glitcher_pio, &glitch_trigger_program);
	programmer_program_offset = pio_add_program(programmer_pio, &programmer_program);

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
		case CMD_SENDCMD: // TODO change the letter + move this below
			send_command(PROGRAMMER_CMD_RESET_ADDR | 0b1111110000000);
			putchar('C');
			break;
		case 'l':
			send_command(PROGRAMMER_CMD_RESET_ADDR | PROGRAMMER_RECEIVE_BITMASK); // Testing a receive command
			putchar('C');
			break;
		case CMD_PING:
			putchar(RESP_PONG);
			break;
		case CMD_DELAY:
			fread(&delay, 1, 4, stdin);
			// printf("Poweron->glitch delay set to %d\n", delay);
			putchar(RESP_OK);
			break;
		case CMD_WIDTH:
			fread(&pulse_width, 1, 4, stdin);
			// printf("Glitch pulse width set to %d\n", pulse_width);
			putchar(RESP_OK);
			break;
		case CMD_TRIG_OUT_EN:
			trig_out = true;
			putchar(RESP_OK);
			// printf("Trigger out on pin %d, state: %d\n", TRIG_OUT_PIN, trig_out);
			break;
		case CMD_TRIG_OUT_DIS:
			trig_out = false;
			putchar(RESP_OK);
			// printf("Disabled trigger out on pin %d\n", TRIG_OUT_PIN);
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
