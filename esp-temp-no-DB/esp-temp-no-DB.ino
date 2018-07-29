#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include "FS.h"
#include "DHT.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Time.h>

String ApName = "esp-temp";
String hostName = "esp-temp";
String ApPass = "0123456789";

#define DHTPIN 2     // sensor pin connected to
#define DHTTYPE DHT22   // DHT 22 sensor
DHT dht(DHTPIN, DHTTYPE);

char room_name[40] = "room";
char pull_time[3] = "15";
unsigned long previousMillis = 0;

char* db_name = "/temps.db";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server = ESP8266WebServer(80);

//flag for saving data
bool shouldSaveConfig = false;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  SPIFFS.begin();
  dht.begin();
  timeClient.begin();
  //clean FS, for testing
  //SPIFFS.format();
  loadSettings();

  WiFiManagerParameter custom_room_name("room_name", "Room name", room_name, 40);
  WiFiManagerParameter custom_pull_time("pull_time", "pull frequency", pull_time, 3);
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_room_name);
  wifiManager.addParameter(&custom_pull_time);

  wifiManager.autoConnect(ApName.c_str(), ApPass.c_str());

  Serial.println("WiFi connected");

  if (MDNS.begin(hostName.c_str())) {
    Serial.println("MDNS responder started");
  }

  strcpy(room_name, custom_room_name.getValue());
  strcpy(pull_time, custom_pull_time.getValue());

  if (shouldSaveConfig) {
    Serial.println("saving config");
    Serial.println(pull_time);
    Serial.println(room_name);
    saveSettings();
    //end save
  }

  server.on("/", []() {
    if (server.hasArg("roomName") && server.hasArg("pull")) {
      server.arg("roomName").toCharArray(room_name, 40);
      server.arg("pull").toCharArray(pull_time, 3);
      saveSettings();
    }

    float temp = dht.readTemperature(true);
    float humidity = dht.readHumidity();
    String roomName = String(room_name);
    String pullTime = String(pull_time);
    String title = roomName + " Temperature and Humidity";
    server.sendHeader("charset", "utf-8", true);
    server.send(200, "text/html", "<html><head><title>"+ title +"</title><script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script> <script type='text/javascript'> var pullTime=" + pullTime + ", roomName='" + roomName + "', temp='" + String(temp) + "', humidity='" + String(humidity) + "'; google.charts.load('current',{'packages': ['corechart']}); google.charts.setOnLoadCallback(fd); setInterval(fd, pullTime * 60000); function fd(){document.getElementById('name').value=roomName; document.getElementById('pull').value=pullTime; fetch('./api').then(r=> r.json()).then(j=> dC(j.data));}function dC(d){if(d.length===0) return; var newData=d.reduce((a, d)=>{var strDT=`${d[2]}`; var nDT=new Date(strDT); a.push([nDT, +d[0], +d[1]]); return a;}, []), d=google.visualization.arrayToDataTable([ ['TimeStamp', 'Temperature', 'Humidity'], ...newData]), options={allowHtml: true, title: `${document.title} - Current: ${temp}&deg;f | ${humidity}% @ ${new Date()}`, curveType: 'function', legend:{position: 'bottom'}}, fD=new google.visualization.NumberFormat({suffix: ' f'}), fH=new google.visualization.NumberFormat({suffix: '%'}), chart=new google.visualization.LineChart(document.getElementById('c')); fD.format(d, 1); fH.format(d, 2); console.log([['TimeStamp', 'Temperature', 'Humidity'], ...newData]); chart.draw(d, options);}</script> <style>.s{margin:0 auto; width:900px; overflow: auto;}</style></head><body> <div class='s' id='h'></div><div id='c' class='s' style='height:500px;'>No data to show</div><form method='post' class='s'> Update Settings<br><label>Room name <input id='name' name='roomName'></label> <label>Pull interval <input id='pull' name='pull'></label> <button>Update</button> </form></body></html>");
  });

  server.on("/delete", []() {
    String data = "File removed";
    if (SPIFFS.remove(db_name)) {
      data = "Failed to remove file";
    }
    server.send(200, "text/html", data);
  });

  server.on("/api", []() {
    File f = SPIFFS.open(db_name, "r");
    String data = "{\"data\":[";
    Serial.println(f.available());
    while (f.available()) {
      String line = f.readStringUntil('\n');
      line.trim();
      data += "[" + line + "],";
    }
    data += "]}";
    int length = data.length();
    data.remove(length - 3, 1);
    server.sendHeader("Access-Control-Allow-Origin", "*", true);
    server.sendHeader("charset", "utf-8", true);
    server.send(200, "application/json", data);
    f.close();
  });

  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  timeClient.update();
  unsigned long currentMillis = millis();
  unsigned long pullTime = char2LL(pull_time);
  unsigned long updateInterval = pullTime * 60000;

  if (currentMillis - previousMillis >= updateInterval) {
    float temp = dht.readTemperature(true);
    float hum = dht.readHumidity();
    previousMillis = currentMillis;
    if (isnan(hum) || isnan(temp)) {
      return;
    }
    saveData(temp, hum, getFullFormattedTime());
  }
}

void saveData(float temp, float hum, String ts) {
  File f = SPIFFS.open(db_name, "a");
  if (!f) {
    Serial.println("file creation failed");
  }
  // now write two lines in key/value style with end-of-line characters
  f.println("\"" + String(temp) + "\",\"" + String(hum) + "\",\"" + ts + "\"");
  f.close();
}

