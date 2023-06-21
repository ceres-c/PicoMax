#include <stdint.h> // TODO remove if not needed



typedef uint8_t uint6_t;



typedef struct imperative_s {
	uint6_t command;
} imperative_t;

// all sizes should be 16bits


typedef struct command_s {
	uint8_t command_type; 	// This is for pio to pick which state macchine to use
	uint8_t command;		// ICSP command bitmask	
	uint16_t payload;		// Load data payload
} command_t;
