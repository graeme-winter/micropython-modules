from machine import Pin, SPI
import sdcard
import os
import time
import gc

import spot_filter


def main():

    spot_filter.init(512, 1028)

    sd = sdcard.SDCard(
        SPI(
            2,
            baudrate=8000000,
            mosi=Pin("SD_MOSI"),
            miso=Pin("SD_MISO"),
            sck=Pin("SD_SCK"),
        ),
        Pin("SD_CS"),
        baudrate=8000000,
    )

    led = Pin("D13", Pin.OUT)
    os.mount(sd, "/sd")

    # file is 18 * 512 * 1028 * 2 bytes
    with open("/sd/frame_00000.raw", "rb") as fin:

        buffer = bytearray(1028 * 2)

        t0 = time.time()
        signal = 0
        for i in range(18):
            for j in range(512 + 3):
                fin.readinto(buffer, 1028 * 2)
                signal += spot_filter.row(buffer)
                led.toggle()
            print(i, signal)

        t1 = time.time()

        print(f"Found {signal} signal pixels in {t1 - t0:.2f}s")

    os.umount("/sd")

    spot_filter.deinit()


main()
