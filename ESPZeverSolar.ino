#include <TFT_eSPI.h>
#include <HTTPClient.h>
#include <WiFi.h>

const char *WIFI_SSID = "YOUR_WIFI_SSID";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char *INVERTER_URL = "http://192.168.1.136/home.cgi";

const unsigned long REQUEST_INTERVAL_MS = 30000;
const int POWER_LINE_INDEX = 10; // Zero-based line number. Example response line 12 is "0.6" kW.
const int TFT_BACKLIGHT_PIN = 4;
#define TFT_GREY 0x5AEB // New colour

TFT_eSPI tft = TFT_eSPI();

unsigned long lastRequestAt = 0;
bool hasReading = false;
float lastPowerWatts = 0;
String lastStatus = "Starting";

void drawStatus(const String &status, bool showPower = false, float watts = 0) {
  Serial.print("Display status: ");
  Serial.print(status);
  if (showPower) {
    Serial.print(" (");
    Serial.print(watts, 0);
    Serial.print(" W)");
  }
  Serial.println();

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);

  tft.setTextSize(2);
  tft.drawString("Zever Solar", tft.width() / 2, 24);

  if (showPower) {
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(4);
    tft.drawString(String((int)round(watts)) + " W", tft.width() / 2, tft.height() / 2);

    tft.setTextColor(TFT_LIGHTGREY);
    tft.setTextSize(2);
    tft.drawString(status, tft.width() / 2, tft.height() - 24);
  } else {
    tft.setTextColor(TFT_YELLOW);
    tft.setTextSize(2);
    tft.drawString(status, tft.width() / 2, tft.height() / 2);
  }
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi SSID: ");
  Serial.println(WIFI_SSID);
  drawStatus("Connecting WiFi");

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    attempt++;
    Serial.print("WiFi connection attempt ");
    Serial.println(attempt);
  }

  lastStatus = "WiFi connected";
  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
  drawStatus(lastStatus);
}

bool parsePowerWatts(const String &response, float &watts) {
  int lineStart = 0;

  for (int lineIndex = 0; lineIndex <= POWER_LINE_INDEX; lineIndex++) {
    int lineEnd = response.indexOf('\n', lineStart);
    if (lineEnd == -1) {
      lineEnd = response.length();
    }

    if (lineIndex == POWER_LINE_INDEX) {
      String value = response.substring(lineStart, lineEnd);
      value.trim();
      Serial.print("Parsed power line value: ");
      Serial.println(value);

      if (value.length() == 0) {
        Serial.println("Power parse failed: value is empty");
        return false;
      }

      bool hasDigit = false;
      for (unsigned int i = 0; i < value.length(); i++) {
        char c = value.charAt(i);
        if (isDigit(c)) {
          hasDigit = true;
          continue;
        }

        if (c != '.' && c != '-' && c != '+') {
          Serial.print("Power parse failed: unexpected character '");
          Serial.print(c);
          Serial.println("'");
          return false;
        }
      }

      if (!hasDigit) {
        Serial.println("Power parse failed: no digit found");
        return false;
      }

      watts = value.toFloat();
      Serial.print("Power converted to watts: ");
      Serial.println(watts, 0);
      return true;
    }

    lineStart = lineEnd + 1;
    if (lineStart > response.length()) {
      break;
    }
  }

  return false;
}

void fetchAndDisplayPower() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected before request");
    lastStatus = "WiFi reconnecting";
    drawStatus(lastStatus, hasReading, lastPowerWatts);
    connectToWiFi();
  }

  Serial.print("Requesting inverter URL: ");
  Serial.println(INVERTER_URL);

  HTTPClient http;
  http.begin(INVERTER_URL);
  int statusCode = http.GET();
  Serial.print("HTTP status code: ");
  Serial.println(statusCode);

  if (statusCode != HTTP_CODE_OK) {
    lastStatus = "HTTP " + String(statusCode);
    drawStatus(lastStatus, hasReading, lastPowerWatts);
    http.end();
    return;
  }

  String response = http.getString();
  http.end();
  Serial.print("Response length: ");
  Serial.println(response.length());

  float watts = 0;
  if (!parsePowerWatts(response, watts)) {
    lastStatus = "Parse error";
    drawStatus(lastStatus, hasReading, lastPowerWatts);
    return;
  }

  lastPowerWatts = watts;
  hasReading = true;
  lastStatus = "Updated";
  Serial.print("Latest power reading stored: ");
  Serial.print(lastPowerWatts, 0);
  Serial.println(" W");
  drawStatus(lastStatus, true, lastPowerWatts);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("ESPZeverSolar starting");

  if (TFT_BACKLIGHT_PIN >= 0) {
    pinMode(TFT_BACKLIGHT_PIN, OUTPUT);
    digitalWrite(TFT_BACKLIGHT_PIN, HIGH);
    Serial.print("LCD backlight enabled on pin ");
    Serial.println(TFT_BACKLIGHT_PIN);
  }

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_GREY);
  Serial.print("LCD initialized. Width: ");
  Serial.print(tft.width());
  Serial.print(", height: ");
  Serial.println(tft.height());
  drawStatus("Starting");

  connectToWiFi();
  fetchAndDisplayPower();
  lastRequestAt = millis();
}

void loop() {
  unsigned long now = millis();
  if (now - lastRequestAt >= REQUEST_INTERVAL_MS) {
    lastRequestAt = now;
    Serial.println("Request interval elapsed");
    fetchAndDisplayPower();
  }
}
