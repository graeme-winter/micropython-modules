import time
import irq
from machine import Pin, PWM, mem32

IO_BANK0_BASE = 0x40028000
PROC0_INTE0 = IO_BANK0_BASE | 0x248

irq.init()

pwm = PWM(Pin(0))
pin = Pin(1, Pin.IN)
out = Pin(2, Pin.OUT)

pwm.freq(1000)
pwm.duty_u16(0x8000)

mem32[PROC0_INTE0] = 0xc0

time.sleep(60)

mem32[PROC0_INTE0] = 0x0

irq.deinit()

