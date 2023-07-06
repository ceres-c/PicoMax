#!/usr/bin/env python

import argparse
import struct

import hexdump

import icsp
from icsp import devices

MEM_OP_TIMEOUT = 5.0 # Serial timeout used for memory reads/writes

def main(args):

	try: 
		pic = icsp.ICSP(devices.PIC16LF1936 , args.port, args.baud, args.timeout, MEM_OP_TIMEOUT)
	except Exception as e:
		print(f'[!] Could not open serial port. Got:\n{e}\nAborting.')
		exit(1)

	connected, res = pic.ping()
	if not connected:
		print(f'[!] Could not ping the programmer. Got:\n{res}\nAborting.')
		exit(1)
	print('[+] Programmer available.')

	if args.read_data:
		dump = pic.read_data(*args.read_data)
		hexdump.hexdump(dump)
	elif args.read_program:
		dump = pic.read_program(*args.read_program)
		hexdump.hexdump(dump)
	elif args.write_program:
		print('[?] Have you erased the PIC program memory first? Otherwise, the write will silently fail.')
		addr, data = args.write_program
		data = struct.pack('<H', data)
		pic.write_program(addr, data)
	elif args.erase_bulk_program:
		pic.erase_bulk_program()
	elif args.erase_bulk_data:
		pic.erase_bulk_data()
	elif args.erase_row_program:
		pic.erase_row_program(*args.erase_row_program)
	elif args.device_id:
		id = pic.read_device_ID()
		print(f'[+] Device ID: {id.dev:b}')
		print(f'[+] Revision: {id.rev:b}')
		if pic.check_device_ID_offline(id.dev):
			print('[+] Device ID is valid.')
		else:
			print('[!] Device ID is invalid.')
	elif args.config:
		conf = pic.read_config()
		for k,v in conf.items():
			print(f'{k}: {v}')

	print('[+] Done.')

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='PIC programmer controller', formatter_class=argparse.RawTextHelpFormatter)
	parser.add_argument('-p', '--port', type=str, default='/dev/ttyACM0',
						help='Serial port of the programmer (default: /dev/ttyACM0)')
	parser.add_argument('-b', '--baud', type=int, default=115200,
						help='serial baudrate (default: 115200)')
	parser.add_argument('-t', '--timeout', type=float, default=0.1,
						help='serial timeout (default: 0.1s)')
	parser.add_argument('--read-data', type=lambda x: int(x,0), nargs=2, default=None, # Not implemented yet
						help=
'''read the PIC data memory [start_address size] (in bytes)
Note: max is not inclusive, i.e. the range is [start_address size)''')
	parser.add_argument('--read-program', type=lambda x: int(x,0), nargs=2, default=None,
						help=
'''read the PIC program memory [start_page page_number] (in 14-bit words)
Note: max is not inclusive, i.e. the range is [start_page page_number)''')
	# TODO write-data
	parser.add_argument('--write-program', type=lambda x: int(x,0), nargs=2, default=None,
						help='write the PIC program memory [start_page data] (start_page in 14-bit words, data as a 16 bits hex number)')
	parser.add_argument('--erase-bulk-program', action='store_true', default=False,
						help='erase the PIC program memory')
	parser.add_argument('--erase-bulk-data', action='store_true', default=False,
						help='erase the PIC data memory')
	parser.add_argument('--erase-row-program', type=lambda x: int(x,0), nargs=1, default=None,
		     			help='erase the entire row of the PIC program memory containing address [address] (address in 14-bit words)')
	parser.add_argument('--device-id', action='store_true', default=False,
						help='read the PIC device ID')
	parser.add_argument('--config', action='store_true', default=False,
						help='read the PIC device configuration')
	args = parser.parse_args()
	main(args)
