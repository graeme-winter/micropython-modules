from machine import Pin, PWM
import time

# don't have a lot of choice over the LED pin

led = Pin("D13", Pin.OUT)

# these are chosen to use GPIO1/0 and GPIO1/1 respectively
# i.e. B0_00 and B0_01 on the schematic - connect a jumper
# from D13 to D10, then 'scope to D13, D12 to plot the delay

sense = Pin("D10", Pin.IN)
out = Pin("D12", Pin.OUT)

out.on()


@micropython.viper
def irq(stuff):
    out.toggle()


sense.irq(handler=irq, trigger=(Pin.IRQ_RISING | Pin.IRQ_FALLING), hard=True)

# clobber existing IRQ
import drive_irq

drive_irq.init()

pwm = PWM(led)
pwm.freq(1000)
pwm.duty_ns(10_000)

print("Sleeping... 100s")
time.sleep(100)
drive_irq.deinit()
