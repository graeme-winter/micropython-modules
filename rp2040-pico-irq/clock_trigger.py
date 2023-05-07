from machine import mem32, Pin
import time

import irq

irq.init()

p20 = Pin(20, Pin.OUT)


def toggle(pin):
    p20.toggle()


toggle()

p19 = Pin(19, Pin.IN)

IO_BANK_BASE = 0x40014000
CLK_BASE = 0x40008000
GPIO21_CTRL = IO_BANK_BASE | 0xAC

mem32[GPIO21_CTRL] = 8  # GPCLK0
mem32[CLK_BASE] = 1 << 11

mem32[CLK_BASE | 4] = 125000 << 8

p19.irq(toggle, Pin.IRQ_RISING | Pin.IRQ_FALLING, hard=True)

time.sleep(60)

irq.deinit()
