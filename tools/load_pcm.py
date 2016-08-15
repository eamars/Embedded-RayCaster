import binascii

fp = open('aug-1.aiff', 'rb')

i = 0
while True:
	byte = fp.read(1)
	if not byte:
		break;
	print('0x{}'.format(binascii.hexlify(byte).decode('ascii')), end=', ')
	i += 1
	if (i % 16 == 0):
		print()