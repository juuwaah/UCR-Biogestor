#include <max6675.h>

// ===== MAX6675 ピン設定 =====
#define THERMO_SCK  3   // CLK
#define THERMO_CS   4   // CS
#define THERMO_SO   5   // DO (MISO)

// ===== DCモーター制御ピン =====
#define MOTOR_PIN   9   // MOSFETゲートへ（PWM対応ピン）

// ===== 温度設定 =====
const float TEMP_ON  = 37.0;   // これ未満でモーターON（加温）
const float TEMP_OFF = 38.0;   // これ以上でモーターOFF

MAX6675 thermocouple(THERMO_SCK, THERMO_CS, THERMO_SO);
bool motorState = false;

void setup() {
  Serial.begin(9600);
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);
  motorState = false;

  // MAX6675 安定待ち
  delay(500);
  Serial.println("=== Biodigester Prototype Start ===");
}

void loop() {
  float tempC = thermocouple.readCelsius();

  // センサー異常チェック（MAX6675は熱電対未接続時にNAN）
  if (isnan(tempC) || tempC > 500.0) {
    Serial.println("Sensor error! Motor OFF.");
    digitalWrite(MOTOR_PIN, LOW);
    motorState = false;
    delay(2000);
    return;
  }

  // ヒステリシス制御
  if (!motorState && tempC < TEMP_ON) {
    motorState = true;
    digitalWrite(MOTOR_PIN, HIGH);
  }
  else if (motorState && tempC > TEMP_OFF) {
    motorState = false;
    digitalWrite(MOTOR_PIN, LOW);
  }

  // シリアル表示
  Serial.print("Temp: ");
  Serial.print(tempC, 1);
  Serial.print(" C | Motor: ");
  Serial.println(motorState ? "ON" : "OFF");

  // MAX6675は読み取り間隔250ms以上必要
  delay(2000);
}
