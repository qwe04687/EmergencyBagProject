#include <Adafruit_AM2320.h>
#include <Adafruit_Sensor.h>
#include <SoftwareSerial.h>
SoftwareSerial BT(3, 4);
Adafruit_AM2320 am2320 = Adafruit_AM2320();
unsigned long now_time = 0;
float dustVal = 0;
int Rvalue = 254, Gvalue = 1, Bvalue = 127;
int Rdirection = -1, Gdirection = 1, Bdirection = -1;
int delayTime = 6;
const int Red = 10, Green = 11, Blue = 9, rgbPower = 5, ledPower = 2, dustPin = 0;
int RGBmode = 0;
int lightness = 0;
String text, environ;

void setup() {
  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(Blue, OUTPUT);
  pinMode(ledPower, OUTPUT);
  pinMode(rgbPower, OUTPUT);
  pinMode(dustPin, INPUT);

  digitalWrite(rgbPower, LOW);
  am2320.begin();
  BT.begin(9600);
  Serial.begin(9600);
  // ----取環境PM2.5值作為隨機種子碼，避免產生偽隨機亂數。---
  digitalWrite(ledPower, LOW);
  delayMicroseconds(280);
  randomSeed(analogRead(dustPin));
  delayMicroseconds(40);
  digitalWrite(ledPower, HIGH);
  // --------------------------------------------------
}

void loop() {
  LedMode(RGBmode);
  char data;
  if (BT.available()) {
    data = BT.read();
    text += data;
    if (data == '*') {
      text = text.substring(0, text.length() - 1);
      Serial.println(text);
      // --------環境感測程式碼區段---------
      if (text == "PM2.5") {
        digitalWrite(ledPower, LOW);
        delayMicroseconds(280);
        dustVal = analogRead(dustPin);
        delayMicroseconds(40);
        digitalWrite(ledPower, HIGH);
        delay(1000);
        if (dustVal > 36.455) {
          dustVal = (float(dustVal / 1024) - 0.0356) * 12000 * 0.035;
          environ = String(am2320.readTemperature()) + " " + String(am2320.readHumidity()) + " " + String(dustVal);
          Serial.println(environ);
          BT.print(environ);
        }
      }
      // --------------------------------
      if (text == "turnOff") //關燈
        RGBmode = 0;
      // -----------燈效參數設為彩虹-----
      if (text == "rainbow") {
        RGBmode = 1;
        Rvalue = 254, Gvalue = 1, Bvalue = 127;
        Rdirection = -1, Gdirection = 1, Bdirection = -1;
      }
      // -----------------------------
      if (text == "breath") // 燈效參數設為呼吸燈
        RGBmode = 3;
      if (text == "random") // 燈效參數設為隨機
        RGBmode = 4;
      // -------取得使用者於APP色環所選取之顏色--------
      if (text.substring(0, 1) == "R")
        Rvalue = 255 - text.substring(1).toInt();
      if (text.substring(0, 1) == "G")
        Gvalue = 255 - text.substring(1).toInt();
      if (text.substring(0, 1) == "B") {
        Bvalue = 255 - text.substring(1).toInt();
        RGBmode = 2;
      }
      // -------------------------------------
      text = "";
    }
  }
}

void LedMode(int Mode) {
  if (Mode == 0) {
    digitalWrite(rgbPower, LOW);
  }
  // -------------彩虹燈效程式碼區塊------
  if (Mode == 1) {
    digitalWrite(rgbPower, HIGH);
    if (millis() > now_time + delayTime) {
      now_time = millis();
      analogWrite(Red, Rvalue); 
      analogWrite(Green, Gvalue);
      analogWrite(Blue, Bvalue);

      Rvalue = Rvalue + Rdirection; 
      Gvalue = Gvalue + Gdirection;
      Bvalue = Bvalue + Bdirection;

      if (Rvalue >= 255 || Rvalue <= 0) {
        Rdirection = Rdirection * -1;
      }
      if (Gvalue >= 255 || Gvalue <= 0) {
        Gdirection = Rdirection * -1;
      }
      if (Bvalue >= 255 || Bvalue <= 0) {
        Bdirection = Bdirection * -1;
      }
    }
  }
  // --------------------------------
  // ----------APP色環選取顏色-----------
  if (Mode == 2) {
    digitalWrite(rgbPower, HIGH);

    analogWrite(Red, Rvalue);
    analogWrite(Green, Gvalue);
    analogWrite(Blue, Bvalue);
  }
  // ------------------------------
  // -------------呼吸燈燈效程式碼區塊---------
  if (Mode == 3) {
    if (millis() > now_time + 4) {
      analogWrite(Red, 0);
      analogWrite(Green, 0);
      analogWrite(Blue, 0);
      now_time = millis();
      if (lightness > 360)
        lightness = 0;
      analogWrite(5, (sin(lightness++ * 0.0174533) + 1) * 127);
    }
  }
  // -------------------------------------
  // ------------------隨機色燈效程式碼區塊--------
  if (Mode == 4) {
    if (millis() > now_time + 1000) {
      now_time = millis();
      digitalWrite(rgbPower, HIGH);
      analogWrite(Red, random(256));
      analogWrite(Green, random(256));
      analogWrite(Blue, random(256));
    }
  }
  // --------------------------------------
}
