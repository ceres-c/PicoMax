#!/usr/bin/env python

import argparse
import itertools
import random
import struct
import sys
import threading
import time

import serial
from PyQt6.QtCore import QTimer
from PyQt6.QtWidgets import QApplication, QGridLayout, QMainWindow, QWidget
import pyqtgraph as pg  # Must be after PyQT import

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
	'GLITCH_WEIRD'		: b'y',
	'WTF'				: b'?',
}
QT_COLORS = {
	RESP['GLITCH_FAIL']:	(255, 0, 0),	# Red
	RESP['KO']:				(255, 255, 0),	# Yellow
	RESP['OK']:				(0, 255, 0),	# Green
	RESP['GLITCH_WEIRD']:	(255, 128, 0),	# Orange
	RESP['WTF']:			(0, 0, 255),	# Blue (used for unknown responses)
}

class Window(QMainWindow):
	def __init__(self, queue: list[tuple[int, int, bytes]]):
		self.queue = queue
		super().__init__()
		self.setWindowTitle("PicoMax Live Plotter")
		# self.setGeometry(100, 100, 600, 500)
		self.UiComponents()
		self.show()

	def UiComponents(self):
		widget = QWidget()

		pg.setConfigOption('background', 'w')
		plot = pg.plot()
		self.scatter = pg.ScatterPlotItem()
		plot.addItem(self.scatter)

		layout = QGridLayout()
		widget.setLayout(layout)
		layout.addWidget(plot, 0, 0, 3, 1)

		self.setCentralWidget(widget)

		# Timer to update the plot
		self.timer = QTimer()
		self.timer.setInterval(50)
		self.timer.timeout.connect(self.update_plot_data)
		self.timer.start()

	def update_plot_data(self):
		try:
			delay, width, result = self.queue.pop()
		except IndexError:
			# No new data
			return

		self.scatter.addPoints([{
			'pos': (delay, width),
			'size': 10,
			'pen': {'color': (0, 0, 0), 'width': .5},
			'brush': QT_COLORS[result],
		}])

class Glitcher(threading.Thread):
	def __init__(self, args: argparse.Namespace, queue: list[tuple[int, int, bytes]]):
		self.args = args
		self.queue = queue
		threading.Thread.__init__(self, daemon=True)


	def run(self):
		def poweroff_target(s: serial.Serial) -> bool:
			s.write(CMD['POWEROFF'])
			r = s.read(len(RESP['OK']))
			if r != RESP['OK']:
				print(f'[!] Could not power off the target. Got:\n{r}\nAborting.')
				return False
			return True

		try: 
			s = serial.Serial(port=self.args.port, baudrate=self.args.baud, timeout=self.args.timeout)
		except Exception as e:
			print(f'[!] Could not open serial port. Got:\n{e}\nAborting.')
			exit(1)

		s.write(CMD['PING'])
		r = s.read(len(RESP['PONG']))
		if r != RESP['PONG']:
			print(f'[!] Could not ping the glitcher. Got:\n{r}\nAborting.')
			exit(1)
		print('[+] PicoMax available.')

		if self.args.output_trigger:
			s.write(CMD['TRIG_OUT_EN'])
			r = s.read(len(RESP['OK']))
			if r != RESP['OK']:
				print(f'[!] Could not enable output trigger. Got:\n{r}\nAborting.')
				exit(1)
			print('[+] Output trigger enabled.')
		else:
			s.write(CMD['TRIG_OUT_DIS'])
			r = s.read(len(RESP['OK']))
			if r != RESP['OK']:
				print(f'[!] Could not disable output trigger. Got:\n{r}\nAborting.')
				exit(1)
			print('[+] Output trigger disabled.')

		if self.args.rising_edge_trigger:
			s.write(CMD['TRIG_IN_RISING'])
			r = s.read(len(RESP['OK']))
			if r != RESP['OK']:
				print(f'[!] Could not set trigger input to rising edge. Got:\n{r}\nAborting.')
				exit(1)
			print('[+] Input trigger set to rising edge.')
		else:
			s.write(CMD['TRIG_IN_FALLING'])
			r = s.read(len(RESP['OK']))
			if r != RESP['OK']:
				print(f'[!] Could not set trigger input to falling edge. Got:\n{r}\nAborting.')
				exit(1)
			print('[+] Input trigger set to falling edge.')

		if not poweroff_target(s):
			exit(1)

		start = time.time()

		prod = list(itertools.product(range(*self.args.delay), range(*self.args.width)))
		random.shuffle(prod)
		i = 0

		for d, w in prod:
			s.write(CMD['DELAY'] + struct.pack('<i', d)) # Pi Pico defaults to little endian
			r = s.read(len(CMD['DELAY']))
			s.write(CMD['WIDTH'] + struct.pack('<i', w))
			r = s.read(len(CMD['WIDTH']))
			if not poweroff_target(s):
				print(f'[!] Could not power off the target. Got:\n{r}\nAborting.')
				break
			s.write(CMD['GLITCH'])
			r = s.read(len(RESP['OK']))
			if r == RESP['GLITCH_FAIL']: # Glitch failed
				self.queue.append((d, w, RESP['GLITCH_FAIL']))
			elif r == RESP['GLITCH_WEIRD']: # Wrong deviceID
				self.queue.append((d, w, RESP['GLITCH_WEIRD']))
			elif r == RESP['KO']: # Target dead
				self.queue.append((d, w, RESP['KO']))
			elif r == RESP['OK']: # Glitched
				r = s.read(2)
				print(f'[+] WTF output: {struct.unpack("<H", r)[0]:x}')
				self.queue.append((d, w, RESP['OK']))
				input('Waiting to continue')
			else:
				print(f'[!] Unknown response: {r}')
				self.queue.append((d, w, RESP['WTF']))
			i += 1

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

	sync_list = []

	glitcher = Glitcher(args, sync_list)
	glitcher.start()

	App = QApplication(sys.argv)
	window = Window(sync_list)
	sys.exit(App.exec())

