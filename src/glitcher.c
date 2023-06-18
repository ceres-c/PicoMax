#include "glitcher.h"

static void init_pins() {
	gpio_set_function(MAX_EN_PIN, GPIO_FUNC_SIO);
	gpio_set_function(MAX_SEL_PIN, GPIO_FUNC_SIO);
	// No need to set TRIG_IN_PIN because it's an input and those are always accessible to the cpu
	gpio_set_function(TRIG_OUT_PIN, GPIO_FUNC_SIO);
	gpio_set_function(PIC_IN_PIN, GPIO_FUNC_SIO);
	// No need to set PIC_OUT_PIN because it's an input and those are always accessible to the cpu

	gpio_pull_up(MAX_EN_PIN); // MAX4619 EN is active low
	gpio_pull_up(MAX_SEL_PIN); // Default selected voltage to the highest of the two
	gpio_pull_down(TRIG_OUT_PIN);
	gpio_pull_down(PIC_OUT_PIN);
	gpio_pull_down(PIC_IN_PIN);

	gpio_set_slew_rate(MAX_EN_PIN, GPIO_SLEW_RATE_FAST); // SPEED
	gpio_set_slew_rate(MAX_SEL_PIN, GPIO_SLEW_RATE_FAST);
	gpio_set_slew_rate(TRIG_OUT_PIN, GPIO_SLEW_RATE_FAST);

	gpio_set_dir(MAX_EN_PIN, GPIO_OUT);
	gpio_set_dir(MAX_SEL_PIN, GPIO_OUT);
	gpio_set_dir(TRIG_OUT_PIN, GPIO_OUT);
	gpio_set_dir(PIC_IN_PIN, GPIO_OUT);

	*SET_GPIO_ATOMIC = (MAX_EN_MASK | MAX_SEL_MASK); // MAX4619 enable is active low, so disable it

	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

void __no_inline_not_in_flash_func(power_off)() {
	// *SET_GPIO_ATOMIC = TRIG_OUT_MASK; // TODO remove
	*SET_GPIO_ATOMIC = (MAX_EN_MASK | MAX_SEL_MASK); // MAX4619's input pin EN=high disables MAX4619's output
	*CLR_GPIO_ATOMIC = (TRIG_OUT_MASK | PIC_IN_MASK); // PIC_IN needs to be cleared, otherwise the PIC will be "powered" by the Pico
	// Right now we have:
	// EN=high, SEL=high, TRIG_OUT=low
	// (output disabled, highest voltage selected, trigger out disabled)
	sleep_ms(50);
}

void read_uart() {
	// Poor man's uart_getc_timeout
	bool is_readable = uart_is_readable_within_us(UART_ID, 50); // Check if anyone responded
	if (!is_readable) {
		// No response
		putchar(RESP_KO);
		return;
	}

	while (uart_is_readable_within_us(UART_ID, 200)) { // 1000000/9600 = 104us
		char c = uart_getc(UART_ID);
		if (c) {
			// Cleanup the output to remove spurious \x00
			putchar(c);
		}
	}
}

bool __no_inline_not_in_flash_func(glitch_init)(uint32_t delay, uint32_t pulse_width, bool trig_out) {
	uint32_t mask_glitch = (MAX_SEL_MASK | trig_out << TRIG_OUT_PIN);
	uint32_t timeout;

	////////// Power on //////////
	*SET_GPIO_ATOMIC = mask_glitch; // Default selected voltage to the highest of the two
	*CLR_GPIO_ATOMIC = MAX_EN_MASK;
	// Right now we have:
	// EN=low, SEL=high, TRIG_OUT=?
	// (output enabled, highest voltage selected, trigger out enabled/disabled)

	/////////// Wait for PIC to boot //////////
	timeout = 100000;
	while(timeout && !gpio_get(PIC_OUT_PIN)) {
		sleep_us(1);
		timeout--;
	}
	if (!timeout) {
		putchar('T');
		return false;
	}

	// *SET_GPIO_ATOMIC = TRIG_OUT_MASK; // TODO remove
	*SET_GPIO_ATOMIC = PIC_IN_MASK; // ACK to ready

	timeout = 100000;
	while(timeout && gpio_get(PIC_OUT_PIN)) { // Wait for PIC ACK to ACK
		sleep_us(1);
		timeout--;
	}
	if (!timeout) {
		putchar('T');
		return false;
	}

	putchar(RESP_OK);
	return true;

	////////// Now we're in sync with PIC //////////
}

// This function must be in RAM to have consistent timing. Due to Pi Pico's slow flash, first few
// executions were taking far longer
bool __no_inline_not_in_flash_func(glitch)(uint32_t delay, uint32_t pulse_width, bool trig_out) {
	uint32_t mask_glitch = (MAX_SEL_MASK | trig_out << TRIG_OUT_PIN);

	uint32_t timeout;

	*CLR_GPIO_ATOMIC = PIC_IN_MASK; // Signal to start the loop

	while(!gpio_get(PIC_OUT_PIN)); // Wait for PIC to start loop

	////////// Wait for glitch moment //////////
	for(uint32_t i = 0; i < delay; i++) {
		asm("NOP");
	}

	////////// Glitch //////////
	*XOR_GPIO_ATOMIC = mask_glitch;
	// Right now we have:
	// EN=low, SEL=low, TRIG_OUT=?
	// (output enabled, lowest voltage selected, trigger out enabled/disabled)

	for(uint32_t i = 0; i < pulse_width; i++) {
		asm("NOP");
	}

	*XOR_GPIO_ATOMIC = mask_glitch;
	// Right now we have:
	// EN=low, SEL=high, TRIG_OUT=low
	// (output enabled, highest voltage selected, trigger out disabled)

	////////// Check if PIC survived //////////

	timeout = 100000;
	while(timeout && gpio_get(PIC_OUT_PIN)) {
	// while(timeout && gpio_get(PIC_OUT_PIN) && gpio_get(PIC_BOD_CANARY_PIN)) {
		// Timeout not hit and PIC is still running
		// This also waits for the PIC UART TX to finish
		sleep_us(1);
		timeout--;
	}
	if (!timeout) {
		putchar('T');
		return false;
	}
	// if (!gpio_get(PIC_BOD_CANARY_PIN)) {
	// 	// BOD canary is tripped, AKA PIC is dead
	// 	putchar('F');
	// 	return false;
	// }

	////////// Reset to initial state //////////
	*SET_GPIO_ATOMIC = PIC_IN_MASK; // ACK to finish
	return true;
}

int main() {
	stdio_init_all();
	init_pins();
	uart_init(UART_ID, BAUD_RATE);

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
			case CMD_TRIG_IN_EN:
				// trig_in = true;
				// printf("Enabled trigger in on pin %d\n", TRIG_IN_PIN);
				puts("Unimplemented");
				break;
			case CMD_TRIG_IN_DIS:
				// trig_in = false;
				// printf("Disabled trigger in on pin %d\n", TRIG_IN_PIN);
				puts("Unimplemented");
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
				if (glitch_init(delay, pulse_width, trig_out)) {
					init = true;
				}
				break;
			case CMD_GLITCH:
				if (!init) {
					putchar(RESP_KO);
					break;
				}
				if (glitch(delay, pulse_width, trig_out)) {
					read_uart();
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
