#!/usr/bin/env python

import argparse
import struct
import subprocess
import threading
import time

import serial

CMD = {
	'DELAY'		: b'D',
	'WIDTH'		: b'W',
	'GLITCH'	: b'G',
	'TRIG_OUT'	: b'O',
	'TRIG_IN'	: b'I',
	'PING'		: b'P',
}
RESP = {
	'GLITCH'	: b'g',
	'TRIG_OUT'	: b'o',
	'PING'		: b'p',
}

def success() -> bool:
	# TODO: Implement
	return False

def main(args):
	try: 
		s = serial.Serial(port=args.port, baudrate=args.baud, timeout=args.timeout)
	except Exception as e:
		print(f'[!] Could not open serial port. Got:\n{e}\nAborting.')
		exit(1)

	s.write(CMD['PING'])
	r = s.read(len(RESP['PING']))
	if r != RESP['PING']:
		print(f'[!] Could not ping the glitcher. Got:\n{r}\nAborting.')
		exit(1)
	print('[+] Glitcher available.')

	if args.output_trigger:
		s.write(CMD['TRIG_OUT'])
		r = s.read(len(RESP['TRIG_OUT']))
		if r != RESP['TRIG_OUT']:
			print(f'[!] Could not enable output trigger. Got:\n{r}\nAborting.')
			exit(1)
		print('[+] Output trigger enabled.')

	for i, (d, w) in enumerate(((x, y) for x in range(*args.delay) for y in range(*args.width))):
		if i % 10 == 0:
			print(f'[+] Sending {i}', end='\r', flush=True)

		# s.write(cmd['DELAY'] + struct.pack('<i', d)) # Pi Pico defaults to little endian on boot
		# # s.read(100) # TODO remove
		# s.write(cmd['WIDTH'] + struct.pack('<i', w))
		# # s.read(100) # TODO remove
		s.write(b'G')
		r = s.read(len(RESP['GLITCH']))
		if r != RESP['GLITCH']:
			print(f'[!] Glitch failed. Got:\n{r}\nAborting.')
			exit(1)
		# s.read(100) # TODO remove
	
	print() # newline
	print('[+] Done.')


	s.close()

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='PicoMax glitcher controller', formatter_class=argparse.RawTextHelpFormatter)
	parser.add_argument('-p', '--port', type=str, default='/dev/ttyACM0',
						help='Serial port of the glitcher (default: /dev/ttyACM0)')
	parser.add_argument('-b', '--baud', type=int, default=115200,
						help='serial baudrate (default: 115200)')
	parser.add_argument('-t', '--timeout', type=float, default=0.1,
						help='serial timeout (default: 0.1s)')
	parser.add_argument('-o', '--output-trigger', action='store_true',
		     			help='enable output trigger (default: False)')
	parser.add_argument('-d', '--delay', type=int, nargs=3, default=[1,100,1],
						help=
'''delay for the pulse [min max step] (default: 1 100 1 glitcher clock cycles)
Note: max is not inclusive, i.e. the range is [min, max)''')
	parser.add_argument('-w', '--width', type=int, nargs=3, default=[1,100,1],
						help=
'''glitch pulse width [min max step] (default: 1 100 1 glitcher clock cycles)
Note: max is not inclusive, i.e. the range is [min, max)''')
	args = parser.parse_args()
	main(args)