#include <Wire.h>
#include <WiFiNINA.h>

#define BH1750_ADDRESS 0x23

const char* ssid = "NetComm 4599";
const char* password = "aqTDf5V4n8RG6qfQ";

const char* iftttKey = "bBP374S895nODcKrnYhRh-";
const char* iftttEventSunlight = "sunlight";

const float sunlightThreshold = 500.0;
bool isSunlight = false;
unsigned long sunlightStartTime = 0;
const unsigned long requiredSunlightDuration = 2 * 60 * 60 * 1000; // 2 hours

WiFiClient wifiClient;

void setup() 
{
  Serial.begin(9600);
  Wire.begin();
  WiFi.begin(ssid, password);
  for (int i = 0; i < 15 && WiFi.status() != WL_CONNECTED; i++) delay(1000);
  Wire.beginTransmission(BH1750_ADDRESS);
  Wire.write(0x10);
  Wire.endTransmission();
}

void loop() 
{
  float lightLevel = readLightIntensity();

  Serial.print("Light Level: ");
  Serial.print(lightLevel);
  Serial.println(" lx");

  if (lightLevel > sunlightThreshold) 
  {
    if (!isSunlight) 
    {
      isSunlight = true;
      sunlightStartTime = millis();
    }
  } 
  else 
  {
    if (isSunlight) 
    {
      isSunlight = false;
      sunlightStartTime = 0;
    }
  }

  if (isSunlight && (millis() - sunlightStartTime >= requiredSunlightDuration)) 
  {
    sendIFTTTNotification(iftttEventSunlight);
    isSunlight = false; 
    sunlightStartTime = 0;
  }

  delay(10000);
}

float readLightIntensity() {
  Wire.beginTransmission(BH1750_ADDRESS);
  if (Wire.requestFrom(BH1750_ADDRESS, 2) == 2) {
    uint16_t value = Wire.read() << 8 | Wire.read();
    return value / 1.2;
  }
  return -1;
}

void sendIFTTTNotification(const char* event) {
  if (wifiClient.connect("maker.ifttt.com", 80)) {
    wifiClient.print("GET /trigger/");
    wifiClient.print(event);
    wifiClient.print("/with/key/");
    wifiClient.print(iftttKey);
    wifiClient.println(" HTTP/1.1");
    wifiClient.println("Host: maker.ifttt.com");
    wifiClient.println("Connection: close");
    wifiClient.println();
    delay(500);
    wifiClient.stop();
  }
}
