#include "glitch.h"

void __no_inline_not_in_flash_func(glitch_power_on)() {
	*SET_GPIO_ATOMIC = (MAX_EN_MASK | MAX_SEL_MASK); // Ensure MAX4619 is disabled and highest voltage is selected
	*CLR_GPIO_ATOMIC = MAX_EN_MASK; // Then enable MAX4619
	// Right now we have:
	// EN=low, SEL=high
	// (output enabled, highest voltage selected)
}

void __no_inline_not_in_flash_func(glitch_power_off)() {
	*SET_GPIO_ATOMIC = (MAX_EN_MASK | MAX_SEL_MASK); // Ensure MAX4619 is disabled and highest voltage is selected
	// Right now we have:
	// EN=high, SEL=high
	// (output disabled, highest voltage selected)
	sleep_ms(50); // Chosed by fair dice roll
}

uint8_t __no_inline_not_in_flash_func(do_glitch)(glitch_t *glitch, uint32_t delay, uint32_t pulse_width, bool trig_out) {
	// TODO maybe add a timeout with an external watchdog timer?
	glitch_trigger_program_init(glitch->pio, glitch->sm, glitch->prog_offs, trig_out, MAX_SEL_PIN, PIC_OUT_PIN, TRIG_OUT_PIN);

	pio_sm_put_blocking(glitch->pio, glitch->sm, delay);
	pio_sm_put_blocking(glitch->pio, glitch->sm, pulse_width);
	uint32_t ret = pio_sm_get_blocking(glitch->pio, glitch->sm);

	gpio_set_function(MAX_SEL_PIN, GPIO_FUNC_SIO); // Return MAX_SEL_PIN to SIO after PIO

	return (uint8_t)ret;
}