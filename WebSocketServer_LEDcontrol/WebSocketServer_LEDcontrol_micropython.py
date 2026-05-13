import board
import digitalio
import neopixel
import socketpool
import supervisor
import time
import wifi

SSID = "Team Rocket"
PASSWORD = "5038038883"
GITHUB_PAGE = "drifterz28.github.io/Arduino-stuff/"

LED_PIN = board.IO3
LED_COUNT = 40
SWITCH_PIN = board.IO18

HOSTNAME = "ESP_Light_1"

red = 0
green = 0
blue = 0
lights_on = False

switch_pin = digitalio.DigitalInOut(SWITCH_PIN)
switch_pin.direction = digitalio.Direction.INPUT
switch_pin.pull = digitalio.Pull.UP

strip = neopixel.NeoPixel(LED_PIN, LED_COUNT, auto_write=False)
pool = None
http_server = None


def color_wipe(r, g, b, wait_ms=0):
    for i in range(LED_COUNT):
        strip[i] = (r, g, b)
        strip.write()
        if wait_ms > 0:
            time.sleep(wait_ms / 1000)


def startup_led_test():
    color_wipe(64, 0, 0, 10)
    color_wipe(0, 64, 0, 10)
    color_wipe(0, 0, 64, 10)
    color_wipe(0, 0, 0, 2)


def current_rgb_csv():
    return "{},{},{}".format(red, green, blue)


def current_visible_rgb_csv():
    if lights_on:
        return current_rgb_csv()
    return "0,0,0"


def apply_light_state():
    if lights_on:
        color_wipe(red, green, blue, 0)
    else:
        color_wipe(0, 0, 0, 0)


def toggle_lights():
    global lights_on
    lights_on = not lights_on
    apply_light_state()


def apply_hex_color(msg):
    global red, green, blue, lights_on

    rgb = int(msg[1:7], 16)
    if rgb == 0:
        lights_on = False
        apply_light_state()
        return "0,0,0"

    red = (rgb >> 16) & 0xFF
    green = (rgb >> 8) & 0xFF
    blue = rgb & 0xFF
    lights_on = True
    apply_light_state()
    return current_visible_rgb_csv()


def make_index_html():
    return (
        '<!DOCTYPE html><html><head><meta charset="utf-8"><title>RGB Colors</title>'
        '<meta name="viewport" content="user-scalable=no, initial-scale=1, maximum-scale=1, minimum-scale=1, width=device-width">'
        '<link rel="stylesheet" href="https://{GITHUB_PAGE}/assets/rgb-led.css"/>'
        '<script src="https://unpkg.com/react@15.3.1/dist/react.js"></script>'
        '<script src="https://unpkg.com/react-dom@15.3.1/dist/react-dom.js"></script>'
        '<script src="https://unpkg.com/babel-core@5.8.38/browser.min.js"></script>'
        '</head><body><div class="container"></div>'
        '<script type="text/babel" src="https://{GITHUB_PAGE}/assets/rgb-led.js"></script></body></html>'
    ).format(GITHUB_PAGE=GITHUB_PAGE)


def send_response(client, status, content_type, body_bytes):
    header = "HTTP/1.1 {}\r\nContent-Type: {}\r\nConnection: close\r\nContent-Length: {}\r\n\r\n".format(
        status, content_type, len(body_bytes)
    ).encode(
        "utf-8"
    )
    client.send(header)
    client.send(body_bytes)


def socket_recv(sock, bufsize):
    buf = bytearray(bufsize)
    try:
        nbytes = sock.recv_into(buf, bufsize)
        return bytes(buf[:nbytes]) if nbytes else b""
    except OSError as e:
        if e.errno in (11, 35):
            return b""
        raise


def socket_recv_wait(sock, bufsize, timeout=5.0):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        data = socket_recv(sock, bufsize)
        if data:
            return data
        time.sleep(0.01)
    return b""


def handle_http_request(client):
    try:
        data = socket_recv_wait(client, 2048)
        if not data:
            client.close()
            return
        request = data.decode("utf-8")
        first_line = request.split("\r\n", 1)[0]
        parts = first_line.split(" ")
        method = parts[0] if len(parts) > 0 else ""
        path = parts[1] if len(parts) > 1 else "/"
        print("HTTP {} {}".format(method, path))

        # Split headers from body
        body = ""
        if "\r\n\r\n" in request:
            body = request.split("\r\n\r\n", 1)[1].strip()

        if method == "GET" and path == "/":
            page = make_index_html().encode("utf-8")
            send_response(client, "200 OK", "text/html", page)

        elif method == "GET" and path == "/color":
            csv = current_visible_rgb_csv().encode("utf-8")
            send_response(client, "200 OK", "text/plain", csv)

        elif method == "POST" and path == "/color":
            if body.startswith("#") and len(body) >= 7:
                try:
                    csv = apply_hex_color(body[:7]).encode("utf-8")
                    send_response(client, "200 OK", "text/plain", csv)
                except ValueError:
                    client.send(
                        b"HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n"
                    )
            else:
                client.send(b"HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n")

        else:
            client.send(b"HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n")

    except Exception as e:
        print("HTTP error:", e)
    finally:
        try:
            client.close()
        except Exception:
            pass


def accept_http_client():
    try:
        client, _ = http_server.accept()
    except OSError:
        return
    handle_http_request(client)


def process_serial():
    global lights_on

    if not supervisor.runtime.serial_bytes_available:
        return

    try:
        cmd = input().strip()
    except Exception:
        return

    upper_cmd = cmd.upper()
    if cmd.startswith("#") and len(cmd) >= 7:
        try:
            apply_hex_color(cmd)
            print("serial color ->", cmd)
        except ValueError:
            print("invalid hex color, use #RRGGBB")
    elif upper_cmd == "ON":
        lights_on = True
        apply_light_state()
        print("serial power -> ON")
    elif upper_cmd == "OFF":
        lights_on = False
        apply_light_state()
        print("serial power -> OFF")
    elif upper_cmd == "TOGGLE":
        toggle_lights()
        print("serial power -> TOGGLE")
    elif upper_cmd == "STATUS":
        print("status on={} rgb={}".format(lights_on, current_rgb_csv()))
    elif cmd:
        print("commands: #RRGGBB, ON, OFF, TOGGLE, STATUS")


def monitor_switch(last_reading, stable_state, last_change_s):
    reading = switch_pin.value
    now = time.monotonic()

    if reading != last_reading:
        last_change_s = now
        last_reading = reading

    if reading != stable_state and (now - last_change_s) > 0.05:
        stable_state = reading
        toggle_lights()

    return last_reading, stable_state, last_change_s


def connect_wifi():
    try:
        wifi.radio.hostname = HOSTNAME
    except Exception:
        pass

    print()
    for t in range(4, 0, -1):
        print("[SETUP] BOOT WAIT {}...".format(t))
        time.sleep(1)

    while not wifi.radio.connected:
        try:
            wifi.radio.connect(SSID, PASSWORD)
        except Exception:
            time.sleep(0.5)

    print("SSID:", SSID)
    print("IP address:", str(wifi.radio.ipv4_address))


def _bind_socket(pool, port, retries=10, delay=2.0):
    for attempt in range(retries):
        sock = pool.socket(pool.AF_INET, pool.SOCK_STREAM)
        sock.setsockopt(pool.SOL_SOCKET, pool.SO_REUSEADDR, 1)
        try:
            sock.bind(("0.0.0.0", port))
            sock.listen(3)
            sock.settimeout(0)
            return sock
        except OSError as e:
            sock.close()
            if attempt < retries - 1:
                print(
                    "Port {} in use, retrying in {}s... ({}/{})".format(
                        port, delay, attempt + 1, retries
                    )
                )
                time.sleep(delay)
            else:
                raise e
    return None


def setup_server():
    global pool, http_server
    pool = socketpool.SocketPool(wifi.radio)
    http_server = _bind_socket(pool, 80)


def main():
    connect_wifi()
    setup_server()
    startup_led_test()

    print("HTTP server started on port 80")
    print("GET  /color  -> current color as r,g,b")
    print("POST /color  -> set color, body: #RRGGBB")
    print("Serial commands: #RRGGBB, ON, OFF, TOGGLE, STATUS")

    last_reading = switch_pin.value
    stable_state = last_reading
    global lights_on
    lights_on = stable_state == 0
    apply_light_state()
    last_change_s = time.monotonic()

    while True:
        accept_http_client()
        process_serial()
        last_reading, stable_state, last_change_s = monitor_switch(
            last_reading, stable_state, last_change_s
        )
        time.sleep(0.01)


main()
