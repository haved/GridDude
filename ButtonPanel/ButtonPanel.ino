#include <SoftwareSerial.h>

const int WIFI_RX=2, WIFI_TX=3;
const int WIFI_ENABLE=2;
const int UP = 9;
const int DOWN = 10;
const int LEFT = 8;
const int RIGHT = 11;
const int LIGHT = 12;

SoftwareSerial wifiSerial(WIFI_RX, WIFI_TX);

void setup() {
  pinMode(WIFI_ENABLE, OUTPUT);
	pinMode(UP, OUTPUT);
  pinMode(DOWN, OUTPUT);
  pinMode(LEFT, OUTPUT);
  pinMode(RIGHT, OUTPUT);
  pinMode(LIGHT, OUTPUT);

  wifiSerial.begin(115200);
  while(!wifiSerial); //Wait

  Serial.begin(115200);
  while(!Serial); //Also wait
}

int in;

void loop() {
  if(Serial.available()>0) {
    Serial.print("To wifi: ");
    while((in=Serial.read())>0) {
      wifiSerial.write(in);
      Serial.write(in);
    }
  }
  if(wifiSerial.available()>0) {
    Serial.print("From wifi: ");
    while((in=wifiSerial.read())>0) {
      in = wifiSerial.read();
      Serial.write(in);
    }
  }
  delay(100);
}

