import board
import busio
import digitalio
import os
import sdcardio
import storage
import time

import spot_filter


def main():
    spot_filter.init(512, 1028)

    spi = busio.SPI(board.SD_SCK, MOSI=board.SD_MOSI, MISO=board.SD_MISO)
    cs = board.SD_CS

    sdcard = sdcardio.SDCard(spi, cs)
    vfs = storage.VfsFat(sdcard)
    storage.mount(vfs, "/sd")

    led = digitalio.DigitalInOut(board.LED)
    led.direction = digitalio.Direction.OUTPUT

    # file is 8 * 512 * 1028 * 2 bytes

    buffer = bytearray(1028 * 2)

    for j in range(81):
        filename = f"/sd/data/frame_{j:05d}.raw"
        with open(filename, "rb") as fin:
            t0 = time.time()
            signal = 0
            spots = 0
            for i in range(8):
                for j in range(512):
                    fin.readinto(buffer, 1028 * 2)
                    signal += spot_filter.row(buffer)
                # last 3 rows
                for j in range(3):
                    signal += spot_filter.row(buffer)
                spots += spot_filter.reset()
                led.value = not led.value

            t1 = time.time()

            print(f"{filename} => {signal} pixels / {spots} spots in {(t1 - t0):d}s")

    spot_filter.deinit()
    storage.umount("/sd")


main()
