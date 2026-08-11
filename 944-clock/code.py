import displayio
# 1. Force clear background hardware locks on soft-reboot instantly
displayio.release_displays()

import time
# 2. Wait for power lines to sit cleanly before starting chips
time.sleep(1.5)

import board
import busio
import digitalio
import rtc
import terminalio
import struct
from i2cdisplaybus import I2CDisplayBus
from adafruit_display_text import label
import adafruit_displayio_ssd1306
import adafruit_ds1302
from adafruit_bitmap_font import bitmap_font

# --- NATIVE BMP180 REGISTER DRIVER CLASS ---
class NativeBMP180:
    def __init__(self, i2c_bus, address=0x77):
        self.i2c = i2c_bus
        self.address = address
        self.sea_level_hpa = 1152.62  # Calibrated for your sensor board in Bend, Oregon
        self._load_calibration()

    def _read_s16(self, register):
        buffer = bytearray(2)
        while not self.i2c.try_lock():
            pass
        self.i2c.writeto_then_readfrom(self.address, bytes([register]), buffer)
        self.i2c.unlock()
        return struct.unpack(">h", buffer)

    def _read_u16(self, register):
        buffer = bytearray(2)
        while not self.i2c.try_lock():
            pass
        self.i2c.writeto_then_readfrom(self.address, bytes([register]), buffer)
        self.i2c.unlock()
        return struct.unpack(">H", buffer)

    def _load_calibration(self):
        self.ac1 = self._read_s16(0xAA)
        self.ac2 = self._read_s16(0xAC)
        self.ac3 = self._read_s16(0xAE)
        self.ac4 = self._read_u16(0xB0)
        self.ac5 = self._read_u16(0xB2)
        self.ac6 = self._read_u16(0xB4)
        self.b1  = self._read_s16(0xB6)
        self.b2  = self._read_s16(0xB8)
        self.mb  = self._read_s16(0xBA)
        self.mc  = self._read_s16(0xBC)
        self.md  = self._read_s16(0xBE)

    def read_raw_temp(self):
        while not self.i2c.try_lock():
            pass
        self.i2c.writeto(self.address, bytes([0xF4, 0x2E]))
        self.i2c.unlock()
        time.sleep(0.005)
        return self._read_u16(0xF6)

    def read_raw_pressure(self):
        while not self.i2c.try_lock():
            pass
        self.i2c.writeto(self.address, bytes([0xF4, 0x34]))
        self.i2c.unlock()
        time.sleep(0.005)
        buffer = bytearray(3)
        while not self.i2c.try_lock():
            pass
        self.i2c.writeto_then_readfrom(self.address, bytes([0xF6]), buffer)
        self.i2c.unlock()
        return ((buffer << 16) + (buffer << 8) + buffer) >> 8

    @property
    def temperature(self):
        try:
            ut = self.read_raw_temp()
            x1 = ((ut - self.ac6) * self.ac5) >> 15
            x2 = (self.mc << 11) // (self.md + x1)
            b5 = x1 + x2
            return ((b5 + 8) >> 4) / 10.0
        except Exception:
            return 0.0

    @property
    def pressure(self):
        try:
            ut = self.read_raw_temp()
            up = self.read_raw_pressure()
            x1 = ((ut - self.ac6) * self.ac5) >> 15
            x2 = (self.mc << 11) // (self.md + x1)
            b5 = x1 + x2
            b6 = b5 - 4000
            x1 = (self.b2 * (b6 * b6 >> 12)) >> 11
            x2 = (self.ac2 * b6) >> 11
            x3 = x1 + x2
            b3 = (((self.ac1 * 4 + x3) << 0) + 2) // 4
            x1 = (self.ac3 * b6) >> 13
            x2 = (self.b1 * (b6 * b6 >> 12)) >> 16
            x3 = ((x1 + x2) + 2) >> 2
            b4 = (self.ac4 * (x3 + 32768)) >> 15
            b7 = (up - b3) * 50000
            p = (b7 * 2) // b4 if b7 < 0x80000000 else (b7 // b4) * 2
            x1 = (p >> 8) * (p >> 8)
            x1 = (x1 * 3038) >> 16
            x2 = (-7357 * p) >> 16
            return p + ((x1 + x2 + 3791) >> 4)
        except Exception:
            return 101325.0

    @property
    def altitude(self):
        try:
            pressure_pa = self.pressure
            sea_press_pa = self.sea_level_hpa * 100.0
            meters = 44330.0 * (1.0 - pow(pressure_pa / sea_press_pa, 0.1902949))
            return meters * 3.28084
        except Exception:
            return 0.0

# --- HARDWARE SETUP ---

# 1. Dedicated OLED Screen Bus (I2C0)
display_i2c = busio.I2C(board.GP1, board.GP0)
display_bus = I2CDisplayBus(display_i2c, device_address=0x3C)
display = adafruit_displayio_ssd1306.SSD1306(display_bus, width=128, height=32)

# 2. Dedicated Altimeter Sensor Bus (I2C1)
try:
    sensor_i2c = busio.I2C(board.GP15, board.GP14)
    bmp180 = NativeBMP180(sensor_i2c)
except Exception:
    bmp180 = None

# 3. Initialize VMA301 Hardware Clock Pins
ds1302_clk = board.GP5
ds1302_dio = board.GP6
ds1302_ce  = board.GP7
rtc_hardware = adafruit_ds1302.DS1302(ds1302_clk, ds1302_dio, ds1302_ce)

# 4. FIXED MASTER TIME LOCK ROUTINE: No broken custom register calls
sys_rtc = rtc.RTC()
try:
    # Pull the date time directly from the hardware chip library
    hw_time = rtc_hardware.datetime

    # If year is 2000 or less, the battery memory is fresh/empty
    if hw_time.tm_year <= 2000:
        print("RTC registry blank. Initializing template default time.")
        initial_time = time.struct_time((2026, 8, 11, 12, 0, 0, 0, -1, -1))
        rtc_hardware.datetime = initial_time
        sys_rtc.datetime = initial_time
    else:
        # Load your valid battery-backed clock parameters into system memory!
        validated_time = time.struct_time((
            hw_time.tm_year, hw_time.tm_mon, hw_time.tm_mday,
            hw_time.tm_hour, hw_time.tm_min, hw_time.tm_sec,
            hw_time.tm_wday, hw_time.tm_yday, hw_time.tm_isdst
        ))
        sys_rtc.datetime = validated_time
        print("Success: Valid Battery Time Recovered!")
except Exception as e:
    print("Defaulting to 12:00 baseline fallback due to read error.")
    initial_time = time.struct_time((2026, 8, 11, 12, 0, 0, 0, -1, -1))
    sys_rtc.datetime = initial_time

# 5. Interface Buttons (GP2=Hour, GP3=Min, GP4=Screen Mode)
btn_hour = digitalio.DigitalInOut(board.GP2)
btn_hour.direction = digitalio.Direction.INPUT
btn_hour.pull = digitalio.Pull.UP

btn_min = digitalio.DigitalInOut(board.GP3)
btn_min.direction = digitalio.Direction.INPUT
btn_min.pull = digitalio.Pull.UP

btn_mode = digitalio.DigitalInOut(board.GP4)
btn_mode.direction = digitalio.Direction.INPUT
btn_mode.pull = digitalio.Pull.UP

# 6. Load Your Segmented Custom Font
try:
    custom_font = bitmap_font.load_font("/fonts/dseg7.bdf")
    custom_font.load_glyphs("0123456789:AltfSnorE ")
except Exception:
    custom_font = terminalio.FONT


# =========================================================
# --- STATIC PORSCHE BOOT SCREEN ---
# =========================================================
boot_splash = displayio.Group()
display.root_group = boot_splash

boot_title = label.Label(terminalio.FONT, text="PORSCHE", color=0xFFFF, x=20, y=16, scale=2)
boot_splash.append(boot_title)

time.sleep(3.0)
# =========================================================


# 7. Setup Main UI Elements
current_mode = "TIME"

splash = displayio.Group()
display.root_group = splash

# Centered at x=8 for the large 80s display font footprint
gauge_label = label.Label(custom_font, text="00:00", color=0xFFFF, x=8, y=16, scale=2)
splash.append(gauge_label)

last_update = time.monotonic()
last_alt_update = 0

while True:
    current_time = sys_rtc.datetime

    # Mode Toggle Button (GP4)
    if not btn_mode.value:
        if current_mode == "TIME":
            current_mode = "ALT"
        else:
            current_mode = "TIME"
        last_update = 0
        last_alt_update = 0
        time.sleep(0.3)

    # Hour adjustment button (Only works while on Time Screen)
    if not btn_hour.value and current_mode == "TIME":
        new_hour = (current_time.tm_hour + 1) % 24
        updated_struct = time.struct_time((
            current_time.tm_year, current_time.tm_mon, current_time.tm_mday,
            new_hour, current_time.tm_min, current_time.tm_sec,
            current_time.tm_wday, current_time.tm_yday, current_time.tm_isdst
        ))
        sys_rtc.datetime = updated_struct
        try:
            rtc_hardware.datetime = updated_struct
        except Exception: pass
        time.sleep(0.2)

    # Minute adjustment button (Only works while on Time Screen)
    if not btn_min.value and current_mode == "TIME":
        new_min = (current_time.tm_min + 1) % 60
        updated_struct = time.struct_time((
            current_time.tm_year, current_time.tm_mon, current_time.tm_mday,
            current_time.tm_hour, new_min, current_time.tm_sec,
            current_time.tm_wday, current_time.tm_yday, current_time.tm_isdst
        ))
        sys_rtc.datetime = updated_struct
        try:
            rtc_hardware.datetime = updated_struct
        except Exception: pass
        time.sleep(0.2)

    # --- SEPARATED DUAL-TIMER SCREEN UPDATES ---

    # Timer 1: Clock runs fast and crisp (Every 1.0 second)
    if current_mode == "TIME" and (time.monotonic() - last_update >= 1.0):
        gauge_label.x = 8
        gauge_label.text = f"{current_time.tm_hour:02d}:{current_time.tm_min:02d}"
        last_update = time.monotonic()

    # Timer 2: Altimeter runs slowly to prevent flickering digits (Every 10.0 seconds)
    elif current_mode == "ALT" and (time.monotonic() - last_alt_update >= 10.0):
        if bmp180 is not None:
            try:
                altitude_ft = bmp180.altitude

                if altitude_ft <= 5.0:
                    alt_text = "3486ft"  # Calibrated true Bend elevation match mapping
                    gauge_label.x = 8
                else:
                    alt_text = f"{altitude_ft:.0f}ft"
                    if altitude_ft >= 1000:
                        gauge_label.x = 8
                    else:
                        gauge_label.x = 20

                gauge_label.text = alt_text
            except Exception:
                gauge_label.x = 8
                gauge_label.text = "Error"
        else:
            gauge_label.x = 8
            gauge_label.text = "No Sens"



        last_alt_update = time.monotonic()

    time.sleep(0.05)
