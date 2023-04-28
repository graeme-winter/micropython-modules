from machine import mem32, Pin

# FIXME import irq module and init here

p20 = Pin(20, Pin.OUT)

def toggle(pin):
    p20.toggle()

p19 = Pin(19, Pin.IN)

IO_BANK_BASE = 0x40014000
CLK_BASE = 0x40008000
GPIO21_CTRL = IO_BANK_BASE | 0xac

mem32[GPIO21_CTRL] = 8 # GPCLK0
mem32[CLK_BASE] = 1 << 11

mem32[CLK_BASE | 4] = 12500000 << 8

p19.irq(toggle, Pin.IRQ_RISING | Pin.IRQ_FALLING, hard=True)

# FIXME irq deinit here after some time
