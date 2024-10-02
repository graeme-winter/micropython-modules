import sys

import numpy
import serial
import bitshuffle

dev = "/dev/tty.usbmodem14232302"

uart = serial.Serial(dev, baudrate=115200 * 8)

NZ = 18
NY = 512
NX = 1028

uint16_t = numpy.zeros((1, 1), dtype=numpy.uint16).dtype

for filename in sys.argv[1:]:
    frame = bitshuffle.decompress_lz4(
        numpy.fromfile(filename, dtype=numpy.uint8), shape=(NZ, NY, NX), dtype=uint16_t
    )
    signal = numpy.zeros(shape=(NZ, NY, NX), dtype=uint16_t)
    for module in range(18):
        # preload the array
        for j in range(3):
            row = frame[module, j, :].tobytes()
            uart.write(row)
        # continue
        for j in range(3, NY):
            row = frame[module, j, :].tobytes()
            uart.write(row)
            signal[module, j - 3, :] = numpy.frombuffer(
                uart.read(NX * 2), dtype=uint16_t
            )
        # finish
        for j in range(NY, NY + 3):
            signal[module, j - 3, :] = numpy.frombuffer(
                uart.read(NX * 2), dtype=uint16_t
            )
    print(f"{filename}: {numpy.count_nonzero(signal)}")
