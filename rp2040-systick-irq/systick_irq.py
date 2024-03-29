from machine import mem32, Pin
import time

import systick

led = Pin(25, Pin.OUT)

# enable interrupt handler
systick.init()

PPB_BASE = 0xE0000000
SYST_CSR = PPB_BASE | 0xE010
SYST_RVR = PPB_BASE | 0xE014
SYST_CVR = PPB_BASE | 0xE018

mem32[SYST_RVR] = 12_499_999

mem32[SYST_CSR] = 0x7
time.sleep(10)
mem32[SYST_CSR] = 0x0

# print number of cycles needed on last delay
print(12_499_999 - systick.deinit())
