import binascii
import board
import digitalio
import hashlib
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

WS_GUID = b"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

red = 0
green = 0
blue = 0
lights_on = False
clients = []

switch_pin = digitalio.DigitalInOut(SWITCH_PIN)
switch_pin.direction = digitalio.Direction.INPUT
switch_pin.pull = digitalio.Pull.UP

strip = neopixel.NeoPixel(LED_PIN, LED_COUNT, auto_write=False)
pool = None
http_server = None
ws_server = None


def color_wipe(r, g, b, wait_ms=0):
    for i in range(LED_COUNT):
        strip[i] = (r, g, b)
        strip.write()
        if wait_ms > 0:
            time.sleep(wait_ms / 1000)


def startup_led_test():
    # Quick power-on verification pattern: RGB wipes, then clear.
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


def ws_accept_key(client_key):
    sha1 = hashlib.new("sha1", client_key + WS_GUID)
    return binascii.b2a_base64(sha1.digest()).strip()


def ws_send_text(client, text):
    payload = text.encode("utf-8")
    header = bytearray()
    header.append(0x81)
    length = len(payload)
    if length < 126:
        header.append(length)
    elif length < 65536:
        header.append(126)
        header.extend(length.to_bytes(2, "big"))
    else:
        header.append(127)
        header.extend(length.to_bytes(8, "big"))
    client.send(header + payload)


def ws_broadcast(text):
    dead = []
    for client in clients:
        try:
            ws_send_text(client, text)
        except Exception:
            dead.append(client)

    for client in dead:
        close_client(client)


def close_client(client):
    try:
        client.close()
    except Exception:
        pass
    if client in clients:
        clients.remove(client)


def recv_exact(sock, size, timeout=5.0):
    data = bytearray()
    deadline = time.monotonic() + timeout
    while len(data) < size:
        if time.monotonic() > deadline:
            return None
        chunk = socket_recv(sock, size - len(data))
        if chunk:
            data.extend(chunk)
        else:
            time.sleep(0.005)
    return bytes(data)


def read_ws_frame(client):
    first = socket_recv(client, 2)
    if not first:
        return None, None
    if len(first) < 2:
        return -1, b""

    b1 = first[0]
    b2 = first[1]
    opcode = b1 & 0x0F
    masked = (b2 & 0x80) != 0
    length = b2 & 0x7F

    if length == 126:
        ext = recv_exact(client, 2)
        if not ext:
            return -1, b""
        length = int.from_bytes(ext, "big")
    elif length == 127:
        ext = recv_exact(client, 8)
        if not ext:
            return -1, b""
        length = int.from_bytes(ext, "big")

    mask = b""
    if masked:
        mask = recv_exact(client, 4)
        if not mask:
            return -1, b""

    payload = recv_exact(client, length)
    if payload is None:
        return -1, b""

    if masked:
        data = bytearray(payload)
        for i in range(length):
            data[i] ^= mask[i % 4]
        payload = bytes(data)

    return opcode, payload


def process_ws_client(client):
    try:
        opcode, payload = read_ws_frame(client)
    except OSError:
        return
    except Exception:
        close_client(client)
        return

    if opcode is None:
        close_client(client)
        return

    if opcode == -1:
        return

    if opcode == 0x8:
        close_client(client)
        return

    if opcode == 0x9:
        try:
            client.send(b"\x8a\x00")
        except Exception:
            close_client(client)
        return

    if opcode == 0x1:
        msg = payload.decode("utf-8").strip()
        if msg.startswith("#") and len(msg) >= 7:
            try:
                ws_broadcast(apply_hex_color(msg))
            except ValueError:
                pass


def socket_recv(sock, bufsize):
    """Non-blocking read using recv_into (CircuitPython 9.x compatible)."""
    buf = bytearray(bufsize)
    try:
        nbytes = sock.recv_into(buf, bufsize)
        return bytes(buf[:nbytes]) if nbytes else b""
    except OSError as e:
        if e.errno in (11, 35):  # EAGAIN / EWOULDBLOCK — no data yet
            return b""
        raise


def socket_recv_wait(sock, bufsize, timeout=5.0):
    """Blocking read that polls until data arrives or timeout (seconds)."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        data = socket_recv(sock, bufsize)
        if data:
            return data
        time.sleep(0.01)
    return b""


def parse_headers(request_text):
    headers = {}
    lines = request_text.split("\r\n")
    for line in lines[1:]:
        if not line or ":" not in line:
            continue
        k, v = line.split(":", 1)
        headers[k.strip().lower()] = v.strip()
    return headers


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

        if method == "GET" and path == "/":
            body = make_index_html().encode("utf-8")
            header = (
                (
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Connection: close\r\n"
                    "Content-Length: {}\r\n\r\n"
                )
                .format(len(body))
                .encode("utf-8")
            )
            client.send(header)
            client.send(body)
            return

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


def accept_ws_client():
    try:
        client, _ = ws_server.accept()
    except OSError:
        return

    try:
        req = socket_recv_wait(client, 2048)
        if not req:
            client.close()
            return
        request_text = req.decode("utf-8")
        headers = parse_headers(request_text)
        key = headers.get("sec-websocket-key", "")
        if not key:
            client.send(b"HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n")
            client.close()
            return

        accept = ws_accept_key(key.encode("utf-8")).decode("ascii")
        response = (
            "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: {}\r\n\r\n"
        ).format(accept)
        client.send(response.encode("utf-8"))
        try:
            client.settimeout(0)
        except (AttributeError, OSError):
            pass
        clients.append(client)
        print("WebSocket client connected")
        ws_send_text(client, current_visible_rgb_csv())
    except Exception as e:
        print("WS accept error:", e)
        try:
            client.close()
        except Exception:
            pass


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
            ws_broadcast(apply_hex_color(cmd))
            print("serial color ->", cmd)
        except ValueError:
            print("invalid hex color, use #RRGGBB")
    elif upper_cmd == "ON":
        lights_on = True
        apply_light_state()
        ws_broadcast(current_visible_rgb_csv())
        print("serial power -> ON")
    elif upper_cmd == "OFF":
        lights_on = False
        apply_light_state()
        ws_broadcast("0,0,0")
        print("serial power -> OFF")
    elif upper_cmd == "TOGGLE":
        toggle_lights()
        ws_broadcast(current_visible_rgb_csv())
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
        ws_broadcast(current_visible_rgb_csv())

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
            sock.listen(2)
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


def setup_servers():
    global pool, http_server, ws_server
    pool = socketpool.SocketPool(wifi.radio)
    http_server = _bind_socket(pool, 80)
    ws_server = _bind_socket(pool, 81)


def main():
    connect_wifi()
    setup_servers()
    startup_led_test()

    print("HTTP server started on port 80")
    print("WebSocket server started on port 81")
    print("Serial commands: #RRGGBB, ON, OFF, TOGGLE, STATUS")

    last_reading = switch_pin.value
    stable_state = last_reading
    global lights_on
    lights_on = stable_state == 0
    apply_light_state()
    last_change_s = time.monotonic()

    while True:
        accept_http_client()
        accept_ws_client()
        process_serial()

        for client in list(clients):
            process_ws_client(client)

        last_reading, stable_state, last_change_s = monitor_switch(
            last_reading, stable_state, last_change_s
        )
        time.sleep(0.01)


main()
