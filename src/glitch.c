#include "glitch.h"

uint8_t __no_inline_not_in_flash_func(do_glitch)(glitch_t *glitch) {
	// TODO maybe add a timeout with an external watchdog timer?
	glitch_trigger_program_init(glitch->pio, glitch->sm, glitch->prog_offs, glitch->trig_out, MAX_SEL_PIN, PIC_OUT_PIN, TRIG_OUT_PIN);

	pio_sm_put_blocking(glitch->pio, glitch->sm, glitch->on_rising);
	pio_sm_put_blocking(glitch->pio, glitch->sm, glitch->delay);
	pio_sm_put_blocking(glitch->pio, glitch->sm, glitch->pulse_width);
	uint32_t ret = pio_sm_get_blocking(glitch->pio, glitch->sm);

	gpio_set_function(MAX_SEL_PIN, GPIO_FUNC_SIO); // Return MAX_SEL_PIN to SIO after PIO

	return (uint8_t)ret;
}

bool glitch_init(PIO pio, glitch_t *glitch) {
	if (!pio_can_add_program(pio, &glitch_trigger_program)) {
		return false;
	}
	glitch->pio = pio;
	glitch->sm = 0;
	glitch->prog_offs = pio_add_program(pio, &glitch_trigger_program);
	glitch->on_rising = true;
	glitch->delay = 0;
	glitch->pulse_width = 0;
	glitch->trig_out = false;
	return true;
}
