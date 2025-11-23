#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h> // library that allows your microcontroller to communicate with MQTT brokers
#include <NTPClient.h> // library for getting time from local NTP server

// WiFi setup
const char* ssid = "Orange-4E82";
const char* password = "5C386H24X6";

// MQTT broker (local)
const char* mqtt_server = "192.168.0.2";

WiFiClient espClient;
PubSubClient client(espClient);

// NTP
const char* ntpServer = "192.168.0.2";

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Sensors
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

const int lightSensorPin = A0;

// ---------- WIFI ----------
void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected, IP: ");
  Serial.println(WiFi.localIP());
}

// ---------- MQTT ----------
void reconnect() {
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
    }
  }
}

// ---------- BUFFER FOR UNSENT DATA ----------
#define MAX_BUFFER 150   // store up to 150 messages in memory
String messageBuffer[MAX_BUFFER];
int bufferCount = 0;

void pushToBuffer(String payload) {
  if (bufferCount < MAX_BUFFER) {
    messageBuffer[bufferCount++] = payload;
    Serial.println("Stored in buffer, count=" + String(bufferCount));
  } else {
    Serial.println("Buffer full, dropping data!");
  }
}

void flushBuffer() {
  if (client.connected() && bufferCount > 0) {
    for (int i = 0; i < bufferCount; i++) {
      client.publish("weather/data", messageBuffer[i].c_str());
      delay(100); // avoid flooding broker
    }
    bufferCount = 0; // reset after sending
    Serial.println("Buffer flushed.");
  }
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(9600);
  Wire.begin(12, 14); // D6(GPIO12) = SDA and D5(GPIO14) = SCL

  // Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // WiFi
  display.println("Connecting to WiFi...");
  display.display();

  setup_wifi();
  client.setServer(mqtt_server, 1883);

  display.println("WiFi connected.");
  display.display();

  // NTP
  display.println("Syncing time...");
  display.display();
  const long gmtOffset_sec = 0;
  const int daylightOffset_sec = 0;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    delay(500);
  }
  display.println("Time synced!");
  display.display();

  // Initialize sensors
  display.println("Setting up sensors...");
  display.display();

  if (!aht.begin()) { // Adresse automatically set to 0x38
    Serial.println("AHT20 not found");
    display.println("AHT20 failed");
    display.display();
    while (1);
  }

  if (!bmp.begin(0x77)) { // Manually set adresse to 0x76 or 0x77 
    Serial.println("BMP280 not found");
    display.println("BMP280 failed");
    display.display();
    while (1);
  }

  display.println("Sensors ready!");
  display.display();
  delay(2000);
}

// ---------- LOOP ----------
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Get current time
  struct tm timeinfo;
  getLocalTime(&timeinfo);

  int currentMinute = timeinfo.tm_min;
  int currentSecond = timeinfo.tm_sec;

  // Convert struct tm to UNIX timestamp
  time_t timestamp = mktime(&timeinfo);

  // collect sensor data
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  int lightLevel = analogRead(lightSensorPin);
  float pressure = bmp.readPressure() / 100.0F;

  // display on OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Abood's");
  display.println("Weather Station");
  display.print("Light: "); display.println(lightLevel);
  display.print("T: "); display.print(temp.temperature, 1); display.print( (char)247); display.println("C");
  display.print("H: "); display.print(humidity.relative_humidity, 0);display.println("%");
  display.print("P: "); display.print(pressure, 0); display.println(" hPa");
  display.setCursor(display.width()-6*3,0); // 6px per char
  display.print(bufferCount);
  display.display();

  // If it's a multiple of 5 minutes and second == 0, publish
  if (currentMinute % 5 == 0 && currentSecond <= 2) {
    // prepare payload
    char payload[256];
    snprintf(payload, sizeof(payload),
      "{\"timestamp\":%ld,\"light\":%d,\"humidity\":%.1f,"
      "\"t\":%.1f,\"pressure\":%.1f}",
      (long)timestamp,
      lightLevel,
      humidity.relative_humidity,
      temp.temperature,
      pressure
    );

    // Try publishing
    if (client.connected()) {
      flushBuffer(); // send any stored messages
      client.publish("weather/data", payload);
    } else {
      pushToBuffer(String(payload));
    }

    // Wait until we are out of this second to avoid publishing multiple times
    delay(3000);
  }

  delay(500);
}