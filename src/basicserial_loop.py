import serial

try: 
	s = serial.Serial(port='/dev/ttyACM0', baudrate=115200, timeout=1)
except Exception as e:
	print(f'[!] Could not open serial port. Got:\n{e}\nAborting.')
	exit(1)

# while True:
# 	s.write(b'L')
# 	r = s.read(1)
# 	if r == b'K':
# 		print('OK')
# 		r = s.read(2)
# 		print(r)
# 	else:
# 		print(f'NOT OK: {r}')

s.write(b'O') # Output trigger enable
r = s.read(1)
print(f'Output trigger enable: {r}')

s.write(b'I') # Rising edge trigger
r = s.read(1)
print(f'Rising edge trigger: {r}')

for _ in range(20):
	s.write(b'L')
	r = s.read(1)
	if r == b'k':
		print('OK')
		r = s.read(2)
		print(r)
	else:
		print(f'NOT OK: {r}')

