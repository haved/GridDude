#include <SoftwareSerial.h>

const int RX=0, TX=1;
const int WIFI_ENABLE=2;
const int UP = 9;
const int DOWN = 10;
const int LEFT = 8;
const int RIGHT = 11;
const int LIGHT = 12;

SoftwareSerial wifiSerial(RX, TX);

void setup() {
  pinMode(WIFI_ENABLE, OUTPUT);
	pinMode(UP, OUTPUT);
  pinMode(DOWN, OUTPUT);
  pinMode(LEFT, OUTPUT);
  pinMode(RIGHT, OUTPUT);
  pinMode(LIGHT, OUTPUT);

  wifiSerial.begin(115200);
  while(!wifiSerial); //Wait
}

void loop() {
	
  delay(1000);
}
