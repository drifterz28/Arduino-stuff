import board
import neopixel
import time

LED_PIN = board.IO35
LED_COUNT = 40
BRIGHTNESS = 0.25

strip = neopixel.NeoPixel(LED_PIN, LED_COUNT, brightness=BRIGHTNESS, auto_write=False)


def wipe(r, g, b, delay=0.02):
    for i in range(LED_COUNT):
        strip[i] = (r, g, b)
        strip.show()
        time.sleep(delay)


def fill(r, g, b, hold=0.5):
    strip.fill((r, g, b))
    strip.show()
    time.sleep(hold)


def chase(r, g, b, delay=0.05):
    for i in range(LED_COUNT):
        strip.fill((0, 0, 0))
        strip[i] = (r, g, b)
        strip.show()
        time.sleep(delay)


print("LED test starting on IO3 with", LED_COUNT, "pixels")

# 1 — wipe red, green, blue
print("Wipe red")
wipe(64, 0, 0)
print("Wipe green")
wipe(0, 64, 0)
print("Wipe blue")
wipe(0, 0, 64)

# 2 — solid white flash
print("White flash")
fill(64, 64, 64)
fill(0, 0, 0, 0.2)

# 3 — single pixel chase
print("Chase white")
chase(64, 64, 64, 0.03)

# 4 — all off
strip.fill((0, 0, 0))
strip.show()

print("LED test complete")
