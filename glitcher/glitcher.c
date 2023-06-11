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

	*(uint32_t*)SET_GPIO_ATOMIC = MAX_EN_HI_MASK; // MAX4619 enable is active low, so disable it
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
				break;
			case CMD_WIDTH:
				fread(&pulse_width, 1, 4, stdin);
				// printf("Glitch pulse width set to %d\n", pulse_width);
				break;
			case CMD_TRIG_IN:
				// trig_in = true;
				// printf("Enabled trigger in on pin %d\n", TRIG_IN_PIN);
				printf("Unimplemented\n");
				break;
			case CMD_TRIG_OUT:
				trig_out ^= 1;
				putchar(RESP_TRIG_OUT);
				// printf("Trigger out on pin %d, state: %d\n", TRIG_OUT_PIN, trig_out);
				break;
			case CMD_GLITCH:
				// MAX4619's input pin EN=high disables MAX4619's output
				uint32_t mask = (MAX_SEL_HI_MASK | MAX_EN_LO_MASK | trig_out << TRIG_OUT_PIN);

				////////// Power cycle //////////
				*(uint32_t*)SET_GPIO_ATOMIC = MAX_EN_HI_MASK;
				sleep_ms(50);
				*(uint32_t*)GPIO_ATOMIC = mask;

				////////// Wait for glitch moment //////////
				for(uint32_t i = 0; i < delay; i++) {
					asm("NOP");
				}

				////////// Glitch //////////
				*(uint32_t*)CLR_GPIO_ATOMIC = MAX_SEL_HI_MASK; // Switch to glitch voltage (clear SEL)

				for(uint32_t i = 0; i < pulse_width; i++) {
					asm("NOP");
				}

				*(uint32_t*)SET_GPIO_ATOMIC = MAX_SEL_HI_MASK; // Switch back to nominal voltage (set SEL)
				*(uint32_t*)CLR_GPIO_ATOMIC = TRIG_OUT_HI_MASK; // Disable trigger out (if enabled, or keep low)

				// putchar(RESP_GLITCH_DONE);



				// // OLD CODE BELOW




				// uint32_t power_mask = (MAX_SEL_HI_MASK | MAX_EN_LO_MASK | trig_out << TRIG_OUT_PIN);
				// // printf("Glitching with mask: %d\n", power_mask); // TODO remove

				// ////////// Power cycle //////////
				// *(uint32_t*)SET_GPIO_ATOMIC = MAX_EN_MASK; // MAX4619's input pin EN=high disables MAX4619's output
				// sleep_ms(50);
				// *(uint32_t*)GPIO_ATOMIC = power_mask;

				// ////////// Wait for glitch moment //////////
				// for(uint32_t i = 0; i < delay; i++) {
				// 	asm("NOP");
				// }

				// ////////// Glitch //////////
				// *(uint32_t*)CLR_GPIO_ATOMIC = MAX_SEL_HI_MASK; // Switch to glitch voltage

				// for(uint32_t i = 0; i < pulse_width; i++) {
				// 	asm("NOP");
				// }

				// *(uint32_t*)SET_GPIO_ATOMIC = MAX_SEL_HI_MASK; // Switch back to nominal voltage
				// *(uint32_t*)CLR_GPIO_ATOMIC = TRIG_OUT_HI_MASK; // Disable trigger out (if enabled, or keep low)

				// putchar(RESP_GLITCH_DONE);

				break;
		}
	}

	return 0;
}
