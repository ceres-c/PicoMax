from .devices import *

import struct

import serial

BYTES_PER_WORD = 2

CMD_PING			= b'P'
CMD_POWERON			= b'+'
CMD_POWEROFF		= b'-'
CMD_READ_DATA		= b'R'
CMD_READ_PROG		= b'r'
CMD_WRITE_DATA		= b'W'
CMD_WRITE_PROG		= b'w'
CMD_ERASE_BULK_DATA	= b'E'
CMD_ERASE_BULK_PROG	= b'e'
CMD_ERASE_ROW_PROG	= b'@'

RESP_OK				= b'k'
RESP_KO				= b'x'
RESP_PONG			= b'p'

class ICSP():
	def __init__(self, device: PIC16F, port: str, baudrate: int, default_timeout: float = 0.1, extra_timeout: float = 5.0) -> None:
		self.device = device
		self.port = port
		self.baudrate = baudrate
		self.default_timeout = default_timeout
		self.extra_timeout = extra_timeout

		self.s = serial.Serial(port=self.port, baudrate=self.baudrate, timeout=self.default_timeout)

	def ping(self) -> (bool, bytes):
		self.s.write(CMD_PING)
		r = self.s.read(len(RESP_PONG))
		return (r == RESP_PONG, r)

	def read_data(self, start_page: int, length: int) -> bytes:
		self.s.write(CMD_READ_DATA)
		self.s.write(struct.pack('<I', start_page))
		self.s.write(struct.pack('<I', length))
		r = self.s.read(len(RESP_OK))
		if r != RESP_OK:
			raise Exception(f'[!] Could not allocate memory on the programmer to read data memory. Got:\n{r}\nAborting.')
		
		self.s.timeout = self.extra_timeout
		r = self.s.read(len(RESP_OK))
		if r != RESP_OK:
			raise Exception(f'[!] The programmer could not read data. Got:\n{r}\nAborting.')
		
		r = self.s.read(length)
		self.s.timeout = self.default_timeout

		return r

	def read_program(self, start_addr: int, length: int) -> bytes:
		self.s.write(CMD_READ_PROG)
		self.s.write(struct.pack('<I', start_addr))
		self.s.write(struct.pack('<I', length))
		r = self.s.read(len(RESP_OK))
		if r != RESP_OK:
			raise Exception(f'[!] Could not allocate memory on the programmer to read program memory. Got:\n{r}\nAborting.')

		self.s.timeout = self.extra_timeout
		r = self.s.read(len(RESP_OK))
		if r != RESP_OK:
			raise Exception(f'[!] The programmer could not read program. Got:\n{r}\nAborting.')

		r = self.s.read(length * BYTES_PER_WORD)
		self.s.timeout = self.default_timeout

		return r

	def read_config(self) -> dict:
		conf_mem_data = self.read_program(self.device.config_memory.addr, self.device.config_memory.size)
		ret = {}
		for id, offset in vars(self.device.config_memory.config):
			ret[id.replace('_offset', '')] = struct.unpack('<H', _slice_data(conf_mem_data, offset, 1))[0]
		return ret

	def read_device_ID(self) -> DeviceID:
		r = self.read_program(self.device.config_memory.addr + self.device.config_memory.config.device_ID_offset, 1)
		id = struct.unpack('<H', r)[0]
		return DeviceID(
			dev = id >> (self.device.word_size - self.device.device_ID.dev_size),
			rev = id & ((1 << self.device.device_ID.rev_size))
		)

	# Checks the device ID of the chip connected to the programmer
	def check_device_ID(self) -> bool:
		r = self.read_device_ID()
		id = struct.unpack('<H', r)[0]
		return id >> (self.device.word_size - self.device.device_ID.dev_size) == self.device.device_ID.dev

	# Checks if a given device ID matches the one in the database
	def check_device_ID_offline(self, id: int) -> bool:
		return id == self.device.device_ID.dev

	def erase_bulk_program(self) -> None:
		self.s.write(CMD_ERASE_BULK_PROG)
		r = self.s.read(len(RESP_OK))
		if r != RESP_OK:
			raise Exception(f'[!] The programmer could not erase the chip. Got:\n{r}\nAborting.')

	def erase_bulk_data(self) -> None:
		self.s.write(CMD_ERASE_BULK_DATA)
		r = self.s.read(len(RESP_OK))
		if r != RESP_OK:
			raise Exception(f'[!] The programmer could not erase the chip. Got:\n{r}\nAborting.')

	def erase_row_program(self, addr: int) -> None:
		self.s.write(CMD_ERASE_ROW_PROG)
		self.s.write(struct.pack('<I', addr))
		self.s.timeout = self.extra_timeout
		r = self.s.read(len(RESP_OK))
		if r != RESP_OK:
			raise Exception(f'[!] The programmer could not erase the required row. Got:\n{r}\nAborting.')
		self.s.timeout = self.default_timeout

	def write_program(self, start_addr: int, data: bytes) -> None:
		if len(data) != BYTES_PER_WORD:
			raise Exception(f'[!] The programmer can only write one word at a time. Got:\n{len(data)} bytes\nAborting.')

		self.s.timeout = self.extra_timeout
		self.s.write(CMD_WRITE_PROG)
		self.s.write(struct.pack('<I', start_addr))
		self.s.write(data)

		r = self.s.read(len(RESP_OK))
		if r != RESP_OK:
			raise Exception(f'[!] The programmer could not write program. Got:\n{r}\nAborting.')
		self.s.timeout = self.default_timeout

def _slice_data(data: bytes, start: int, length: int) -> bytes:
	return data[start * BYTES_PER_WORD : start * BYTES_PER_WORD + length * BYTES_PER_WORD]
