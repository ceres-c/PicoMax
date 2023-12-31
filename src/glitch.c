#include "glitch.h"

const PIO glitcher_pio = pio0;
bool glitch_irq_registered = false;

void __no_inline_not_in_flash_func(prepare_glitch)(glitch_t *glitch) {
	// TODO maybe add a timeout with an external watchdog timer?
	glitch_pio_program_init(glitch, MAX_SEL_PIN, TRIG_IN_PIN, TRIG_OUT_PIN);

	pio_sm_put_blocking(glitch->pio, glitch->sm, glitch->on_rising);
	pio_sm_put_blocking(glitch->pio, glitch->sm, glitch->delay);
	pio_sm_put_blocking(glitch->pio, glitch->sm, glitch->pulse_width);
}
void __isr glitch_irq_func() {
	// These two operations take roughly 4,5us
	irq_set_enabled((glitcher_pio == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0, false); // Disable this IRQ
	gpio_set_function(MAX_SEL_PIN, GPIO_FUNC_SIO); // Return MAX_SEL_PIN to SIO after PIO
	// Add here code that should be executed when right after the glitch happened, if needed
}

bool glitch_init(PIO pio, glitch_t *glitch) {
	if (!pio_can_add_program(pio, &glitch_pio_program)) {
		return false;
	}
	glitch->pio = pio;
	glitch->sm = 0;
	glitch->prog_offs = pio_add_program(pio, &glitch_pio_program);
	glitch->on_rising = true;
	glitch->delay = 0;
	glitch->pulse_width = 0;
	glitch->trig_out = false;
	return true;
}
