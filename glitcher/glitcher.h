#define CMD_DELAY			'D'
#define CMD_WIDTH			'W'
#define CMD_GLITCH			'G'
#define CMD_TRIG_OUT		'O'
#define CMD_TRIG_IN			'I'
#define CMD_PING			'P'
#define RESP_TRIG_OUT		'o'
#define RESP_GLITCH_DONE	'g'
#define RESP_PONG			'p'

#define MAX_EN_PIN			0x02 // Rpi pico pin 4
#define MAX_EN_HI_MASK		(1 << MAX_EN_PIN)
#define MAX_EN_LO_MASK		(0 << MAX_EN_PIN)
#define MAX_SEL_PIN			0x03 // pin 5
#define MAX_SEL_HI_MASK		(1 << MAX_SEL_PIN)
#define MAX_SEL_LO_MASK		(0 << MAX_SEL_PIN)
#define TRIG_IN_PIN			0x04 // pin 6
#define TRIG_IN_MASK		(1 << TRIG_IN_PIN)
#define TRIG_OUT_PIN		0x05 // pin 7
#define TRIG_OUT_HI_MASK	(1 << TRIG_OUT_PIN)
#define TRIG_OUT_LO_MASK	(0 << TRIG_OUT_PIN)

#define SET_ATOMIC_OFFST	0x2000 // Set on write atomic access ofset (see rp2040 datasheet 2.1.2)
#define CLR_ATOMIC_OFFST	0x3000 // Clear on write

#define GPIO_ATOMIC			(SIO_BASE + SIO_GPIO_OUT_OFFSET)
#define SET_GPIO_ATOMIC		(SIO_BASE + SIO_GPIO_OUT_SET_OFFSET)
#define CLR_GPIO_ATOMIC		(SIO_BASE + SIO_GPIO_OUT_CLR_OFFSET)
#define XOR_GPIO_ATOMIC		(SIO_BASE + SIO_GPIO_OUT_XOR_OFFSET)

#define EN_PIN_PAD_CTRL		(IO_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET)
