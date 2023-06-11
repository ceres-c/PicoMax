#include <stdio.h>
#include "pico/stdlib.h"

#include "glitcher.h"

unsigned int* gpio_out = (unsigned int*)(SIO_BASE + SIO_GPIO_OUT_OFFSET);

static void power_cycle_target() {
	gpio_put(MAX_EN_PIN, 0);
	sleep_ms(50);
	gpio_put(MAX_EN_PIN, 1);
}

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
}

int main() {
	stdio_init_all();

	init_pins();

	uint32_t delay = 0;
	uint32_t pulse_width = 0;
	// bool trig_in = false;
	bool trig_out = false;

	while (true) {
		uint8_t cmd = getchar();
		switch(cmd) {
			case CMD_DELAY:
				fread(&delay, 1, 4, stdin);
				printf("Poweron->glitch delay set to %d\n", delay);
				break;
			case CMD_WIDTH:
				fread(&pulse_width, 1, 4, stdin);
				printf("Glitch pulse width set to %d\n", pulse_width);
				break;
			case CMD_TRIG_IN:
				// trig_in = true;
				// printf("Enabled trigger in on pin %d\n", TRIG_IN_PIN);
				printf("Unimplemented\n");
				break;
			case CMD_TRIG_OUT:
				trig_out ^= 1;
				printf("Trigger out on pin %d, state: %d\n", TRIG_OUT_PIN, trig_out);
				break;
			case CMD_GLITCH:
				// power_cycle_target(); // TODO decomment this

				uint32_t sel_trig_mask = (MAX_SEL_MASK | trig_out << TRIG_OUT_PIN);
				printf("Glitching with mask: %d\n", sel_trig_mask);

				// Sart off with enable because because the currently selected voltage is the lowest one anyway,
				// so the output is 0V anways. Note that there is an 800ns delay between the following two lines.
				*(unsigned int*)CLR_GPIO_ATOMIC = MAX_EN_MASK; // MAX4619 enable is active low
				*(unsigned int*)SET_GPIO_ATOMIC = sel_trig_mask;
				sleep_ms(10); // TODO use delay
				*(unsigned int*)CLR_GPIO_ATOMIC = MAX_SEL_MASK; // Switch to glitch voltage
				sleep_ms(1); // TODO use pulse_width
				*(unsigned int*)SET_GPIO_ATOMIC = MAX_SEL_MASK; // Switch back to nominal voltage
				sleep_ms(10); // TODO remove
				*(unsigned int*)CLR_GPIO_ATOMIC = sel_trig_mask; // TODO move this at the beginning, should clear when starting the glitch
				*(unsigned int*)SET_GPIO_ATOMIC = MAX_EN_MASK;

				// gpio_put(MAX_EN_PIN, 1);
				// sleep_ms(1000);
				// gpio_put(MAX_EN_PIN, 0);

				// while(!gpio_get(1 + IN_NRF_VDD));
				// for(uint32_t i=0; i < delay; i++) {
				// 	asm("NOP");
				// }
				// gpio_put(PDND_GLITCH, 1);
				// for(uint32_t i=0; i < pulse_width; i++) {
				// 	asm("NOP");
				// }
				// gpio_put(PDND_GLITCH, 0);
				break;
		}
	}

	return 0;
}
