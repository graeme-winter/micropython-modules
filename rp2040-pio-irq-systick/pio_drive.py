from uctypes import addressof
from machine import mem32, Pin
from array import array
import rp2
import time
import pio_irq_syst

# configure the scratch buffer
scratch = array("I", [0])
address = addressof(scratch)

# configure the count buffer
count = array("I", [0 for j in range(100)])

# pins
pins = [Pin(j, Pin.OUT) for j in (0, 1)]

# DREQ definitions
DREQ_PIO0_RX0 = 4 << 15

# register definitions
PIO0_FLEVEL = 0x50200000 | 0xC
PIO0_RXF0 = 0x50200000 | 0x20
PIO0_INTE = 0x50200000 | 0x12C

# systick registers
PPB_BASE = 0xE0000000
SYST_CSR = PPB_BASE | 0xE010
SYST_CVR = PPB_BASE | 0xE018

# DMA registers
CH0_READ_ADDR = 0x50000000 | 0x0
CH0_WRITE_ADDR = 0x50000000 | 0x4
CH0_TRANS_COUNT = 0x50000000 | 0x8
CH0_CTRL_TRIG = 0x50000000 | 0xC

QUIET = 0x1 << 21
DATA_SIZE = 0x2 << 2
ENABLE = 0x1

NN = 1_000_000

BUSY = 0x1 << 24


@rp2.asm_pio(sideset_init=rp2.PIO.OUT_LOW)
def tick():
    mov(x, invert(null))
    label("start")
    jmp(x_dec, "cmp")
    label("cmp")
    mov(isr, invert(x))
    push().side(0)
    jmp(x_not_y, "start")
    irq(0).side(1)
    mov(x, invert(null))
    jmp("start")


# drain FIFO
while mem32[PIO0_FLEVEL] & 0xF0:
    scr = mem32[PIO0_RXF0]

# set up DMA
mem32[CH0_READ_ADDR] = PIO0_RXF0
mem32[CH0_WRITE_ADDR] = address
mem32[CH0_TRANS_COUNT] = NN
mem32[CH0_CTRL_TRIG] = QUIET + DREQ_PIO0_RX0 + DATA_SIZE + ENABLE

# set up systick - reset the counter and set the systick to
# emabled | clock driven (125 MHz)
mem32[SYST_CVR] = 0
mem32[SYST_CSR] = 5


# useless handler - is never called
def nope(sm):
    pass


sm = rp2.StateMachine(0, tick, sideset_base=pins[0])
sm.irq(nope)

# clock a static value into y for comparison
sm.put(10000)
sm.exec("pull()")
sm.exec("mov(y, invert(osr))")

# enable interrupt (un)mask
mem32[PIO0_INTE] = 0x1 << 8

pio_irq_syst.init(addressof(count), 100)
sm.active(1)

while mem32[CH0_CTRL_TRIG] & BUSY:
    continue

sm.active(0)
pio_irq_syst.deinit()

# disable systick
mem32[SYST_CSR] = 0

for j in range(1, 100):
    print(j, count[j - 1] - count[j])
