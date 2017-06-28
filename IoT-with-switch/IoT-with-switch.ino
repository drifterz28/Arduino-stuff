#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "FS.h"
#include <ArduinoJson.h>
#include "fauxmoESP.h"

#define DEFAULT_AP "ESP8266"
#define DEFAULT_AP_PASS "admin-password"
// setch size in bytes
// 311,081
char* ssid = "";
char* password = "";
char* deviceName = "";
char* apSsid = DEFAULT_AP;
char* apPass = DEFAULT_AP_PASS;
bool useApMode = true;
bool hasDoneSetup = false;

// constance
#define SERIAL_BAUDRATE 115200
fauxmoESP fauxmo;
int relay = 2;
int toggleSwitch = 0;

int holdState = 0;
int buttonState = 0;

ESP8266WebServer server(80);

bool loadConfig() {
//  File configFile = SPIFFS.open("/config.json", "r");
//  if (!configFile) {
//    //if(!saveConfig()) return false;
//  }
//  size_t size = configFile.size();
//  if (size > 1024) {
//    return false;
//  }
//  std::unique_ptr<char[]> buf(new char[size]);
//
//  configFile.readBytes(buf.get(), size);
//
//  StaticJsonBuffer<200> jsonBuffer;
//  JsonObject& json = jsonBuffer.parseObject(buf.get());
//
//  if (!json.success()) {
//    return false;
//  }
//  
//  apSsid = json["apSsid"];
//  apPass = json["apPass"];
//  useApMode = json["useApMode"];
//  
//  ssid = json["ssid"];
//  password = json["password"];
//  deviceName = json["deviceName"];
//  hasDoneSetup = json["hasDoneSetup"];
//  delay(200);
//  configFile.close();
//  return true;
}

String htmlTemplate(String content) {
  String html = "<html><head><title>Setup device</title>";
  html += "<style>body{background:#fafafa;}.container{margin:auto;width:300px;background:white;padding:10px;}fieldset{padding-bottom:5px;border-top:1px solid;}legend{text-align:center;}label{float:left;width:100%;padding-top:5px;}input{float:right;width:60%;}button{width:100%;}</style>";
  html += "</head><body><div class=\"container\">";
  html += content;
  html += "</div></body></html>";
  return html;
}

void handleSaveSetup() {
  File configFile = SPIFFS.open("/config.txt", "w+");
  String content = "";
  bool errors = false;
  
  //char* ssid = "";
  //char* password = "";
  //char* deviceName = "";
  //char* apSsid = DEFAULT_AP;
  //char* apPass = DEFAULT_AP_PASS;
  //bool useApMode = true;
  //bool hasDoneSetup = false;
  
  if(!configFile) {
    content += "<li>Failed to open config to save.";
    Serial.println("Failed to open config to save.");
    errors = true;
  }
  if(server.hasArg("ssidAp")) {
    configFile.println("apSsid=" + server.arg("ssidAp"));
    Serial.println("apSsid=" + server.arg("ssidAp"));
  }
  if(server.hasArg("apPassword")) {
    configFile.println("apPass=" + server.arg("apPassword"));
    Serial.println("apPass=" + server.arg("apPassword"));
  }
  if(server.hasArg("device")) {
    configFile.println("deviceName=" + server.arg("device"));
    Serial.println("deviceName=" + server.arg("device"));
  }
  if(server.hasArg("wifiSsid")) {
    configFile.println("ssid=" + server.arg("wifiSsid"));
    Serial.println("ssid=" + server.arg("wifiSsid"));
  }
  if(server.hasArg("wifiPassword")) {
    configFile.println("password=" + server.arg("wifiPassword"));
    Serial.println("password=" + server.arg("wifiPassword"));
  }
  if(server.hasArg("useAP")) {
    configFile.println("useApMode=" + server.arg("useAP"));
    Serial.println("useApMode=" + server.arg("useAP"));
  }
    //if (server.arg("apPassword")
  if(!errors) {
    content += "<li>AP SSID: ";
  }
  String html = htmlTemplate(content);
  server.send(200, "text/html", html);
  configFile.close();
  // reboot after saving
  //ESP.reset();
  // build in factory reset
  //SPIFFS.remove("/config.txt");
}

void handleSetup() {
  String useAP = useApMode ? "checked" : "";
  String content = "<form method='post' action='save'>";
  content += "<fieldset><legend>Access Point setup</legend><label>AP SSID: <input name='ssidAp' value='" + String(apSsid) + "'></label>";
  content += "<label>AP Password: <input type='password' name='apPassword' value='" + String(apPass) + "'></label>";
  content += "<label>Use AP: <input type='checkbox' name='useAP' " + useAP +"></label></fieldset>";
  content += "<fieldset><legend>Wifi Setup</legend>";
  content += "<label>Wifi SSID: <input name='wifiSsid' value='" + String(ssid) + "'></label>";
  content += "<label>Wifi Password: <input type='password' name='wifiPassword' value='" + String(password) + "'></label></fieldset>";
  content += "<fieldset><legend>Device Setup</legend>";
  content += "<label>Device Name: <input name='device' value='" + String(deviceName) + "'></label></fieldset><button>Submit</button></form>";
  String html = htmlTemplate(content);
  server.send(200, "text/html", html);
}

void handleRoot() {
  if(!hasDoneSetup) {
    handleSetup();
  } else {
    String content;
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    // espIP?state=1
    // no state will give current state
    if(server.hasArg("state")) {
      if(server.arg("state") == "1") {
        digitalWrite(relay, 1);
        holdState = 1;
      } else if(server.arg("state") == "0") {
        digitalWrite(relay, 0);
        holdState = 0;
      }
    }
    
    root["deviceName"] = deviceName;
    root["output"] = holdState;
    root["input"] = buttonState;
    
    root.printTo(content);
    server.sendHeader("Cache - Control","no - cache");
    server.send(200, "application/json", content);
  }
}

void wifiSetup() {
  //  File configFile = SPIFFS.open("/config.txt", "r"); 
//  WiFi.disconnect();
//  WiFi.softAPdisconnect(true);
  
  if(useApMode) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.hostname(apSsid);
    WiFi.softAP(apSsid, apPass);
  } else {
    WiFi.mode(WIFI_STA);
  }
  
  if(ssid != "") {
    Serial.printf("[WIFI] Connecting to % s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
    }
    Serial.printf("SSID: % s, IP address: % s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
  }
  
  server.on("/", handleRoot);
  server.on("/setup", handleSetup);
  server.on("/save", handleSaveSetup);
  server.begin();
  Serial.println("server started");
}

void setup() {
  // Init serial port and clean garbage
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println();
  Serial.println("Start");
  
  SPIFFS.begin();
  //SPIFFS.format();
  
//  File configFile = SPIFFS.open("/config.txt", "r+");
//  if(configFile) {
//    Serial.println("file open");
//    hasDoneSetup = true;
//    while(configFile.available()) {
//      //Lets read line by line from the file
//      String line = configFile.readStringUntil('\n');
//      Serial.println(line);
//    }
//    configFile.close();
//  } else {
//    Serial.println("file not open");  
//  }
  
  wifiSetup();
  
  pinMode(relay, OUTPUT);
  pinMode(toggleSwitch, INPUT);
  digitalWrite(relay, 1);
  if(deviceName != "") {
    fauxmo.addDevice(deviceName);
    fauxmo.onMessage([](const char * device_name, bool state) {
      Serial.printf("[MAIN] % s state: % s\n", device_name, state ? "ON" : "OFF");
      if(!state) {
        digitalWrite(relay, 1);
        holdState = 1;
      } else {
        digitalWrite(relay, 0);
        holdState = 0;
      }
    });
  }
}

void loop() {
  server.handleClient();
  int newButtonState = digitalRead(toggleSwitch);
  if(buttonState != newButtonState) {
    buttonState = newButtonState;
    holdState = holdState ? 0 : 1;
    digitalWrite(relay, holdState);
  }
  delay(10);
}
