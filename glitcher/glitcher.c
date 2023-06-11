#include <stdio.h>
#include "pico/stdlib.h"

#include "glitcher.h"

static void init_pins() {
	gpio_set_function(MAX_EN_PIN, GPIO_FUNC_SIO);
	gpio_set_function(MAX_SEL_PIN, GPIO_FUNC_SIO);
	// NO need to set TRIG_IN_PIN because it's an input and those are always accessible to the cpu
	gpio_set_function(TRIG_OUT_PIN, GPIO_FUNC_SIO);

	gpio_pull_up(MAX_EN_PIN); // MAX4619 EN is active low
	gpio_pull_up(MAX_SEL_PIN); // Default selected voltage to the highest of the two
	gpio_pull_down(TRIG_OUT_PIN);

	gpio_set_slew_rate(MAX_EN_PIN, GPIO_SLEW_RATE_FAST); // SPEED
	gpio_set_slew_rate(MAX_SEL_PIN, GPIO_SLEW_RATE_FAST);
	gpio_set_slew_rate(TRIG_OUT_PIN, GPIO_SLEW_RATE_FAST);

	gpio_set_dir(MAX_EN_PIN, GPIO_OUT);
	gpio_set_dir(MAX_SEL_PIN, GPIO_OUT);
	gpio_set_dir(TRIG_OUT_PIN, GPIO_OUT);

	*(uint32_t*)SET_GPIO_ATOMIC = MAX_EN_MASK; // MAX4619 enable is active low, so disable it
}

// This function must be in RAM to have consistent timing. Due to Pi Pico's slow flash, first few
// executions were taking far longer
void __no_inline_not_in_flash_func(glitch)(uint32_t delay, uint32_t pulse_width, bool trig_out) {
	uint32_t mask_glitch = (MAX_SEL_MASK | trig_out << TRIG_OUT_PIN);

	////////// Power cycle //////////
	*(uint32_t*)SET_GPIO_ATOMIC = (MAX_EN_MASK | MAX_SEL_MASK); // MAX4619's input pin EN=high disables MAX4619's output
	*(uint32_t*)CLR_GPIO_ATOMIC = TRIG_OUT_MASK;
	// Right now we have:
	// EN=high, SEL=high, TRIG_OUT=low
	// (output disabled, highest voltage selected, trigger out disabled)
	sleep_ms(50);
	*(uint32_t*)XOR_GPIO_ATOMIC = MAX_EN_MASK;
	// Right now we have:
	// EN=low, SEL=high, TRIG_OUT=low
	// (output enabled, highest voltage selected, trigger out disabled)

	////////// Wait for glitch moment //////////
	for(uint32_t i = 0; i < delay; i++) {
		asm("NOP");
	}

	////////// Glitch //////////
	*(uint32_t*)XOR_GPIO_ATOMIC = mask_glitch;
	// Right now we have:
	// EN=low, SEL=low, TRIG_OUT=?
	// (output enabled, lowest voltage selected, trigger out enabled/disabled)

	for(uint32_t i = 0; i < pulse_width; i++) {
		asm("NOP");
	}

	*(uint32_t*)XOR_GPIO_ATOMIC = mask_glitch;
	// Right now we have:
	// EN=low, SEL=high, TRIG_OUT=low
	// (output enabled, highest voltage selected, trigger out disabled)
}

int main() {
	stdio_init_all();

	init_pins();

	// uint32_t delay = 0;
	uint32_t delay = 10; // TODO remove and uncomment above
	// uint32_t pulse_width = 0;
	uint32_t pulse_width = 10; // TODO remove and uncomment above
	// bool trig_in = false;
	bool trig_out = false;

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
			case CMD_TRIG_IN_EN:
				// trig_in = true;
				// printf("Enabled trigger in on pin %d\n", TRIG_IN_PIN);
				printf("Unimplemented\n");
				break;
			case CMD_TRIG_IN_DIS:
				// trig_in = false;
				// printf("Disabled trigger in on pin %d\n", TRIG_IN_PIN);
				printf("Unimplemented\n");
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
			case CMD_GLITCH:
				glitch(delay, pulse_width, trig_out);

				putchar(RESP_OK);
				break;
		}
	}

	return 0;
}
