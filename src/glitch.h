#ifndef _GLITCH_H
#define _GLITCH_H

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "glitch.pio.h"
#include "pins.h"

typedef struct glitch_s {
	PIO pio;
	uint prog_offs;
	uint sm;
	bool on_rising;	// Defaults to rising edge
	uint32_t delay;	// Defaults to 0
	uint32_t pulse_width;	// Defaults to 0
	bool trig_out;	// Defaults to false
	bool blocking;	// Defaults to blocking, if disabled an interrupt is generated when the glitch is done
	// TODO register interrupt
} glitch_t;

static inline void glitch_power_on(bool prog_mode) {
	if (prog_mode) {
		gpio_pull_down(nMCLR);
	} else {
		// PIC boots in normal mode when MCLR is HiZ
		gpio_disable_pulls(nMCLR);
	}

	*SET_GPIO_ATOMIC = (MAX_EN_MASK | MAX_SEL_MASK); // Ensure MAX4619 is disabled and highest voltage is selected
	*CLR_GPIO_ATOMIC = MAX_EN_MASK; // Then enable MAX4619
	// Right now we have:
	// EN=low, SEL=high
	// (output enabled, highest voltage selected)
}

static inline void glitch_power_off() {
	*SET_GPIO_ATOMIC = (MAX_EN_MASK | MAX_SEL_MASK); // Ensure MAX4619 is disabled and highest voltage is selected
	// Right now we have:
	// EN=high, SEL=high
	// (output disabled, highest voltage selected)
	sleep_ms(50); // Chosed by fair dice roll
}

static inline void enable_prog_mode() {
	gpio_pull_down(nMCLR);
}

// These functions are in RAM to be fast.
// Due to Pi Pico's slow flash, first few executions after cache evict take far longer
void __no_inline_not_in_flash_func(target_glitch)(glitch_t *glitch);
void __no_inline_not_in_flash_func(target_wait)();
bool __no_inline_not_in_flash_func(target_alive)();
bool __no_inline_not_in_flash_func(target_glitched)();

static inline void glitch_pio_program_init(glitch_t *glitch, uint pin_max_sel, uint pin_trig_in, uint pin_trig_out) {

	// NOTE: Setting pin directions and values first to avoid cutting power to the target
	// device by switching the MAX4619 selector

	// Set pin directions (pin_trigger_out and pin_max_sel are outputs, pin_trigger_in is an input)
	pio_sm_set_pindirs_with_mask(
		glitch->pio, glitch->sm,
		(glitch->trig_out << pin_trig_out) | (1u << pin_max_sel),
		(glitch->trig_out << pin_trig_out) | (1u << pin_max_sel) | (1u << pin_trig_in));

	// Set default pin values (pin_trigger_out is low, pin_max_sel is high)
	pio_sm_set_pins_with_mask(
		glitch->pio, glitch->sm,
		(1u << pin_max_sel),
		(1u << pin_trig_out) | (1u << pin_max_sel));

	// Claim pins for PIO
	pio_gpio_init(glitch->pio, pin_max_sel);
	pio_gpio_init(glitch->pio, pin_trig_in);
	pio_gpio_init(glitch->pio, pin_trig_out);

	pio_sm_config c = glitch_pio_program_get_default_config(glitch->prog_offs);

	sm_config_set_set_pins(&c, pin_max_sel, 1); 	// Set base pin and number of pins for SET operand
	sm_config_set_in_pins(&c, pin_trig_in); 		// Set input base pin
	sm_config_set_sideset_pins(&c, pin_trig_out);	// Set sideset base pin

	if (!glitch->blocking) {
		// TODO register interrupt for non-blocking mode
		// irq_add_shared_handler(pio_irq, pio_irq_func, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY); // Add a shared IRQ handler
		// irq_set_enabled(pio_irq, true); // Enable the IRQ
		// const uint irq_index = pio_irq - ((pio == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0); // Get index of the IRQ
		// pio_set_irqn_source_enabled(pio, irq_index, pis_sm0_rx_fifo_not_empty + sm, true); // Set pio to tell us when the FIFO is NOT empty
	}

	// Initialize the state machine
	pio_sm_init(glitch->pio, glitch->sm, glitch->prog_offs, &c);
	pio_sm_set_enabled(glitch->pio, glitch->sm, true);
}

bool glitch_init(PIO pio, glitch_t *glitch);

#endif // _GLITCH_H