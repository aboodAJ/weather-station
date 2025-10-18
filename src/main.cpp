#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>


// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Sensors
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

const int lightSensorPin = A0;


// ---------- SETUP ----------
void setup() {
  Serial.begin(9600);
  Wire.begin(12, 14); // D6(GPIO12) = SDA and D5(GPIO14) = SCL

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Setting up sensors...");
  display.display();

  // Initialize sensors
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
  display.display();

  delay(500);
}