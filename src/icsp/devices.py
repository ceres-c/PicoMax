from dataclasses import dataclass, replace

@dataclass(frozen=True)
class Memory:
	addr: int
	size: int

@dataclass(frozen=True)
class IDs:
	user_ID1_offset: int
	user_ID2_offset: int
	user_ID3_offset: int
	user_ID4_offset: int
	device_ID_offset: int
	config_word1_offset: int
	config_word2_offset: int
	calib_word1_offset: int
	calib_word2_offset: int

@dataclass(frozen=True)
class ConfigMemory(Memory):
	IDs: IDs

@dataclass(frozen=True)
class PIC16F:
	program_memory: Memory
	config_memory: ConfigMemory
	data_memory: Memory
	device_ID: int
	device_ID_size: int = 9
	word_size: int = 14

PIC16F1936 = PIC16F(
	program_memory=Memory(addr=0x0, size=0xFFF),
	config_memory=ConfigMemory(
		addr=0x8000,
		size=0x0A,
		IDs=IDs(
			user_ID1_offset=0x00,
			user_ID2_offset=0x01,
			user_ID3_offset=0x02,
			user_ID4_offset=0x03,
			device_ID_offset=0x06,
			config_word1_offset=0x07,
			config_word2_offset=0x08,
			calib_word1_offset=0x09,
			calib_word2_offset=0x0A)),
	data_memory=Memory(addr=0x0, size=0x100),
	device_ID=0b100011011
)
PIC16LF1936 = replace(PIC16F1936, device_ID=0b100100011)
