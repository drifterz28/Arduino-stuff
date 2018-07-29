

long long char2LL(char *str) {
  long long result = 0; // Initialize result
  // Iterate through all characters of input string and update result
  for (int i = 0; str[i] != '\0'; ++i)
    result = result*10 + str[i] - '0';
  return result;
}

String getFullFormattedTime() {
  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);

  uint16_t year = ti->tm_year + 1900;
  String yearStr = String(year);

  uint8_t month = ti->tm_mon + 1;
  String monthStr = month < 10 ? "0" + String(month) : String(month);

  uint8_t day = ti->tm_mday;
  String dayStr = day < 10 ? "0" + String(day) : String(day);

  uint8_t hours = ti->tm_hour;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  uint8_t minutes = ti->tm_min;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  uint8_t seconds = ti->tm_sec;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

  return yearStr + "-" + monthStr + "-" + dayStr + " " +
         hoursStr + ":" + minuteStr + ":" + secondStr + "  GMT";
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void saveSettings() {
  // open the file in write mode
  File f = SPIFFS.open("/f.txt", "w");
  if (!f) {
    Serial.println("file creation failed");
  }
  // now write two lines in key/value style with end-of-line characters
  f.println("room_name=" + String(room_name));
  f.println("pull_time=" + String(pull_time));
  f.close();
}

void loadSettings() {
  File f = SPIFFS.open("/f.txt", "r");
  while (f.available()) {
    //Lets read line by line from the file
    String line = f.readStringUntil('\n');
    int firstIndex = line.indexOf("=");
    String key = line.substring(0, firstIndex);
    String value = line.substring(firstIndex + 1);
    value.trim();
    if (key == "room_name") {
      value.toCharArray(room_name, 40);
    }
    if (key == "pull_time") {
      value.toCharArray(pull_time, 3);
    }
  }
  f.close();
}
