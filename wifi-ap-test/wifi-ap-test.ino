#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

extern "C" {
#include "user_interface.h"
}

/* Set these to your desired credentials. */
const char *ssid = "TESTAP";
const char *password = "123456789";

ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
    server.send(200, "text/html", "<h1>You are connected</h1>");
}

void setup() {
  ESP.eraseConfig();
    delay(1000);
    Serial.begin(115200);
    Serial.println();
    Serial.print("Configuring access point...");
    /* You can remove the password parameter if you want the AP to be open. */
    wifi_set_phy_mode(PHY_MODE_11B);
  WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
  WiFi.printDiag(Serial);
}
