float getHumidity() {
  float humidity = dht.readHumidity();
  if (isnan(humidity)) {
    Serial.println("bad humidity, re-run");
  }
  Serial.print("Humidity: ");
  Serial.println(humidity);
  return humidity;
}

float getTemp() {
  float temp_f = dht.readTemperature(isFahrenheit);
  if (isnan(temp_f)) {
    Serial.println("bad temp, rerun");
  }
  Serial.print("Temperature: ");
  Serial.println(temp_f);
  return temp_f;
}

//callback notifying us of the need to save config
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void loadSpiffs() {
  Serial.println("mounting FS...");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/con.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(minute_delay, json["minute_delay"]);
          strcpy(location_name, json["location_name"]);
          strcpy(sheet_id, json["sheet_id"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
}

void sheetSetup() {
  client = new HTTPSRedirect(httpsPort);
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  Serial.print("Connecting to ");
  Serial.println(host);
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }

  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    return;
  }

  if (client->verify(fingerprint, host)) {
    Serial.println("Certificate match.");
  } else {
    Serial.println("Certificate mis-match");
  }

  // Send memory data to Google Sheets
  payload = payload_base + "\"" + free_heap_before + "," + free_stack_before + "\"}";
  client->POST(url2, host, payload, false);
  payload = payload_base + "\"" + ESP.getFreeHeap() + "," +
            cont_get_free_stack(&g_cont) + "\"}";
  client->POST(url2, host, payload, false);

  Serial.println("\nGET: Write into cell 'A1'");
  Serial.println("=========================");

  // fetch spreadsheet data
  client->GET(url, host);

  // Send memory data to Google Sheets
  payload = payload_base + "\"" + ESP.getFreeHeap() + "," +
            cont_get_free_stack(&g_cont) + "\"}";
  client->POST(url2, host, payload, false);

  // fetch spreadsheet data
  client->GET(url2, host);

  payload = payload_base + "\"" + ESP.getFreeHeap() + "," +
            cont_get_free_stack(&g_cont) + "\"}";
  client->POST(url2, host, payload, false);

  Serial.println("\nSeries of GET and POST requests");
  Serial.println("===============================");

  Serial.printf("Free heap: %u\n", ESP.getFreeHeap());
  Serial.printf("unmodified stack   = %4d\n", cont_get_free_stack(&g_cont));

  // delete HTTPSRedirect object
  delete client;
  client = nullptr;
}

void wifiConfigSetup() {
  WiFiManagerParameter custom_minute_delay("delay", "10", minute_delay, 3);
  WiFiManagerParameter custom_location_name("Room name", "Some Room", location_name, 50);
  WiFiManagerParameter custom_sheet_id("Sheet ID", "Sheet ID", sheet_id, 50);
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  Serial.print("Wait for WiFi... ");

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_minute_delay);
  wifiManager.addParameter(&custom_location_name);
  wifiManager.addParameter(&custom_sheet_id);

  // set callback that gets called when connecting to previous WiFi
  // fails, and enters Access Point mode
  // wifiManager.setAPCallback(configModeCallback);
  if (!wifiManager.startConfigPortal(ApName)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  strcpy(minute_delay, custom_minute_delay.getValue());
  strcpy(location_name, custom_location_name.getValue());
  strcpy(sheet_id, custom_sheet_id.getValue());
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["minute_delay"] = minute_delay;
    json["location_name"] = location_name;
    json["sheet_id"] = sheet_id;

    File configFile = SPIFFS.open("/con.json", "w+");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    } else {
      json.printTo(Serial);
      json.printTo(configFile);
    }
    configFile.close();
  }
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}
