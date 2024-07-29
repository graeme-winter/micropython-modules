from uctypes import addressof
import time

from mandelbrot import mandelbrot

import hashlib
import binascii


def main():
    # scratch area for working
    data = bytearray(1280 * 4)
    address = addressof(data)

    led = Pin("D13")

    # compute shasum => proof of work
    hash = hashlib.sha1()

    for i in range(1280):
        led.toggle()
        Ci = (-5 << 22) + 0x4000 + i * 0x8000
        mandelbrot(address, Ci)
        hash.update(data)
    assert (
        "5e13ee4c948261d71a2070f3e07a06233275d1e5"
        == binascii.hexlify(hash.digest()).decode()
    )


t0 = time.ticks_us()
main()
t1 = time.ticks_us()
print(f"{1e-6*(t1 - t0):.2f}")
