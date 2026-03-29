#include <OneWire.h>
#include <DallasTemperature.h>

// ===== ピン設定 =====
#define ONE_WIRE_BUS 2   // DS18B20のDATAピン
#define SSR_PIN 8        // SSR制御ピン

// ===== 温度センサー設定 =====
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ===== 温度設定 =====
const float TARGET_TEMP = 37.5;   // 目標温度
const float TEMP_ON     = 37.0;   // これ未満でヒーターON
const float TEMP_OFF    = 38.0;   // これ以上でヒーターOFF

bool heaterState = false;

void setup() {
  Serial.begin(9600);
  sensors.begin();

  pinMode(SSR_PIN, OUTPUT);

  // 初期状態はOFF
  digitalWrite(SSR_PIN, LOW);
  heaterState = false;

  Serial.println("=== Biodigester Temperature Control Start ===");
}

void loop() {
  // 温度取得
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  // センサー異常チェック
  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Sensor error! Heater OFF.");
    digitalWrite(SSR_PIN, LOW);
    heaterState = false;
    delay(2000);
    return;
  }

  // ヒステリシス制御
  if (!heaterState && tempC < TEMP_ON) {
    heaterState = true;
    digitalWrite(SSR_PIN, HIGH);   // SSR ON
  } 
  else if (heaterState && tempC > TEMP_OFF) {
    heaterState = false;
    digitalWrite(SSR_PIN, LOW);    // SSR OFF
  }

  // シリアル表示
  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.print(" C | Heater: ");
  if (heaterState) {
    Serial.println("ON");
  } else {
    Serial.println("OFF");
  }

  delay(2000);
}