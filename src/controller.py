#!/usr/bin/env python

import argparse
import struct
import subprocess
import threading
import time

import serial

CMD = {
	'DELAY'			: b'D',
	'WIDTH'			: b'W',
	'GLITCH'		: b'G',
	'TRIG_OUT_EN'	: b'O',
	'TRIG_OUT_DIS'	: b'o',
	'TRIG_IN_EN'	: b'I',
	'TRIG_IN_DIS'	: b'i',
	'PING'			: b'P',
}
RESP = {
	'OK'		: b'k',
	'KO'		: b'x',
	'PONG'		: b'p',
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
	r = s.read(len(RESP['PONG']))
	if r != RESP['PONG']:
		print(f'[!] Could not ping the glitcher. Got:\n{r}\nAborting.')
		exit(1)
	print('[+] Glitcher available.')

	if args.output_trigger:
		s.write(CMD['TRIG_OUT_EN'])
		r = s.read(len(RESP['OK']))
		if r != RESP['OK']:
			print(f'[!] Could not enable output trigger. Got:\n{r}\nAborting.')
			exit(1)
		print('[+] Output trigger enabled.')

	start = time.time()
	i = 0
	for i, (d, w) in enumerate(((x, y) for x in range(*args.delay) for y in range(*args.width))):
		if i % 10 == 0:
			print(f'[.] Sending {i}', end='\r', flush=True)

		s.write(CMD['DELAY'] + struct.pack('<i', d)) # Pi Pico defaults to little endian
		s.read(len(CMD['DELAY']))
		s.write(CMD['WIDTH'] + struct.pack('<i', w))
		s.read(len(CMD['WIDTH']))
		s.write(CMD['GLITCH'])
		r = s.read(len(RESP['OK']))
		if r != RESP['OK']:
			print(f'[!] Glitch failed. Got:\n{r}\nAborting.')
			exit(1)
	end = time.time()

	print(f'[+] Sent {i} in {end - start}s.')

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
