#define CMD_DELAY			'D' // Accepts 4 bytes (little endian) of delay value
#define CMD_WIDTH			'W' // Accepts 4 bytes (little endian) of pulse width value
#define CMD_GLITCH			'G'
#define CMD_TRIG_OUT_EN		'O'
#define CMD_TRIG_OUT_DIS	'o'
#define CMD_TRIG_IN_EN		'I'
#define CMD_TRIG_IN_DIS		'i'
#define CMD_PING			'P'
#define RESP_OK				'k'
#define RESP_KO				'x'
#define RESP_PONG			'p'

#define MAX_EN_PIN			0x02 // Rpi pico pin 4
#define MAX_EN_MASK			(1 << MAX_EN_PIN)
#define MAX_SEL_PIN			0x03 // pin 5
#define MAX_SEL_MASK		(1 << MAX_SEL_PIN)
#define TRIG_IN_PIN			0x04 // pin 6
#define TRIG_IN_MASK		(1 << TRIG_IN_PIN)
#define TRIG_OUT_PIN		0x05 // pin 7
#define TRIG_OUT_MASK		(1 << TRIG_OUT_PIN)

#define SET_ATOMIC_OFFST	0x2000 // Set on write atomic access ofset (see rp2040 datasheet 2.1.2)
#define CLR_ATOMIC_OFFST	0x3000 // Clear on write

#define GPIO_ATOMIC			(SIO_BASE + SIO_GPIO_OUT_OFFSET)
#define SET_GPIO_ATOMIC		(SIO_BASE + SIO_GPIO_OUT_SET_OFFSET)
#define CLR_GPIO_ATOMIC		(SIO_BASE + SIO_GPIO_OUT_CLR_OFFSET)
#define XOR_GPIO_ATOMIC		(SIO_BASE + SIO_GPIO_OUT_XOR_OFFSET)

#define EN_PIN_PAD_CTRL		(IO_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET)
