#include "iconSuDung.h"
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

//DFPlayer Mini
#include <SoftwareSerial.h> 
SoftwareSerial mySerial(27,26);   //  rx,tx
#define Start_Byte 0x7E        //
#define Version_Byte 0xFF       //
#define Command_Length 0x06    //    
#define End_Byte 0xEF          //
#define Acknowledge 0x01       //

TaskHandle_t Task2;
TFT_eSPI tft = TFT_eSPI();

//LoRa
#include "LoRa_E32.h"
LoRa_E32 e32ttl(&Serial2);

//===== Blynk IOT =====//
#define BLYNK_TEMPLATE_ID "TMPL63n88OAMc"
#define BLYNK_TEMPLATE_NAME "Forest fire and deforestation monitoring system"
#define BLYNK_AUTH_TOKEN "zkyhIhIsahuhSkIpjpaTMqHcB_wSq-tc"
#define BLYNK_PRINT Serial

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "ManhToan";
char pass[] = "123456789";

TaskHandle_t Core2;
BlynkTimer timer;
WidgetRTC rtc;


BLYNK_CONNECTED() {
  rtc.begin();
}

#define buzz       19
#define ledSafe    14
#define ledWarn    13
#define notice1    27
#define notice2    26
#define notice3    25
#define notice4    33
#define notice5    32


// -------------------------------------
void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);

long dv, chuc, tram, nghin, chucnghin;
int s;
long t1, t2, t3, t4, t5, t6, t7;
String str = "";
String currentTime, currentDate;

float   temp1, humi1, smoke1, sound1, thief1, battery1,
        temp2, humi2, smoke2, sound2, thief2, battery2,
        smokeHigh = 45.00,
        batLow  = 3.60;

//forest fire level
float a1, E1, e1, 
      a2, E2, e2;
int   P1, fire_level_1, led11, led12, led13, led14, led15,
      P2, fire_level_2, led21, led22, led23, led24, led25;

unsigned long
previousMillis        = 0,
currentMillis         = 0,
prevSerialMillis      = 0,
currentSerialMillis   = 0,
prevBlynkMillis       = 0,
currentBlynkMillis    = 0;

float
pin1 = 99,
pin2 = 99;

int
checkpin1 = 4,
checkpin2 = 4,
mauCanhBao = 0,
ERR1       = 0,
ERR2       = 0,
ERR3       = 0,
ERR4       = 0;

void clockDisplay()
{
  currentTime = String(hour()) + ":" + minute() + ":" + second();
  currentDate = String(day()) + " " + month() + " " + year();
}

void introduction(){
  digitalWrite(notice2, HIGH);
  tft.setSwapBytes(true);
  tft.pushImage(2, 2, 50, 50, logo);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(1);
  tft.setCursor(50, 10); tft.print("Ho Chi Minh City");
  tft.setCursor(50, 20); tft.print(" University of");
  tft.setCursor(50, 30); tft.print("  Technology ");
  
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(50, 50); tft.print("FOREST FIRE"); 
  tft.setCursor(7, 65);  tft.print("MONITORING-WARNING SYSTEM");
  
  tft.setCursor(5, 80); tft.print("Lecturer:");
  tft.setCursor(5, 100); tft.print("Student:");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(5, 90); tft.print("ThS Vo Thi Thu Hong");
  tft.setCursor(5, 110); tft.print("Nguyen Manh Toan-1814368");
  delay(7000);
  tft.fillScreen(TFT_BLACK);
}

void setup()
{
  Serial.begin(9600);

  mySerial.begin(9600);
  setVolume(20); // âm lượng 0 - 30
  setNumber(1);

  pinMode(ledSafe,    OUTPUT);
  pinMode(ledWarn,    OUTPUT);
  pinMode(buzz,       OUTPUT);
  pinMode(notice1,    OUTPUT);
  pinMode(notice2,    OUTPUT);
  pinMode(notice3,    OUTPUT);
  pinMode(notice4,    OUTPUT);
  pinMode(notice5,    OUTPUT);
  
  digitalWrite(ledSafe,    HIGH);
  digitalWrite(ledWarn,    HIGH);
  digitalWrite(buzz,       HIGH);
  delay(1000);
  digitalWrite(ledSafe,    LOW);
  digitalWrite(ledWarn,    LOW);
  digitalWrite(buzz,       LOW);
  delay(1000);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  e32ttl.begin();
  introduction();

  // ---------------------------
  xTaskCreatePinnedToCore(
    Task2code,   /* Task function. */
    "Task2",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task2,      /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */

} // end Setup

/////////////////////////////////////////////
void setVolume(int volume)
{
  execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
  delay(2000);
}
void setNumber(int NUM) 
{
   execute_CMD(0x03, 0, NUM); // chọn bài trong danh sách
  delay(200);
 }
void execute_CMD(byte CMD, byte Par1, byte Par2)
// Excecute the command and parameters
{
// Calculate the checksum (2 bytes)
word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
// Build the command line
byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte};
//Send the command line to the module
for (byte k=0; k<10; k++)
 {
mySerial.write( Command_line[k]);
 }
}
////////////////////////////////////////////////////////////////

void Task2code( void * pvParameters ) {
  Blynk.begin(auth, ssid, pass);
  setSyncInterval(5); //thoi gian dong bo 5s
  while (1) {
    senBlynk();
    Blynk.run();
    timer.run();
  }
}

void loop()
{
  NhanDuLieu();
  TFT_Print();
  Onboard_Telemetry();
  CanhBao();
  delay(1000);
} // end loop

//////Receiver data
void NhanDuLieu() {
  if (e32ttl.available()  > 1) {
    ResponseContainer rs = e32ttl.receiveMessage();
    // First of all get the data
    String message = rs.data;
    xulydulieu(message);

    if (t1 == 1) {
      temp1     = t2 / 10.0;
      humi1     = t3 / 10.0;
      smoke1    = t4 / 10.0;
      if (t5 == 0) sound1 = 1;
      if (t5 == 1) sound1 = 0;
      thief1    = t6;
      battery1  = t7 / 100.0;
      pin1 = battery1*100/5;
      checkpin1 = battery1;
    }
    else if (t1 == 2) {
      temp2     = t2 / 10.0;
      humi2     = t3 / 10.0;
      smoke2    = t4 / 10.0;
      if (t5 == 0) sound2 = 1;
      if (t5 == 1) sound2 = 0;
      thief2    = t6;
      battery2  = t7 / 100.0;
      pin2 = battery2*100/5;
      checkpin2 = battery2;
    }
  }
}

void xulydulieu(String mes) {
  str = (String)mes;

  //t1 - nodeID
  s = str.indexOf("b") - str.indexOf("a");
  if (s == 2) {
    t1 = str[str.indexOf("a") + 1] - '0';
  }
  else if (s == 3) {
    chuc = str[str.indexOf("a") + 1] - '0';
    dv = str[str.indexOf("a") + 2] - '0';
    t1 = chuc * 10 + dv;
  }
  else if (s == 4) {
    tram = str[str.indexOf("a") + 1] - '0';
    chuc = str[str.indexOf("a") + 2] - '0';
    dv = str[str.indexOf("a") + 3] - '0';
    t1 = tram * 100 + chuc * 10 + dv;
  }
  else if (s == 5) {
    nghin = str[str.indexOf("a") + 1] - '0';
    tram = str[str.indexOf("a") + 2] - '0';
    chuc = str[str.indexOf("a") + 3] - '0';
    dv = str[str.indexOf("a") + 4] - '0';
    t1 = nghin * 1000 + tram * 100 + chuc * 10 + dv;
  }
  else if (s == 6) {
    chucnghin = str[str.indexOf("a") + 1] - '0';
    nghin = str[str.indexOf("a") + 2] - '0';
    tram = str[str.indexOf("a") + 3] - '0';
    chuc = str[str.indexOf("a") + 4] - '0';
    dv = str[str.indexOf("a") + 5] - '0';
    t1 = chucnghin * 10000 + nghin * 1000 + tram * 100 + chuc * 10 + dv;
  }

  //t2 = temp
  s = str.indexOf("c") - str.indexOf("b");
  if (s == 2) {
    t2 = str[str.indexOf("b") + 1] - '0';
  }
  else if (s == 3) {
    chuc = str[str.indexOf("b") + 1] - '0';
    dv = str[str.indexOf("b") + 2] - '0';
    t2 = chuc * 10 + dv;
  }
  else if (s == 4) {
    tram = str[str.indexOf("b") + 1] - '0';
    chuc = str[str.indexOf("b") + 2] - '0';
    dv = str[str.indexOf("b") + 3] - '0';
    t2 = tram * 100 + chuc * 10 + dv;
  }
  else if (s == 5) {
    nghin = str[str.indexOf("b") + 1] - '0';
    tram = str[str.indexOf("b") + 2] - '0';
    chuc = str[str.indexOf("b") + 3] - '0';
    dv = str[str.indexOf("b") + 4] - '0';
    t2 = nghin * 1000 + tram * 100 + chuc * 10 + dv;
  }

  //t3 - humi
  s = str.indexOf("d") - str.indexOf("c");
  if (s == 2) {
    t3 = str[str.indexOf("c") + 1] - '0';
  }
  else if (s == 3) {
    chuc = str[str.indexOf("c") + 1] - '0';
    dv = str[str.indexOf("c") + 2] - '0';
    t3 = chuc * 10 + dv;
  }
  else if (s == 4) {
    tram = str[str.indexOf("c") + 1] - '0';
    chuc = str[str.indexOf("c") + 2] - '0';
    dv = str[str.indexOf("c") + 3] - '0';
    t3 = tram * 100 + chuc * 10 + dv;
  }
  else if (s == 5) {
    nghin = str[str.indexOf("c") + 1] - '0';
    tram = str[str.indexOf("c") + 2] - '0';
    chuc = str[str.indexOf("c") + 3] - '0';
    dv = str[str.indexOf("c") + 4] - '0';
    t3 = nghin * 1000 + tram * 100 + chuc * 10 + dv;
  }

  //t4 - smoke
  s = str.indexOf("e") - str.indexOf("d");
  if (s == 2) {
    t4 = str[str.indexOf("d") + 1] - '0';
  }
  else if (s == 3) {
    chuc = str[str.indexOf("d") + 1] - '0';
    dv = str[str.indexOf("d") + 2] - '0';
    t4 = chuc * 10 + dv;
  }
  else if (s == 4) {
    tram = str[str.indexOf("d") + 1] - '0';
    chuc = str[str.indexOf("d") + 2] - '0';
    dv = str[str.indexOf("d") + 3] - '0';
    t4 = tram * 100 + chuc * 10 + dv;
  }
  else if (s == 5) {
    nghin = str[str.indexOf("d") + 1] - '0';
    tram = str[str.indexOf("d") + 2] - '0';
    chuc = str[str.indexOf("d") + 3] - '0';
    dv = str[str.indexOf("d") + 4] - '0';
    t4 = nghin * 1000 + tram * 100 + chuc * 10 + dv;
  }

  //t5 - sound
  s = str.indexOf("f") - str.indexOf("e");
  if (s == 2) {
    t5 = str[str.indexOf("e") + 1] - '0';
  }
  else if (s == 3) {
    chuc = str[str.indexOf("e") + 1] - '0';
    dv = str[str.indexOf("e") + 2] - '0';
    t5 = chuc * 10 + dv;
  }
  else if (s == 4) {
    tram = str[str.indexOf("e") + 1] - '0';
    chuc = str[str.indexOf("e") + 2] - '0';
    dv = str[str.indexOf("e") + 3] - '0';
    t5 = tram * 100 + chuc * 10 + dv;
  }
  else if (s == 5) {
    nghin = str[str.indexOf("e") + 1] - '0';
    tram = str[str.indexOf("e") + 2] - '0';
    chuc = str[str.indexOf("e") + 3] - '0';
    dv = str[str.indexOf("e") + 4] - '0';
    t5 = nghin * 1000 + tram * 100 + chuc * 10 + dv;
  }

  //t6 - thief
  s = str.indexOf("g") - str.indexOf("f");
  if (s == 2) {
    t6 = str[str.indexOf("f") + 1] - '0';
  }
  else if (s == 3) {
    chuc = str[str.indexOf("f") + 1] - '0';
    dv = str[str.indexOf("f") + 2] - '0';
    t6 = chuc * 10 + dv;
  }
  else if (s == 4) {
    tram = str[str.indexOf("f") + 1] - '0';
    chuc = str[str.indexOf("f") + 2] - '0';
    dv = str[str.indexOf("f") + 3] - '0';
    t6 = tram * 100 + chuc * 10 + dv;
  }
  else if (s == 5) {
    nghin = str[str.indexOf("f") + 1] - '0';
    tram = str[str.indexOf("f") + 2] - '0';
    chuc = str[str.indexOf("f") + 3] - '0';
    dv = str[str.indexOf("f") + 4] - '0';
    t6 = nghin * 1000 + tram * 100 + chuc * 10 + dv;
  }

  //t7 - battery
  s = str.indexOf("h") - str.indexOf("g");
  if (s == 2) {
    t7 = str[str.indexOf("g") + 1] - '0';
  }
  else if (s == 3) {
    chuc = str[str.indexOf("g") + 1] - '0';
    dv = str[str.indexOf("g") + 2] - '0';
    t7 = chuc * 10 + dv;
  }
  else if (s == 4) {
    tram = str[str.indexOf("g") + 1] - '0';
    chuc = str[str.indexOf("g") + 2] - '0';
    dv = str[str.indexOf("g") + 3] - '0';
    t7 = tram * 100 + chuc * 10 + dv;
  }
  else if (s == 5) {
    nghin = str[str.indexOf("g") + 1] - '0';
    tram = str[str.indexOf("g") + 2] - '0';
    chuc = str[str.indexOf("g") + 3] - '0';
    dv = str[str.indexOf("g") + 4] - '0';
    t7 = nghin * 1000 + tram * 100 + chuc * 10 + dv;
  }

} // end xulydulieu

//ASCII value of '0' (character) is 48 and '1' is 49. So to convert 48-56('0'-'9') to 0-9

//////Telemetry
void Onboard_Telemetry() {

  currentSerialMillis = millis();
  if (currentSerialMillis - prevSerialMillis >= 100) { //Run routine every millisRoutineInterval (ms)
    prevSerialMillis = currentSerialMillis;
    Serial.print("     Temp1: "); Serial.println(temp1);
    Serial.print("      Humi: "); Serial.println(humi1);
    Serial.print("    Smoke1: "); Serial.println(smoke1);
    Serial.print("    Sound1: "); Serial.println(sound1);
    Serial.print("    Thief1: "); Serial.println(thief1);
    Serial.print("  Battery1: "); Serial.println(battery1);
    Serial.print("fire_level_1:"); Serial.println(P1);
    Serial.println("=============");
    Serial.print("     Temp2: "); Serial.println(temp2);
    Serial.print("      Humi: "); Serial.println(humi2);
    Serial.print("    Smoke2: "); Serial.println(smoke2);
    Serial.print("    Sound2: "); Serial.println(sound2);
    Serial.print("    Thief2: "); Serial.println(thief2);
    Serial.print("  Battery2: "); Serial.println(battery2);
    Serial.print("fire_level_2:"); Serial.println(P2);
    Serial.println("=============");
  }
}
//LoRa config (information)
void printParameters(struct Configuration configuration) {
  Serial.println("----------------------------------------");

  Serial.print(F("HEAD : "));  Serial.print(configuration.HEAD, BIN); Serial.print(" "); Serial.print(configuration.HEAD, DEC); Serial.print(" "); Serial.println(configuration.HEAD, HEX);
  Serial.println(F(" "));
  Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, HEX);
  Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, HEX);
  Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, HEX); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
  Serial.println(F(" "));
  Serial.print(F("SpeedParityBit     : "));  Serial.print(configuration.SPED.uartParity, BIN); Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
  Serial.print(F("SpeedUARTDatte  : "));  Serial.print(configuration.SPED.uartBaudRate, BIN); Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRate());
  Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN); Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRate());

  Serial.print(F("OptionTrans        : "));  Serial.print(configuration.OPTION.fixedTransmission, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getFixedTransmissionDescription());
  Serial.print(F("OptionPullup       : "));  Serial.print(configuration.OPTION.ioDriveMode, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getIODroveModeDescription());
  Serial.print(F("OptionWakeup       : "));  Serial.print(configuration.OPTION.wirelessWakeupTime, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getWirelessWakeUPTimeDescription());
  Serial.print(F("OptionFEC          : "));  Serial.print(configuration.OPTION.fec, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getFECDescription());
  Serial.print(F("OptionPower        : "));  Serial.print(configuration.OPTION.transmissionPower, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());

  Serial.println("----------------------------------------");

}
void printModuleInformation(struct ModuleInformation moduleInformation) {
  Serial.println("----------------------------------------");
  Serial.print(F("HEAD BIN: "));  Serial.print(moduleInformation.HEAD, BIN); Serial.print(" "); Serial.print(moduleInformation.HEAD, DEC); Serial.print(" "); Serial.println(moduleInformation.HEAD, HEX);

  Serial.print(F("Freq.: "));  Serial.println(moduleInformation.frequency, HEX);
  Serial.print(F("Version  : "));  Serial.println(moduleInformation.version, HEX);
  Serial.print(F("Features : "));  Serial.println(moduleInformation.features, HEX);
  Serial.println("----------------------------------------");

}

////// Display LCD
void TFT_Print() {

  tft.drawRect(1, 2, 158, 126, TFT_WHITE);
  tft.drawLine(40, 0, 40, 128, TFT_WHITE);
  tft.drawLine(100, 0, 100, 128, TFT_WHITE);

  tft.drawLine(0, 29, 158, 29, TFT_WHITE);
  tft.drawLine(0, 54, 158, 54, TFT_WHITE);
  tft.drawLine(0, 79, 158, 79, TFT_WHITE);
  tft.drawLine(0, 103, 158, 103, TFT_WHITE);
  tft.setSwapBytes(true);
  tft.pushImage(2, 30, 24, 24, nhietdo);
  tft.pushImage(5, 55, 24, 24, iconDoAm);
  if (pin1 > 75) tft.pushImage(44, 104, 23, 23, full);
  else if (75 > pin1 && pin1 > 50) tft.pushImage(44, 104, 23, 23, high);
  else if (50 > pin1 && pin1 > 25) tft.pushImage(44, 104, 23, 23, low);
  else if (25 > pin1 && pin1 > 0) tft.pushImage(44, 104, 23, 23, over);

  if (pin2 > 75) tft.pushImage(104, 104, 23, 23, full);
  else if (75 > pin2 && pin2 > 50) tft.pushImage(104, 104, 23, 23, high);
  else if (50 > pin2 && pin2 > 25) tft.pushImage(104, 104, 23, 23, low);
  else if (25 > pin2 && pin2 > 0) tft.pushImage(104, 104, 23, 23, over);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(1);
  tft.setCursor(21, 35); tft.print(" C");
  tft.setCursor(29, 60); tft.print("%");
  tft.drawString("SMOKE", 6, 83);
  tft.drawString("% PIN", 6, 112);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("<ppm>", 7, 93);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(1);
  tft.setCursor(7, 8); tft.print((String)hour()); tft.print(":");
  tft.print(String(minute()));
  tft.setCursor(15, 17);
  tft.print((String)second()); tft.print(" ");

  switch (fire_level_1){
    case 1 : {tft.setTextColor(TFT_WHITE, TFT_BLUE);   break;}
    case 2 : {tft.setTextColor(TFT_WHITE, TFT_GREEN);  break;}
    case 3 : {tft.setTextColor(TFT_BLACK, TFT_YELLOW); break;}
    case 4 : {tft.setTextColor(TFT_BLACK, TFT_ORANGE); break;}
    case 5 : {tft.setTextColor(TFT_WHITE, TFT_RED);    break;}
    default: {tft.setTextColor(TFT_WHITE, TFT_BLACK);  break;}
  }
  tft.setTextFont(2);
  tft.drawString(" Node 1 ", 43, 7);

  switch (fire_level_2){
    case 1 : {tft.setTextColor(TFT_WHITE, TFT_BLUE);   break;}
    case 2 : {tft.setTextColor(TFT_WHITE, TFT_GREEN);  break;}
    case 3 : {tft.setTextColor(TFT_BLACK, TFT_YELLOW); break;}
    case 4 : {tft.setTextColor(TFT_BLACK, TFT_ORANGE); break;}
    case 5 : {tft.setTextColor(TFT_WHITE, TFT_RED);    break;}
    default: {tft.setTextColor(TFT_WHITE, TFT_BLACK);  break;}
  }
  tft.setTextFont(2);
  tft.drawString(" Node 2 ", 102, 7);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextFont(2);
  tft.setCursor(44, 31); tft.print(temp1, 2); tft.setCursor(104, 31); tft.print(temp2, 2);
  tft.setCursor(44, 57); tft.print(humi1, 2); tft.setCursor(104, 57); tft.print(humi2, 2);
  tft.setTextFont(1);
  tft.setCursor(66, 112); tft.print(pin1, 2);
  tft.setCursor(125, 112); tft.print(pin2, 2);

  if ( mauCanhBao == 1) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
  }
  else tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextFont(2);
  tft.setCursor(44, 84); tft.print(smoke1, 2); tft.print(" ");
  tft.setCursor(104, 84); tft.print(smoke2, 2); tft.print(" ");
}


/////////////Warning
void CanhBao() {
  currentMillis = millis();
  if (currentMillis - previousMillis >= 500) {
    previousMillis = currentMillis;
  }
  digitalWrite(ledSafe, HIGH);
  digitalWrite(ledWarn, LOW);
  digitalWrite(buzz,    LOW);

  mauCanhBao = 0;
  fire_level();
  
  if (smoke1>smokeHigh || temp1>40)               {Blynk.logEvent("node1"); mauCanhBao=1; warning();} 
  else if (smoke2>smokeHigh || temp2>40)          {Blynk.logEvent("node2"); mauCanhBao=1; warning();}
  
  else if (thief1 == 1 || sound1 == 1)                           {Blynk.logEvent("node1"); warning();}
  else if (thief2 == 1 || sound2 == 1)                           {Blynk.logEvent("node2"); warning();}

  if(smoke1>smokeHigh) {
    setNumber(2);
    delay(500);
  }
  else if(smoke2>smokeHigh) {
    setNumber(3);
    delay(500);
  }
  else if(sound1 == 1 || thief1 == 1) {
    setNumber(4);
    delay(500);
  }
  else if(sound2 == 1 || thief2 == 1) {
    setNumber(5);
    delay(500);
  }
  else if(temp1>40) {
    setNumber(6);
    delay(500);
  }
  else if(temp2>40) {
    setNumber(7);
    delay(500);
  }
}

//Warning LED
void warning(){
    digitalWrite(ledSafe, LOW);
    digitalWrite(ledWarn, HIGH);
    digitalWrite(buzz,    HIGH);
    delay(500);
    digitalWrite(ledSafe, HIGH);
    digitalWrite(ledWarn, LOW);
    digitalWrite(buzz,    LOW);
}

//Level Fire
void fire_level(){
  a1 = temp1;
  E1 = 100;
  e1 = humi1;
  P1 = (a1 * (E1 - e1));
  fire_level_1 = P1 / 5000 + 1;
  switch (fire_level_1){
    case 1:  {led11 = 1; break;}
    case 2:  {led11 = 1; led12 = 1; break;}
    case 3:  {led11 = 1; led12 = 1; led13 = 1; break;}
    case 4:  {led11 = 1; led12 = 1; led13 = 1; led14 = 1; break;}
    case 5:  {led11 = 1; led12 = 1; led13 = 1; led14 = 1; led15 = 1; break;}
    default: {led11 = 1; break;}
  }
  
  a2 = temp2;
  E2 = 100;
  e2 = humi2;
  P2 = (a2 * (E2 - e2));
  fire_level_2 = P2 / 5000 + 1;
  switch (fire_level_2){
    case 1:  {led21 = 1; break;}
    case 2:  {led21 = 1; led22 = 1; break;}
    case 3:  {led21 = 1; led22 = 1; led23 = 1; break;}
    case 4:  {led21 = 1; led22 = 1; led23 = 1; led24 = 1; break;}
    case 5:  {led21 = 1; led22 = 1; led23 = 1; led24 = 1; led25 = 1; break;}
    default: {led21 = 1; break;}
  }
}

////Blynk
void senBlynk() {
  Blynk.virtualWrite(0, temp1);
  Blynk.virtualWrite(1, humi1);
  Blynk.virtualWrite(2, smoke1);
  Blynk.virtualWrite(4, thief1);
  Blynk.virtualWrite(5, pin1);
  Blynk.virtualWrite(6, temp2);
  Blynk.virtualWrite(7, humi2);
  Blynk.virtualWrite(8, smoke2);
  Blynk.virtualWrite(9, thief2);
  Blynk.virtualWrite(3, pin2);
}
//End
