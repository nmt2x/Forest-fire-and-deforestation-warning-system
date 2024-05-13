/*
  SEND NODE 2 (09 X 09)

*/
char s[32];
#include "LoRa_E32.h"
LoRa_E32 e32ttl(3, 2);
#define nodeID 2

//DHT11
#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float t, h;  
int tt, hh;

//MQ2
#include <MQ2.h>
int pin = A1;
int lpg, co, smoke;
MQ2 mq2(pin);

//Sound
#define sound_sensor 5
int sound_value;

//SR501
#define thief_sensor 12
int thief_value;

//Vol
int volPin = A0;
float vol;
int battery_value;
float vout = 0,
      vin  = 0;

//LED
#define ledSafe 8
#define ledWarn 7

void setup()
{
  Serial.begin(9600);
  pinMode(sound_sensor, INPUT);
  pinMode(thief_sensor, INPUT);
  pinMode(ledSafe, OUTPUT);
  pinMode(ledWarn, OUTPUT);
  dht.begin();
  mq2.begin();
  e32ttl.begin();
}

void loop()
{
  temphumi();
  battery();
  sound();
  thief();
  dust();
  show();
  send_to_gateway();
}

void temphumi(){
  t = dht.readTemperature();
  h = dht.readHumidity();
  tt = t * 10.0;
  hh = h * 10.0;
}

void dust(){
  float* values= mq2.read(true);
  lpg = mq2.readLPG();
  co = mq2.readCO();
  smoke = mq2.readSmoke() ;
  if (smoke < 0 )   smoke = 0;
  if (smoke > 9999)  smoke = 9999;
}

void sound(){
  sound_value = digitalRead(sound_sensor);
}

void thief(){
  thief_value = digitalRead(thief_sensor);
}

void battery(){
    vin = analogRead(volPin) * (5.0 / 1023.0);
    vol = vin * 0.992;
    battery_value = vol * 100.0;
}

void show(){
  Serial.print("    ID : ");  Serial.print(nodeID);
  Serial.print("  Temp : ");  Serial.print(t);
  Serial.print("  Humi : ");  Serial.print(h);
  Serial.print("  Dust : ");  Serial.print(smoke);
  Serial.print(" Sound : ");  Serial.print(sound_value);
  Serial.print(" Thief : ");  Serial.print(thief_value);
  Serial.print("   Vol : ");  Serial.println(vol);
  digitalWrite(ledSafe, HIGH);

  if (t>40 || smoke>250 || sound_value==0 || thief_value==1){
    digitalWrite(ledSafe, LOW);
    digitalWrite(ledWarn, HIGH);
    delay(500);
    digitalWrite(ledSafe, HIGH);
    digitalWrite(ledWarn, LOW);
  }
}

void send_to_gateway(){ 
  if (runEvery(5000)) {
    String mess1 = 'a' + String(nodeID) + 'b' + String(tt) + 'c' + String(hh) 
                 + 'd' + String(smoke) + 'e' + String(sound_value) + 'f' 
                 + String(thief_value) + 'g' + String(battery_value) + 'h';
    ResponseStatus rs = e32ttl.sendFixedMessage(0, 9, 0x09, mess1);
  }
}

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
