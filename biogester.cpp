#include <WiFi.h>
#include <WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ===== Wi-Fi =====
// TODO: Migrate to NVS (Preferences library) for production use
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// ===== DS18B20 Pin (Biodigester + Water Tank) =====
// Both sensors share a single OneWire bus on GPIO4
// Red=3.3V, Black=GND, White=DATA
// 4.7kohm pull-up resistor required between VCC (3.3V) and DATA
#define ONE_WIRE_BUS 4

// ===== SSR Control Pins =====
// Driven via Optocoupler MOSFET Driver Modules (3.3V -> 5V level shift with optical isolation)
// ESP32 GPIO -> Module PWM input -> Module OUT -> SSR DC input (3-32V)
#define SSR_HEATER 25  // Water heater SSR
#define SSR_PUMP   26  // Circulation pump SSR

// ===== Temperature Thresholds (Hysteresis Control) =====
const float TEMP_ON  = 37.0;  // Heater + Pump ON below this
const float TEMP_OFF = 38.0;  // Heater + Pump OFF above this

// ===== Objects =====
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
WebServer server(80);

// ===== DS18B20 Sensor Addresses =====
// Run scanDS18B20Addresses() once to discover addresses, then set them here.
// Each DS18B20 has a unique 64-bit ROM code.
// Example: {0x28, 0xFF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}
DeviceAddress bioSensorAddr   = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
DeviceAddress waterSensorAddr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
bool addressesConfigured = false;

// ===== State Variables =====
bool heaterState = false;
bool pumpState   = false;
float bioTemp    = 0.0;
float waterTemp  = 0.0;

// ===== Scan and print DS18B20 addresses =====
void scanDS18B20Addresses() {
  int deviceCount = ds18b20.getDeviceCount();
  Serial.printf("Found %d DS18B20 device(s) on OneWire bus\n", deviceCount);

  DeviceAddress addr;
  for (int i = 0; i < deviceCount; i++) {
    if (ds18b20.getAddress(addr, i)) {
      Serial.printf("  Sensor %d address: {", i);
      for (int j = 0; j < 8; j++) {
        Serial.printf("0x%02X", addr[j]);
        if (j < 7) Serial.print(", ");
      }
      Serial.println("}");
    }
  }

  if (deviceCount < 2) {
    Serial.printf("WARNING: Expected 2 DS18B20 sensors, found %d.\n", deviceCount);
  }

  // Check if addresses have been configured (not all zeros)
  bool allZero = true;
  for (int i = 0; i < 8; i++) {
    if (bioSensorAddr[i] != 0x00) { allZero = false; break; }
  }
  if (allZero) {
    Serial.println("WARNING: DS18B20 addresses not configured. Using index-based reading.");
    Serial.println("         Copy the addresses above into bioSensorAddr and waterSensorAddr.");
    addressesConfigured = false;
  } else {
    addressesConfigured = true;
    Serial.println("DS18B20 addresses configured. Using address-based reading.");
  }
}

// ===== Web Endpoint: JSON =====
void handleData() {
  String json = "{";
  json += "\"biodigester_temp\":" + String(bioTemp, 1) + ",";
  json += "\"water_temp\":" + String(waterTemp, 1) + ",";
  json += "\"heater\":" + String(heaterState ? "true" : "false") + ",";
  json += "\"pump\":" + String(pumpState ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

// ===== Web Endpoint: Dashboard =====
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<title>Biodigester Monitor</title>";
  html += "<style>body{font-family:sans-serif;max-width:600px;margin:40px auto;padding:0 20px}";
  html += ".card{background:#f5f5f5;border-radius:8px;padding:16px;margin:12px 0}";
  html += ".on{color:#e53e3e}.off{color:#38a169}</style></head><body>";
  html += "<h1>Biodigester Monitor</h1>";
  html += "<div class='card'><b>Biodigester Temp:</b> " + String(bioTemp, 1) + " &deg;C</div>";
  html += "<div class='card'><b>Water Tank Temp:</b> " + String(waterTemp, 1) + " &deg;C</div>";
  html += "<div class='card'><b>Heater:</b> <span class='" + String(heaterState ? "on'>ON" : "off'>OFF") + "</span></div>";
  html += "<div class='card'><b>Pump:</b> <span class='" + String(pumpState ? "on'>ON" : "off'>OFF") + "</span></div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  // Pin initialization (SSR OFF at startup via optocoupler driver modules)
  pinMode(SSR_HEATER, OUTPUT);
  pinMode(SSR_PUMP, OUTPUT);
  digitalWrite(SSR_HEATER, LOW);
  digitalWrite(SSR_PUMP, LOW);

  // DS18B20 initialization
  ds18b20.begin();
  delay(500);
  scanDS18B20Addresses();

  // Wi-Fi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected! IP: " + WiFi.localIP().toString());

  // Web server
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  Serial.println("=== Biodigester Control System Start ===");
}

void loop() {
  server.handleClient();

  // DS18B20 reading (both sensors)
  ds18b20.requestTemperatures();
  if (addressesConfigured) {
    bioTemp   = ds18b20.getTempC(bioSensorAddr);
    waterTemp = ds18b20.getTempC(waterSensorAddr);
  } else {
    bioTemp   = ds18b20.getTempCByIndex(0);
    waterTemp = ds18b20.getTempCByIndex(1);
  }

  // Sensor error check
  if (bioTemp == DEVICE_DISCONNECTED_C || waterTemp == DEVICE_DISCONNECTED_C) {
    Serial.println("Sensor error! Heater & Pump OFF.");
    digitalWrite(SSR_HEATER, LOW);
    digitalWrite(SSR_PUMP, LOW);
    heaterState = false;
    pumpState = false;
    delay(2000);
    return;
  }

  // Hysteresis control (based on biodigester temperature)
  if (!heaterState && bioTemp < TEMP_ON) {
    heaterState = true;
    pumpState = true;
    digitalWrite(SSR_HEATER, HIGH);
    digitalWrite(SSR_PUMP, HIGH);
  }
  else if (heaterState && bioTemp > TEMP_OFF) {
    heaterState = false;
    pumpState = false;
    digitalWrite(SSR_HEATER, LOW);
    digitalWrite(SSR_PUMP, LOW);
  }

  // Serial output
  Serial.printf("Bio: %.1fC | Water: %.1fC | Heater: %s | Pump: %s\n",
    bioTemp, waterTemp,
    heaterState ? "ON" : "OFF",
    pumpState ? "ON" : "OFF");

  delay(2000);
}
