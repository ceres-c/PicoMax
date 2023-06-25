#!/usr/bin/env python

import icsp
from icsp import devices

from hexdump import hexdump

icsp = icsp.ICSP(devices.PIC16F1936 , '/dev/ttyACM0', 115200, 0.1, 5.0)
hexdump(icsp.read_program(0, 1))
