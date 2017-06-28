#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>

ESP8266WebServer server = ESP8266WebServer(80);

unsigned long currentMillis;
unsigned long timeOffset;
int64_t long setTime;

long long char2LL(String str) {
  long long result = 0; // Initialize result
  // Iterate through all characters of input string and update result
  for (int i = 0; str[i] != '\0'; ++i)
    result = result*10 + str[i] - '0';
  return result;
}

void handleRoot() {
  currentMillis = millis();
  if(server.hasArg("time")) {
    timeOffset = currentMillis;
    Serial.print("setTimeSrc: ");
    Serial.println(server.arg("time"));
    setTime = char2LL(server.arg("time"));
  }
  Serial.print("currentMillis: ");
  Serial.println(currentMillis);
  Serial.print("timeOffset: ");
  Serial.println(timeOffset);
  Serial.print("the math: ");
  Serial.println((currentMillis - timeOffset));
  unsigned long showTime = setTime + (currentMillis - timeOffset);
    Serial.print("setTime: ");
  Serial.println(setTime);
  Serial.print("showTime: ");
  Serial.println(showTime);
  server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset=\"utf-8\"></head><body>ESP time: <span class=\"esp\"></span></br>Current Time: <span class=\"client\"></span><br><button>Update time</button><script>var espM = " + String(showTime) + ";var client = document.querySelector('.client');var button = document.querySelector('button');var esp = document.querySelector('.esp');var theTime = (m) => {var nt = new Date(m);var t = nt.toLocaleTimeString();var d = nt.toLocaleDateString();return d + ' ' + t + ' ' + m;};client.textContent = theTime(Date.now());esp.textContent = theTime(espM);button.addEventListener('click', (e) => {location.href = location.origin + '?time=' + Date.now();});</script></body></html>");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Start setup");
  delay(4000);
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");

  if (MDNS.begin("esp-rtc")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
}
