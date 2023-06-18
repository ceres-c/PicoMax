#include "glitcher.h"

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
bool __no_inline_not_in_flash_func(glitch)(uint32_t delay, uint32_t pulse_width, bool trig_out) {
	// TODO configure gpio function to PIO (if needed?)
	// TODO enable PIO
	// TODO wait for interrupt from PIO after glitching is done
	// TODO check if PIC is still alive

	PIO pio = pio0;
	uint sm = 0;
	uint offset = pio_add_program(pio, &glitch_trigger_program);
	// glitch_trigger_program_init(pio, sm, offset, trig_out, MAX_SEL_PIN, TRIG_IN_PIN, TRIG_OUT_PIN);
	glitch_trigger_program_init(pio, sm, offset, trig_out, 5, TRIG_IN_PIN, TRIG_OUT_PIN); // TODO using random pin for now

	// pio_sm_put_blocking(pio, sm, 1); // TODO decomment once the PIO actually pulls anything
	// pio_sm_put_blocking(pio, sm, 2);
	// pio_sm_get_blocking(pio, sm); // TODO decomment once the PIO actually returns something

	return true;
}

int main() {
	stdio_init_all();
	init_pins();

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
			case CMD_GLITCH_INIT:
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
				if (glitch(delay, pulse_width, trig_out)) {
					// TODO do something here
				} // Else PIC is dead and the glitch function will write on UART why
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
