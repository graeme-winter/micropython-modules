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

    buffer = bytearray(1028 * 2)

    for j in range(20):
        filename = f"/sd/frame_{j:05d}.raw"
        with open(filename, "rb") as fin:
            t0 = time.ticks_ms()
            signal = 0
            spots = 0
            for i in range(18):
                for j in range(512):
                    fin.readinto(buffer, 1028 * 2)
                    signal += spot_filter.row(buffer)
                # last 3 rows
                for j in range(3):
                    signal += spot_filter.row(buffer)
                led.toggle()
                spots += spot_filter.reset()
            t1 = time.ticks_ms()

            print(
                f"{filename} => {signal} pixels / {spots} spots in {0.001*(t1 - t0):.2f}s"
            )

    os.umount("/sd")

    spot_filter.deinit()


main()
