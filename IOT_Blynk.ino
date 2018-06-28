#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "DHT.h"
#include <SimpleTimer.h>
#define DHTPIN 2   // Chân DATA nối với D4
#define AS A0
// Relay, nút nhấn
#define DEN D3
#define UP D7
#define DOW D5
#define NUOC D6
#define PUMP_PIN D8   //Quat
#define LAMP_PIN D0   //Den
#define NUOC_PIN D9 // Bơm
#define DHTTYPE DHT22
/* Thông số cho chế độ tự động */
int ghnhiet = 35;
/* TIMER */
#define Thoigian 50L // 50s 
#define READ_BUTTONS_TM   1L  // Tương ứng với giây
#define READ_SOIL_HUM_TM  2L //Đọc cảm biến anh sang
#define READ_AIR_DATA_TM  1L  //Đọc DHT
#define AUTO_CTRL_TM      5L //Chế độ tư động
//Token Blynk và wifi
char auth[] = "b9f5f20c34c04645a94153b0c7336ec0"; // Blynk token
char ssid[] = "ASUS_X018D"; //Tên wifi
char pass[] = "12041204"; //Mật khẩu
// Đèn trạng thái  quat
WidgetLED QUAT1(V5);
float tempDHT = 0;
float doamDHT = 0;
int as = 0;
boolean pumpStatus = 0;
boolean lampStatus = 0;
boolean nuocStatus = 0;
void startTimers(void);
// Khởi tạo timer
SimpleTimer timer;
// Khởi tạo cảm biến
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
  lcd.begin();// Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("WELCOME TO OUR FARM");
  Blynk.virtualWrite(V0, LOW);
  Blynk.virtualWrite(V1, LOW);
  Blynk.virtualWrite(V4, ghnhiet);
}

void loop() {
  timer.run(); // Chạy SimpleTimer
  Blynk.run();
  readbutton();
}
BLYNK_WRITE(0) // Điều khiển đèn
{
  int i = param.asInt();
  if (i == 1)
  {
    lampStatus = !lampStatus;
    aplyCmd();
  }
}
BLYNK_WRITE(1) // Điều khiển bơm
{
  int i = param.asInt();
  if (i == 1)
  {
    bat_bom();
  }
} BLYNK_WRITE(4) // Điều khiển bơm
{
  ghnhiet = param.asInt();
}
void docanhsang(void)
{
  int i = 0;
  as = 0;
  for (i = 0; i < 20; i++)
  {
    as += analogRead(AS); //Đọc giá trị cảm biến ánh sáng
    delay(5);   // Đợi đọc giá trị ADC
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
  // IN thông tin ra LCD
  lcd.setCursor(0, 1);
  lcd.print("  Temp    : "); lcd.print(tempDHT); lcd.setCursor(16, 1); lcd.print("|"); lcd.print(ghnhiet);
  lcd.setCursor(0, 2);
  lcd.print("  Humidity: "); lcd.print(doamDHT); lcd.print("%");
  lcd.setCursor(0, 3);
  lcd.print("  Light   :     "); lcd.setCursor(13, 3); lcd.print(as);
}
/***************************************************
  Thực hiện điều khiển các bơm
****************************************************/
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
/***************************************************
  Hàm kiểm tra trạng thái phím bấm
****************************************************/
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
/***************************************************
  Khởi động Timers
****************************************************/
void startTimers(void)
{
  timer.setInterval(500L, printData);
  timer.setInterval(READ_AIR_DATA_TM * 1000, getDhtData);
  timer.setInterval(READ_SOIL_HUM_TM * 100, docanhsang);
  timer.setInterval(AUTO_CTRL_TM * 1000, che_tu_dong);
  timer.setInterval(Thoigian * 1000, tat_bom);
  timer.setInterval(1000L, sendUptime);
}
