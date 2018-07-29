#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti WiFiMulti;

#define DHTPIN 0     // sensor pin connected to
#define DHTTYPE DHT22   // DHT 22 sensor

const char* ssid = "CenturyLink1356";
const char* pass = "5038038883";
String url = "http://192.168.0.115/temp-track";
int main_delay = 15; // delay in min for the loop
bool isFahrenheit = true;

uint8_t MAC_array[6];
char MAC_char[18];

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("WiFi temp link");
  Serial.println();
  dht.begin();
  delay(4000);

  WiFiMulti.addAP(ssid, pass);
  Serial.print("Wait for WiFi... ");
  WiFi.macAddress(MAC_array);
  for (int i = 0; i < sizeof(MAC_array); ++i) {
    sprintf(MAC_char, "%s%02x:", MAC_char, MAC_array[i]);
  }
  Serial.println("WiFi connected");
  Serial.print("Get MAC address: ");
  Serial.println(MAC_char); // just to see the mac
  Serial.print("IP address: ");
  WiFi.softAP("esp8266-3", "nothingyoucanfindout", 1, 1);
  Serial.println(WiFi.localIP()); // just so you know
  delay(500);
}

float getHumidity() {
  float humidity = dht.readHumidity();
  if (isnan(humidity)) {
    Serial.println("bad humidity, re-run");
    delay(5000);
    humidity = getHumidity();
  }
  Serial.print("Humidity: ");
  Serial.println(humidity);
  return humidity;
}

float getTemp() {
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float temp_f = dht.readTemperature(isFahrenheit);
  if (isnan(temp_f)) {
    Serial.println("bad temp, rerun");
    delay(5000);
    temp_f = getTemp();
  }
  Serial.print("Temperature: ");
  Serial.println(temp_f);
  return temp_f;
}

void loop() {
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    String fullUrl = url + "?mac=" + String(MAC_char) + "&t=" + String(getTemp()) + "&h=" + String(getHumidity());
    Serial.print("Send to server: ");
    Serial.println(fullUrl);
    HTTPClient http;
    http.begin(fullUrl);
    int httpCode = http.GET();
    if (httpCode != -1) {
      Serial.println("data sent to server");
      delay(1000 * 60 * main_delay);
    } else {
      Serial.println("fail");
      delay(50000); // run in 5 min on fail
    }
  }
}
