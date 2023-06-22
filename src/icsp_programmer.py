#!/usr/bin/env python

import argparse
import struct

import hexdump
import serial

BYTES_PER_WORD = 2
MEM_OP_TIMEOUT = 5 # Serial timeout used for memory reads/writes

CMD = {
	'PING'			: b'P',
	'READ_DATA'		: b'R',
	'READ_PROG'		: b'r',
}
RESP = {
	'OK'			: b'k',
	'KO'			: b'x',
	'PONG'			: b'p',
}

def pic_read_data(s: serial.Serial, addr: int, size: int, timeout: int) -> bytes:
	s.write(CMD['READ_DATA'] + struct.pack('<i', addr) + struct.pack('<i', size)) # Pi Pico defaults to little endian
	s.write(struct.pack('<i', addr)) # Pi Pico defaults to little endian
	s.write(struct.pack('<i', size))
	r = s.read(len(RESP['OK']))
	if r != RESP['OK']:
		raise Exception(f'[!] Could not allocate memory on the programmer to read data memory. Got:\n{r}\nAborting.')

	s.timeout = MEM_OP_TIMEOUT
	r = s.read(len(RESP['OK']))
	if r != RESP['OK']:
		raise Exception(f'[!] The programmer could not read data. Got:\n{r}\nAborting.')
	
	r = s.read(size * BYTES_PER_WORD)
	s.timeout = timeout # Reset timeout

	return r

def pic_read_program(s: serial.Serial, addr: int, size: int, timeout: int) -> bytes:
	# s.write(CMD['READ_PROG']) # Pi Pico defaults to little endian
	s.write(CMD['READ_PROG'] + struct.pack('<i', addr) + struct.pack('<i', size)) # Pi Pico defaults to little endian
	r = s.read(len(RESP['OK']))
	if r != RESP['OK']:
		raise Exception(f'[!] Could not allocate memory on the programmer to read program memory. Got:\n{r}\nAborting.')

	s.timeout = MEM_OP_TIMEOUT
	r = s.read(len(RESP['OK']))
	if r != RESP['OK']:
		raise Exception(f'[!] The programmer could not read program. Got:\n{r}\nAborting.')

	r = s.read(size * BYTES_PER_WORD)
	s.timeout = timeout # Reset timeout

	return r

def main(args):

	try: 
		s = serial.Serial(port=args.port, baudrate=args.baud, timeout=args.timeout)
	except Exception as e:
		print(f'[!] Could not open serial port. Got:\n{e}\nAborting.')
		exit(1)

	s.write(CMD['PING'])
	r = s.read(len(RESP['PONG']))
	if r != RESP['PONG']:
		print(f'[!] Could not ping the programmer. Got:\n{r}\nAborting.')
		exit(1)
	print('[+] Programmer available.')

	if args.read_data:
		addr, size = args.read_data
		dump = pic_read_data(s, addr, size, args.timeout)
		hexdump.hexdump(dump)
	elif args.read_program:
		addr, size = args.read_program
		dump = pic_read_program(s, addr, size, args.timeout)
		hexdump.hexdump(dump)
	
	print('[+] Done.')

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='PIC programmer controller', formatter_class=argparse.RawTextHelpFormatter)
	parser.add_argument('-p', '--port', type=str, default='/dev/ttyACM0',
						help='Serial port of the programmer (default: /dev/ttyACM0)')
	parser.add_argument('-b', '--baud', type=int, default=115200,
						help='serial baudrate (default: 115200)')
	parser.add_argument('-t', '--timeout', type=float, default=0.1,
						help='serial timeout (default: 0.1s)')
	parser.add_argument('--read-data', type=lambda x: int(x,0), nargs=2, default=None,
						help=
'''read the PIC data memory [start_address size] (in bytes)
Note: max is not inclusive, i.e. the range is [start_address size)''')
	parser.add_argument('--read-program', type=lambda x: int(x,0), nargs=2, default=None,
						help=
'''read the PIC program memory [start_page page_number] (in 14-bit words)
Note: max is not inclusive, i.e. the range is [start_page page_number)''')
	args = parser.parse_args()
	main(args)
