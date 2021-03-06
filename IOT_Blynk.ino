#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "DHT.h"
#include <SimpleTimer.h>
#define DHTPIN 2  
#define AS A0
#define DEN D3
#define UP D7
#define DOW D5
#define NUOC D6
#define PUMP_PIN D8   
#define LAMP_PIN D0   
#define NUOC_PIN D9 
#define DHTTYPE DHT22
int ghnhiet = 35;
#define Thoigian 50L
#define READ_BUTTONS_TM   1L  
#define READ_SOIL_HUM_TM  2L 
#define READ_AIR_DATA_TM  1L  
#define AUTO_CTRL_TM      5L 
char auth[] = "authtoken"; // Blynk token
char ssid[] = "wifiname"; 
char pass[] = "wifipassword"; 
WidgetLED QUAT1(V5);
float tempDHT = 0;
float doamDHT = 0;
int as = 0;
boolean pumpStatus = 0;
boolean lampStatus = 0;
boolean nuocStatus = 0;
void startTimers(void);
SimpleTimer timer;
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x3F, 20, 4);
void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LAMP_PIN, OUTPUT);
  pinMode(NUOC_PIN, OUTPUT);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DEN, INPUT_PULLUP);
  pinMode(DOW, INPUT_PULLUP);
  pinMode(NUOC, INPUT_PULLUP);
  dht.begin();
  aplyCmd();
  startTimers();
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("WELCOME TO OUR FARM");
  Blynk.virtualWrite(V0, LOW);
  Blynk.virtualWrite(V1, LOW);
  Blynk.virtualWrite(V4, ghnhiet);
}
void loop() {
  timer.run(); 
  Blynk.run();
  readbutton();
}
BLYNK_WRITE(0) 
{
  int i = param.asInt();
  if (i == 1)
  {
    lampStatus = !lampStatus;
    aplyCmd();
  }
}
BLYNK_WRITE(1) 
{
  int i = param.asInt();
  if (i == 1)
  {
    bat_bom();
  }
} BLYNK_WRITE(4) 
{
  ghnhiet = param.asInt();
}
void docanhsang(void)
{
  int i = 0;
  as = 0;
  for (i = 0; i < 20; i++)
  {
    as += analogRead(AS); 
    delay(5);  
  }
  as = as / (i);
  as = map(as, 1023, 0, 0, 999);
}
void getDhtData(void)
{
  doamDHT = dht.readHumidity();
  tempDHT = dht.readTemperature();
}
void printData(void)
{
  lcd.setCursor(0, 1);
  lcd.print("  Temp    : "); lcd.print(tempDHT); lcd.setCursor(16, 1); lcd.print("|"); lcd.print(ghnhiet);
  lcd.setCursor(0, 2);
  lcd.print("  Humidity: "); lcd.print(doamDHT); lcd.print("%");
  lcd.setCursor(0, 3);
  lcd.print("  Light   :     "); lcd.setCursor(13, 3); lcd.print(as);
}
void aplyCmd()
{
  if (pumpStatus == 0) {
    QUAT1.off();
    digitalWrite(PUMP_PIN, LOW);
  }
  if (pumpStatus == 1) {
    QUAT1.on();
    digitalWrite(PUMP_PIN, HIGH);
  }
  if (lampStatus == 0) {
    Blynk.virtualWrite(V0, LOW);
    digitalWrite(LAMP_PIN, LOW);
  }
  if (lampStatus == 1) {
    Blynk.virtualWrite(V0, HIGH);
    digitalWrite(LAMP_PIN, HIGH);
  }
  if (nuocStatus == 0) {
    Blynk.virtualWrite(V1, LOW);
    digitalWrite(NUOC_PIN, LOW);
  }
  if (nuocStatus == 1) {
    Blynk.virtualWrite(V1, HIGH);
    digitalWrite(NUOC_PIN, HIGH);
  }
}
void che_tu_dong(void)
{
  if (tempDHT > ghnhiet)
  {
    pumpStatus = 1;
    aplyCmd();
  }
  else
  {
    pumpStatus = 0;
    aplyCmd();
  }
}
void readbutton(void)
{
  if (!digitalRead(UP))
  {
    ghnhiet++;
    if (ghnhiet > 100) ghnhiet = 0;
    delay(100);
  }
  if (!digitalRead(DOW))
  {
    ghnhiet--;
    if (ghnhiet < 0) ghnhiet = 100;
    delay(100);
  }

  if (!digitalRead(NUOC))
  {
    delay(10);
    if (!digitalRead(NUOC))
    {
      bat_bom();
      delay(40);
      while (!digitalRead(NUOC));
    }
  }
  if (!digitalRead(DEN))
  {
    delay(10);
    if (!digitalRead(DEN))
    {
      lampStatus = !lampStatus;
      aplyCmd();
      delay(40);
      while (!digitalRead(DEN));
    }
  }

}
void sendUptime()
{
  Blynk.virtualWrite(V10, tempDHT);
  Blynk.virtualWrite(V11, as);
  Blynk.virtualWrite(V12, doamDHT);
  Blynk.virtualWrite(V4, ghnhiet);
}
void bat_bom()
{
  nuocStatus = 1;
  aplyCmd();
  timer.restartTimer (timer.setInterval(1000L, sendUptime));
}
void tat_bom()
{
  nuocStatus = 0;
  aplyCmd();
}
void startTimers(void)
{
  timer.setInterval(500L, printData);
  timer.setInterval(READ_AIR_DATA_TM * 1000, getDhtData);
  timer.setInterval(READ_SOIL_HUM_TM * 100, docanhsang);
  timer.setInterval(AUTO_CTRL_TM * 1000, che_tu_dong);
  timer.setInterval(Thoigian * 1000, tat_bom);
  timer.setInterval(1000L, sendUptime);
}
