#!/usr/bin/env python

import argparse
import struct
import time

import serial

CMD = {
	'DELAY'				: b'D',
	'WIDTH'				: b'W',
	'GLITCH'			: b'G',
	'TRIG_OUT_EN'		: b'O',
	'TRIG_OUT_DIS'		: b'o',
	'TRIG_IN_RISING'	: b'I',
	'TRIG_IN_FALLING'	: b'i',
	'PING'				: b'P',
	'POWERON'			: b'+',
	'POWEROFF'			: b'-',
}
RESP = {
	'OK'				: b'k',
	'KO'				: b'x',
	'PONG'				: b'p',
	'GLITCH_FAIL'		: b'.',
}

def success() -> bool:
	# TODO: Implement
	return False

def main(args):

	def reboot_target(s: serial.Serial) -> bool:
		s.write(CMD['POWEROFF'])
		r = s.read(len(RESP['OK']))
		if r != RESP['OK']:
			print(f'[!] Could not power off the target. Got:\n{r}\nAborting.')
			return False
		s.write(CMD['POWERON'])
		r = s.read(len(RESP['OK']))
		if r != RESP['OK']:
			print(f'[!] Could not power on the target. Got:\n{r}\nAborting.')
			return False
		return True

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
	print('[+] PicoMax available.')

	if args.output_trigger:
		s.write(CMD['TRIG_OUT_EN'])
		r = s.read(len(RESP['OK']))
		if r != RESP['OK']:
			print(f'[!] Could not enable output trigger. Got:\n{r}\nAborting.')
			exit(1)
		print('[+] Output trigger enabled.')
	else:
		print('[+] Output trigger disabled.')

	if args.rising_edge_trigger:
		s.write(CMD['TRIG_IN_RISING'])
		r = s.read(len(RESP['OK']))
		if r != RESP['OK']:
			print(f'[!] Could not set trigger input to rising edge. Got:\n{r}\nAborting.')
			exit(1)
		print('[+] Input trigger set to rising edge.')
	else:
		print('[+] Input trigger set to falling edge.')

	if not reboot_target(s):
		exit(1)

	start = time.time()
	i = 0
	for i, (d, w) in enumerate(((x, y) for x in range(*args.delay) for y in range(*args.width))):
		if i % 10 == 0:
			# print(f'[.] Sending {i}', end='\r', flush=True)
			print('.', end='', flush=True)

		s.write(CMD['DELAY'] + struct.pack('<i', d)) # Pi Pico defaults to little endian
		r = s.read(len(CMD['DELAY']))
		s.write(CMD['WIDTH'] + struct.pack('<i', w))
		r = s.read(len(CMD['WIDTH']))
		s.write(CMD['GLITCH'])
		r = s.read(len(RESP['OK']))
		# print(r.decode('ascii'), end='', flush=True)
		if r == RESP['GLITCH_FAIL']: # Glitch failed
			print(RESP['GLITCH_FAIL'].decode('ascii'), end='', flush=True)
		elif r == RESP['KO']: # Target dead
			print(RESP['KO'].decode('ascii'), end='', flush=True)
			if not reboot_target(s):
				break
		elif r == RESP['OK']: # Glitched
			print(f'\n[!] SUCCESS! Settings: delay={d}, width={w}')
			break
		else:
			print(f'\n[!] Unknown response: {r}')

	end = time.time()
	print()
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
	parser.add_argument('-r', '--rising-edge-trigger', action='store_true',
						help='trigger glitch on input rising edge (default: falling edge)')
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
