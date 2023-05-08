from machine import Pin, UART
import time

import spot_filter


def main():

    spot_filter.init(512, 1028)

    uart = UART(1, 115200 * 8, rx=Pin("D1"), tx=Pin("D0"), timeout=1000)

    time.sleep(5)

    led = Pin("D13", Pin.OUT)

    buffer = bytearray(1028 * 2)

    frame = 0

    while True:
        t0 = time.ticks_ms()
        signal = 0
        for i in range(18):
            for j in range(512):
                uart.readinto(buffer)
                signal += spot_filter.row(buffer)
                if j >= 3:
                    uart.write(buffer)
            # last 3 rows
            for j in range(3):
                signal += spot_filter.row(buffer)
                uart.write(buffer)
            led.toggle()

        t1 = time.ticks_ms()

        print(f"Frame {frame} => {signal} signal pixels in {0.001*(t1 - t0):.2f}s")
        frame += 1

    spot_filter.deinit()


main()
